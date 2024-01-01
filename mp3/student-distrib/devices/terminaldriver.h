#ifndef _TERMINALDRIVER_H
#define _TERMINALDRIVER_H
#include "../lib.h"
#include "../types.h"

extern int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

#endif /* _TERMINALDRIVER_H */
