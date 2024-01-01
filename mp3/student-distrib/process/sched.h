#ifndef SCHED_H
#define SCHED_H

#include "../types.h"
#include "process.h"

//////////////////// TERMINAL STUFF /////////////////////

#define MAX_TERMINAL 3
#define KEYBUF_MAX_SIZE 128
// start terminals' vidmem after actual vidmem in vmem (with a single page gap in between)
#define TERM0_VIDMEM_ADDR 0xBA000

/* term info key pressed flag bitnumber for each key */
#define SHIFT_FLAG_BITNUM 0
#define BKSPC_FLAG_BITNUM 1
#define TAB_FLAG_BITNUM 2
#define CTRL_FLAG_BITNUM 3
#define ALT_FLAG_BITNUM 4
#define ENTER_FLAG_BITNUM 5

/* terminal info struct. stores info about current state of a terminal
 * to allow recovering terminal state when switching to it */
typedef struct term_info {
    volatile term_id_t term_id;	     /* unique id for terminal (8 bits - support up to 255 open terminals) */
    int32_t cursor_x;		     /* x-pos of cursor on screen */
    int32_t cursor_y;		     /* y-pos of cursor on screen */
    uint32_t nprocess;		     /* number of processes running on the terminal */
    pid_t curr_pid;	             /* pid of process currently running in terminal */
    uint32_t vidmem_addr;	     /* virtual mem addr for video memory terminal will write to */
    uint32_t user_vidmem;        /* user video memory virtual address */
    uint32_t keybufcnt;		     /* count characters typed in buffer */
    uint32_t prev_keybufcnt;	     /* num chars typed in previous buffer (before enter pressed) */
    volatile uint8_t key_flags;      /* flags indicating whether key is pressed */
    char keybuf[KEYBUF_MAX_SIZE];    /* characters in terminal's keyboard buffer */
} term_info_t;


/* currently active terminal id */
extern volatile term_id_t curr_term;

/* store info about all terminals in system */
extern term_info_t terminals[MAX_TERMINAL];

/* initialize a new terminal and return terminal id */
extern int32_t init_term(term_id_t term_id);

/* determines whether given terminal id is considered invalid */
extern int32_t invalid_term_id(term_id_t term_id);

/* switch active terminal to terminal with given id */
extern void switch_terminal(term_id_t term_id);

/* get user video memory address for given terminal */
extern inline uint32_t get_term_user_vidmem(term_id_t term_id);

////////////////////////////////////////////////////////

///////////////// PROCESS HANDLING STUFF ///////////////

/* schedules next process in run queue */
extern void schedule();

////////////////////////////////////////////////////////

#endif	/* SCHED_H */
