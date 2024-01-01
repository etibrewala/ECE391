#include "process.h"
#include "../x86_desc.h"
#include "../filesystems/filesystem.h"
#include "../interrupts/syscalls.h"


/////// EXTERNALLY VISIBLE VARIABLES ////////

pid_t curr_pid = -1;
pcb_t* pcb_arr[NUM_PROCESS];

/////////////////////////////////////////////

static uint32_t return_status = 0;
/* buffer for command line arguments */
static int8_t command_line[CMD_ARG_LEN];

////// HELPER FUNCTIONS ///////
static inline int32_t get_next_pid(pid_t pid);

/////////////////// SYSTEM EXECUTE HELPERS /////////////////////////////

/* parse args from shell */
static int32_t parse_args(const int8_t* arg, int8_t* const buf);

/* check if file with given contents is an executable */
static inline uint32_t is_executable(const int8_t* file_contents);

/* initialize pcb memory block in kernel for given pid */
static pcb_t* create_new_pcb(pid_t pid, term_id_t term_id);

/* set up stack and iret to user space */
static void switch_to_user(uint32_t user_eip);

//////////////// SYSTEM HALT HELPERS //////////////////////////////////

/* clear all entries in file descriptor array of given process */
static void clear_fd_array(pid_t pid);

///////////////////////////////////////////////////////////////////////


void init_pcb_arr() {
    unsigned int i;
    for (i = 0; i < NUM_PROCESS; ++i) {
	pcb_arr[i] = NULL;
    }
}


/* start_process()
 * 
 * DESCRIPTION:   helper function for sys_execute, starts a process for command with given name
 * INPUTS:        cmd - program name to execute
 *                term_id - terminal to start process in
 * OUTPUTS:       none
 * RETURNS:       returns 0-255 status from halt, 256 for exceptions, -1 if error occurs or invalid executable
 * SIDE EFFECTS:  Creates new PCB entry, modifies program page directory memory mapping, flushes TLB, updates curr_pid, updates TSS
 *                modifies memory at determined program location, and switches to user space to run program
 */
int32_t start_process(const int8_t* cmd, term_id_t term_id) {
    cli();
    int32_t res;    /* check whether file system operations succeeded */
    // later change to get all arguments to given command as well (probably get_args syscall?)
    int8_t fname[FILENAME_LEN+1] = {'\0'};

    // empty command line arg
    memset(command_line, '\0', CMD_ARG_LEN);

    pid_t next_pid = get_next_pid(curr_pid);
    if (next_pid < 0) {
        // max num processes reached, can't create more
        return -1;
    }

    res = parse_args(cmd, fname);
    if (res < 0) {
	return -1;
    }
    setup_process_page(next_pid);

    // Obtain directory entry info for file
    dentry_t file_dentry;
    res = read_dentry_by_name(fname, &file_dentry);
    if (res < 0) {
	// reverse paging back to current process's page before returning
	setup_process_page(curr_pid);
	return -1;
    }

    // reads file contents
    uint32_t file_len = fs_inode_arr[file_dentry.inode_num].length;
    res = read_data(file_dentry.inode_num, 0, (int8_t*)PROGRAM_VIRTUAL_ADDR, file_len);
    if (res < 0) { // read failed
	    setup_process_page(curr_pid);
	    return -1;
    }
    //checks if program is a valid executable
    if (!is_executable((int8_t*)PROGRAM_VIRTUAL_ADDR)) {
	    setup_process_page(curr_pid);
	    return -1;
    }
    // init new pcb struct
    (void)create_new_pcb(next_pid, term_id);
    /* if new process has a parent and is running in same terminal as parent, pause the parent
     * note: in this design, curr_pid is the parent process. */
    if (pcb_arr[next_pid]->parent_pid >= 0 && pcb_arr[curr_pid]->term_id == pcb_arr[next_pid]->term_id) {
	pcb_arr[curr_pid]->state = PAUSED;
    }
    curr_pid = next_pid;
    set_process_tss(curr_pid);

    terminals[term_id].nprocess++;
    terminals[term_id].curr_pid = curr_pid;
    
    // set up stack for iret
    uint32_t *first_instr_addr = (uint32_t*)(PROGRAM_VIRTUAL_ADDR + 24);
    switch_to_user(*first_instr_addr);

    /* make parent of halted program active again */
    pcb_arr[curr_pid]->state = ACTIVE;
    
    return return_status;   //value returned is set by system halt
}

/* squash_process()
 * 
 * DESCRIPTION:   Terminate current process
 * INPUTS:        status - 8-bit unsigned value ranging from 0-255
 * OUTPUTS:       none
 * RETURNS:       status. If called through an exception returns 256
 *                Note: never reaches return if sentinel shell program being halted
 * SIDE EFFECTS:  stops current process, restores parent process's page directory memory mapping, flushes TLB, updates curr_pid, updates TSS, resets exception flag
 *                sets ebp and esp registers
 */
int32_t squash_process(uint8_t status) {
    cli();
    term_id_t process_term = pcb_arr[curr_pid]->term_id;
    if (terminals[process_term].nprocess == 1) { /* if terminating last process in the terminal */
	terminals[process_term].nprocess--;
	pcb_arr[curr_pid]->state = STOPPED;
        curr_pid = pcb_arr[curr_pid]->parent_pid;
        // always start shell if nothing else running in the terminal
        start_process("shell", process_term);
    } else {
        clear_fd_array(curr_pid);

        // disable user video mem for program
        pt1[get_pt_idx(terminals[process_term].user_vidmem)].present = 0;
        pcb_arr[curr_pid]->using_video = 0;

	// update process state
	pcb_arr[curr_pid]->state = STOPPED;
	
        setup_process_page(pcb_arr[curr_pid]->parent_pid);
        flush_tlb();
        set_process_tss(pcb_arr[curr_pid]->parent_pid);
	curr_pid = pcb_arr[curr_pid]->parent_pid;
	
	terminals[process_term].curr_pid = curr_pid;
	terminals[process_term].nprocess--;

        if(pcb_arr[curr_pid]->exception_flag == 1) {
	    pcb_arr[curr_pid]->exception_flag = 0;
            return_status = 256; // 256 - Exception return value
        } else {
            return_status = (uint32_t)status;
        }
        // restore saved ebp and esp from before running execute
	uint32_t saved_ebp = pcb_arr[curr_pid]->stack_base_ptr;
	uint32_t saved_esp = pcb_arr[curr_pid]->stack_ptr; 
	asm volatile(
            "movl %0, %%ebp;"
            "movl %1, %%esp;"
            :   
            : "r"(saved_ebp), "r"(saved_esp)
            );
        sti();
        return return_status;
    }
    return -1;
}


//////////// HELPER FUNCTIONS ///////////////


/* setup_process_tss()
 * 
 * DESCRIPTION:   sets up tss
 * INPUTS:        pid - current process id number
 * OUTPUTS:       none
 * RETURNS:       none
 * SIDE EFFECTS:  sets segment and stack base pointer of current process
 */
inline void set_process_tss(pid_t pid) {
    tss.ss0 = KERNEL_DS;
    uint32_t pcb_bottom_addr = KERNEL_END_ADDR - (pid)*PCB_SIZE;
    tss.esp0 = pcb_bottom_addr;
}


/* flush_tlb()
 * 
 * DESCRIPTION:   flushes TLB
 * INPUTS:        none
 * OUTPUTS:       none
 * RETURNS:       none
 * SIDE EFFECTS:  resets cr3 to point to page directory (so nothing should change here), flushes TLB
 */
inline void flush_tlb() {
    asm volatile(
    	"movl %0, %%eax;"
    	"movl %%eax, %%cr3;"
    	:
    	: "r"(pd)
    	: "%eax"
	);
}


/* get_next_pid()
 * 
 * DESCRIPTION:   gets next available pid
 * INPUTS:        none
 * OUTPUTS:       none
 * RETURNS:       next available pid. Returns -1 if none available
 * SIDE EFFECTS:  none
 */
static inline int32_t get_next_pid(pid_t pid) {
    unsigned int i;
    for (i = 0; i < NUM_PROCESS; ++i) {
	if (pcb_arr[i] == NULL || pcb_arr[i]->state == STOPPED) {
	    return i;
	}
    }
    return -1;
}

/* EXECUTE helper functions */

// returns number of bytes copied into buffer

/* parse_args()
 * 
 * DESCRIPTION:   parses through terminal read buffer, gets executable argument
 * INPUTS:        arg - input from terminal
 * OUTPUTS:       buf - buffer to store executable command
 * RETURNS:       bytes loaded into buffer, -1 on failure
 * SIDE EFFECTS:  loads buffer with just the executable command from argument
 */
int32_t parse_args(const int8_t* arg, int8_t* const buf) {
    uint32_t start_pos = 0;
    // skip leading spaces/null chars
    while ((arg[start_pos] == ' ') || (arg[start_pos] == '\0')) {
	    start_pos++;
    }
    // copy to buf until space or null
    uint32_t idx = 0;
    while ((arg[start_pos + idx] != ' ') && (arg[start_pos + idx] != '\0')) {
	    buf[idx] = arg[start_pos + idx];
	    idx++;
	    if (start_pos + idx >= FILENAME_LEN) { // input too long for filename
		return -1;
	    }
    }
     
    start_pos += idx;           // new start position for args
    while ((arg[start_pos] == ' ') || (arg[start_pos] == '\0')) {
        if(arg[start_pos]=='\0') {
            buf[idx] = '\0';
            return idx;
        }
	    start_pos++;
    }

    uint32_t args_idx = 0;
    while ((arg[start_pos + args_idx] != ' ') && (arg[start_pos + args_idx] != '\0')) {
	    command_line[args_idx] = arg[start_pos + args_idx];
	    args_idx++;
    }

    command_line[args_idx] = '\0';
    buf[idx] = '\0';
    return idx;
}

/* is_executable
 * DESCRIPTION:     Check if file with given contents is an exectuable
 * INPUTS:          file_contents  - The contents of the file to check
 * OUTPUTS:         none
 * RETURN VALUE:    1 (true) if file is executable, 0 (false) otherwise
 */
static inline uint32_t is_executable(const int8_t* file_contents) {
    // check ELF magic number at start of file
    return (file_contents[0] == 0x7f) && (file_contents[1] == 0x45) && (file_contents[2]     == 0x4C) && (file_contents[3] == 0x46);
}

/* setup_process_page()
 * 
 * DESCRIPTION:   sets up 8MB or 12MB page accordingly
 * INPUTS:        pid - current process id number
 * OUTPUTS:       none
 * RETURNS:       none
 * SIDE EFFECTS:  fills page directory entry with correct physical memory mapping
 */
void setup_process_page(pid_t pid) {
    //Chooses correct page directory entry offset based on page
    pd[PROCESS_DIR_IDX].mb.page_baseaddr_bit31_22 = (PROCCESS_0_ADDR + pid * PAGE_SIZE_4MB) >> 22;
    flush_tlb();
}

/* create_new_pcb()
 * 
 * DESCRIPTION:   creates new entry in pcb array for new process forking off current process
 * INPUTS:        pid  -  pid to use for new pcb
 * OUTPUTS:       none
 * RETURNS:       new pcb_t struct object
 * SIDE EFFECTS:  initalizes new pcb_t struct object, and adds it to pcb array
 */
pcb_t* create_new_pcb(pid_t pid, term_id_t term_id) {
    uint32_t pcb_bottom_addr = KERNEL_END_ADDR - (pid)*PCB_SIZE;
    pcb_arr[pid] = (pcb_t*)(pcb_bottom_addr - PCB_SIZE);

    pcb_arr[pid]->pid = pid;
    pcb_arr[pid]->parent_pid = curr_pid;

    strcpy(pcb_arr[pid]->command_line_args, command_line);
	
    //Initializes fd array for PCB with stdin and stdout
    SET_FD_FLAG_INUSE(pcb_arr[pid]->fd_arr[STDIN_FD].flags); // stdin
    fill_stdin_ops(&pcb_arr[pid]->fd_arr[STDIN_FD].ops_jtab);
    SET_FD_FLAG_INUSE(pcb_arr[pid]->fd_arr[STDOUT_FD].flags); // stdout
    fill_stdout_ops(&pcb_arr[pid]->fd_arr[STDOUT_FD].ops_jtab);
    unsigned int i;
    for (i = 2; i < MAXFILES_PER_TASK; ++i) {
      UNSET_FD_FLAG_INUSE(pcb_arr[pid]->fd_arr[i].flags);
    }

    //Saves Kernel stack pointer
    pcb_arr[pid]->stack_base_ptr = pcb_bottom_addr - sizeof(uint32_t);
    pcb_arr[pid]->stack_ptr = pcb_bottom_addr - sizeof(uint32_t);
	
    //Sets PCB status to active
    pcb_arr[pid]->state = ACTIVE;

    pcb_arr[pid]->exception_flag = 0;
    pcb_arr[pid]->term_id = term_id;
    pcb_arr[pid]->using_video = 0;

    pcb_arr[pid]->rtc_counter = RTC_MAX_FREQ / 2;  /* default RTC freq set to 2Hz */
    pcb_arr[pid]->rtc_interrupt_cnt = 0;

    return pcb_arr[pid];
}

/* switch_to_user()
 * 
 * DESCRIPTION:   switches to user space to run program
 * INPUTS:        user_eip - instruction pointer to the first instruction in program
 * OUTPUTS:       none
 * RETURNS:       none
 * SIDE EFFECTS:  saves old process esp and ebp so that halt can return to execute when program is finished running
 */
static void switch_to_user(uint32_t user_eip) {
    /* no need for stack pointer later if there is no parent process since in that case halt
     * will just restart shell and not return to execute/syscall linkage */
    if (pcb_arr[curr_pid]->parent_pid >= 0 && (nterm_started <= 3 || terminals[pcb_arr[curr_pid]->term_id].nprocess > 1)) {
        uint32_t saved_ebp;
        uint32_t saved_esp;
        // save current ebp and esp
        asm volatile(
            "movl %%ebp, %0;"
            "movl %%esp, %1;"
            : "=r"(saved_ebp), "=r"(saved_esp) 
            );
        pcb_arr[pcb_arr[curr_pid]->parent_pid]->stack_ptr = saved_esp;
        pcb_arr[pcb_arr[curr_pid]->parent_pid]->stack_base_ptr = saved_ebp;
    }

    sti();              // enable interrupts before going to user space (need to allow kbd interrupts)
    // set up stack and iret
    asm volatile(
    	"pushl $%P1;"       // push user_ds
        "pushl $%P2;"       
        "pushfl;"
        "pushl $%P3;"       // push user_cs
        "pushl %0;"
        "iret;"
            : 
            : "r"(user_eip),
              "p"(USER_DS),
              "p"(PROCESS_IMG_ADDR + PAGE_SIZE_4MB - 4),
              "p"(USER_CS)
            : "memory"
        );
        return;
}



/* clear_fd_array()
 * 
 * DESCRIPTION:   clears fd array for a process when done using
 * INPUTS:        pid - current process id number
 * OUTPUTS:       none
 * RETURNS:       none
 * SIDE EFFECTS:  clears the flags and resets all file operation pointers
 */
void clear_fd_array(pid_t pid) {
    int i;
    //iterate through max size of fd array
    for (i = 0; i < MAXFILES_PER_TASK; i++) {
        UNSET_FD_FLAG_INUSE(pcb_arr[pid]->fd_arr[i].flags);		//set flags to unused
        fill_badcall_ops(&pcb_arr[pid]->fd_arr[i].ops_jtab);
    }
}


/* get_command_line_args()
 * 
 * DESCRIPTION:   get command line arguments from user input
 * INPUTS:        buf     - buffer to copy args into
 *                nbytes  - number of bytes of args to read into buf
 * OUTPUTS:       none
 * RETURNS:       none
 * SIDE EFFECTS:  overwrites given buffer
 */
int32_t get_command_line_args(int8_t* buf, int32_t nbytes){
    if (buf == NULL || strlen(command_line) > nbytes || strlen(command_line) == 0) {
        return -1;
    }
    strncpy(buf, pcb_arr[curr_pid]->command_line_args, CMD_ARG_LEN);
    return 0;
}


