#ifndef SYSCALLNUM_H
#define SYSCALLNUM_H

#define NUM_SYSCALLS 10

/* number associated with each system call
 * Used to access system calls in jump table */

// idk if we actually use these anywhere but here for reference

#define SYS_HALT    1
#define SYS_EXECUTE 2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETARGS 7
#define SYS_VIDMAP  8
#define SYS_SET_HANDLER  9
#define SYS_SIGRETURN  10

#endif	/* end if SYSCALLNUM_H */
