#include "syscalls.h"
#include "../lib.h"

/* sys_halt(uint8_t)
 * 
 * DESCRIPTION:   system call for halt
 * INPUTS:        status - return value for system execute call
 * OUTPUTS:       none
 * RETURNS:       value for execute to return
 * SIDE EFFECTS:  sets return value for execute
 */
int32_t sys_halt(uint8_t status) {
    return squash_process(status);
}

/* sys_execute(const int8_t*)
 * 
 * DESCRIPTION:   system call for execute
 * INPUTS:        cmd - program to execute
 * OUTPUTS:       none
 * RETURNS:       0-255 status, 256 is exception is raised, -1 otherwise
 * SIDE EFFECTS:  starts program
 */
int32_t sys_execute(const int8_t* cmd) {
    if (cmd == NULL) {
        return -1;
    }
    return start_process(cmd, curr_term);
}

/* sys_read(int32_t, void*, int32_t)
 * 
 * DESCRIPTION:   system call for read
 * INPUTS:        fd - file descriptor index
 *                buf - buffer to read
 *                nbytes - number of bytes to read
 * OUTPUTS:       none
 * RETURNS:       number of bytes read
 * SIDE EFFECTS:  none
 */
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes) {
    int32_t res = fs_read(fd, buf, nbytes);
    sti();
    return res;
}

/* sys_write(int32_t, const void*, int32_t)
 * 
 * DESCRIPTION:   system call for write
 * INPUTS:        fd - file descriptor index
 *                buf - buffer to write to
 *                nbytes - number of bytes to write
 * OUTPUTS:       none
 * RETURNS:       number of bytes written
 * SIDE EFFECTS:  fills buffer with characters to write
 */
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes) {
    cli();
    int32_t res = fs_write(fd, buf, nbytes);
    sti();
    return res;
}

/* sys_open(const int8_t*)
 * 
 * DESCRIPTION:   system call for open
 * INPUTS:        file - file to open
 * OUTPUTS:       none
 * RETURNS:       0 if success, -1 otherwise
 * SIDE EFFECTS:  opens file
*/
int32_t sys_open(const int8_t* fname) {
    cli();
    int32_t res = fs_open(fname);
    sti();
    return res;
}

/* sys_close(int32_t)
 * 
 * DESCRIPTION:   system call for close
 * INPUTS:        fd - file directory index
 * OUTPUTS:       none
 * RETURNS:       0 if success, -1 otherwise
 * SIDE EFFECTS:  closes corresponding file descriptor entry
*/
int32_t sys_close(int32_t fd) {
    cli();
    int32_t res = fs_close(fd);
    sti();
    return res;
}


/* sys_getargs(int8_t*, int32_t)
 * 
 * DESCRIPTION:   system call to get command line arguments
 * INPUTS:        buf - buffer to copy args into
 *                nbytes - max number of bytes to copy into buffer
 * OUTPUTS:       none
 * RETURNS:       0 if success, -1 otherwise
 * SIDE EFFECTS:  fills given buffer
*/
int32_t sys_getargs(int8_t* buf, int32_t nbytes) {
    cli();
    int32_t res = get_command_line_args(buf,nbytes);
    sti();
    return res;
}


/* sys_vidmap(int8_t**)
 * 
 * DESCRIPTION:   system call to map user video memory
 * INPUTS:        screen_start - pointer to pointer in user space to the video memory address for program
 * OUTPUTS:       none
 * RETURNS:       0 if success, -1 otherwise
 * SIDE EFFECTS:  modify page tables
*/
int32_t sys_vidmap(int8_t** screen_start) {
    // invalid pointer given; address doesn't point to something in program's addr space
    if ((screen_start == NULL) || (((uint32_t)screen_start & 0xFFB00000) != PROCESS_IMG_ADDR)) {
        return -1;
    }

    pcb_arr[curr_pid]->using_video = 1;

    uint32_t pd_idx = get_pd_idx(terminals[curr_term].user_vidmem);
    uint32_t pt_idx = get_pt_idx(terminals[curr_term].user_vidmem);
    pd[pd_idx].kb.present = 1;
    pd[pd_idx].kb.us = 1;
    pd[pd_idx].kb.rw = 1;
    pd[pd_idx].kb.pt_baseaddr = ((uint32_t)pt1) >> 12;
    pt1[pt_idx].present = 1;
    pt1[pt_idx].rw = 1;
    pt1[pt_idx].us = 1;
    pt1[pt_idx].page_baseaddr = terminals[pcb_arr[curr_pid]->term_id].vidmem_addr >> 12;
    *screen_start = (int8_t*)terminals[curr_term].user_vidmem;
    flush_tlb();
    return 0;
}


///////// SIGNALS SYSCALLS //////////////

int32_t sys_set_handler(int32_t signum, void* handler_addr) {
    return -1;
}

int32_t sys_sigreturn() {
    return -1;
}

