#ifndef SYSCALL_H
#define SYSCALL_H

#include "../types.h"
#include "../process/process.h"
#include "../filesystems/filesystem.h"
#include "../paging/paging.h"


/* system calls referenced by the system call handler. */

extern int32_t sys_halt(uint8_t status);
extern int32_t sys_execute(const int8_t* cmd);
extern int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t sys_open(const int8_t* fname);
extern int32_t sys_close(int32_t fd);
extern int32_t sys_getargs(int8_t* buf, int32_t nbytes);
extern int32_t sys_vidmap(int8_t** screen_start);
extern int32_t sys_set_handler(int32_t signum, void* handler_addr);
extern int32_t sys_sigreturn(void);


#endif	/* endif SYSCALL_H */
