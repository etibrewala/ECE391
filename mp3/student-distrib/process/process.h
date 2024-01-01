#ifndef PROCESS_H
#define PROCESS_H

#include "../types.h"
#include "../paging/paging.h"
#include "sched.h"

#define MAXFILES_PER_TASK 8
#define NUM_PROCESS 6
// physical addresses //
#define PROCCESS_0_ADDR KERNEL_END_ADDR
/////////////////////////
#define PROGRAM_VIRTUAL_ADDR 0x08048000
#define PCB_SIZE (1 << 13)
#define CMD_ARG_LEN 128


/* file operations jump table. Different for each file type */
struct file_ops {
    int32_t (*read) (int32_t, void*, int32_t);
    int32_t (*write) (int32_t, const void*, int32_t);
    int32_t (*open) (const int8_t*);
    int32_t (*close) (int32_t);
};


/* Stores info about a file in the file descriptor array
 * Note that read_pos is interpretted differently for each file type.
 * For directories it indicates 1 + the number of directory entrie that have been read so far. */
typedef struct fd_arr_entry {
    struct file_ops ops_jtab;		/* jmp table containing type-specific open,read,write,close fncs */
    uint32_t inode_num;				/* inode number for the file */
    uint32_t read_pos;              /* offset (in bytes) from start of file to start reading from */
    uint32_t flags;	                /* first bit 1 indicates in use, 0 indicates not in use. */
} fd_arr_entry_t;

/* Current state of a task. UPDATE LATER TO ADD MORE STATES AS NEEDED
 * ACTIVE  - process is currently running
 * PAUSED  - process suspended while child process runs
 * STOPPED - process terminated */
enum task_state {
    ACTIVE = 0,
    PAUSED,
    STOPPED
};

/* Process control block stored in kernel for each running process */
typedef struct pcb {
    pid_t pid;
    pid_t parent_pid;
    uint32_t stack_ptr;		    /* ESP */
    uint32_t stack_base_ptr;	/* EBP */
    enum task_state state;
    volatile uint8_t exception_flag : 1;       /* track whether exception thrown in process (1 if excp occured, 0 otherwise) */
    uint8_t using_video : 1;                   /* whether current process using user video memory */
    volatile term_id_t term_id;		           /* terminal process is running in */
    uint16_t rtc_counter;                      /* number of rtc interrupts to wait before noifying program of rtc interrupt ("virtual interrupt"). acts as a frequency divider. */             
    uint16_t rtc_interrupt_cnt;                /* number of rtc interrupts occured since last virtual interrupt sent */
    fd_arr_entry_t fd_arr[MAXFILES_PER_TASK];  /* file descriptor array. A given file descriptor indexes into this array to get info about the file. */
    char command_line_args[CMD_ARG_LEN];
} pcb_t;


//////////////// Variables to track the current state of running processes ////////////////////

extern pcb_t* pcb_arr[NUM_PROCESS];

/* pid of most recently created process */
extern pid_t curr_pid;

//////////////////////////////// PROCEESS HANDLING FUNCTIONS ////////////////////////////////////

/* initialize PCBs in pcb_arr */
extern void init_pcb_arr();

/* start up a process */
extern int32_t start_process(const int8_t* cmd, term_id_t term_id);

/* squash a process and return control to parent process */
extern int32_t squash_process(uint8_t status);

extern int32_t get_command_line_args(int8_t* buf, int32_t nbytes);

extern inline void set_process_tss(pid_t pid);

/* setup process page addresses for process with given pid */
extern void setup_process_page(pid_t pid);

extern inline void flush_tlb();

#endif /* endif PROCESS_H */
