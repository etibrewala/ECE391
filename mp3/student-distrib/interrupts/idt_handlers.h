#ifndef IDT_FNCS_H
#define IDT_FNCS_H

#include "../devices/init_devices.h"
#include "../types.h"
#include "syscallnums.h"
#include "kbd_map.h"
#include "../process/process.h"

/* has rtc encountered interrupt */
extern volatile uint32_t rtc_flag;

/* number of terminals started up (goes up to 3 on boot and then stays constant) */
extern unsigned int nterm_started;

/* Exception handlers. These are the functions that are called when an
* exception occurs in kernel */

/* TODO - add comments */

/* Divide by zero exception handler */
extern void divide_zero_handler();

/* Debug exception handler */
extern void debug_handler();

/* Non-maskable interrupt exception handler */
extern void nmi_handler();

/* Breakpoint exception handler*/
extern void breakpoint_handler();

/* Overflow exception handler*/
extern void overflow_handler();

/* Bound Range Exceeded handler */
extern void bnd_rng_exceed_handler();

/* Invalid opcode exception handler */
extern void invalid_opcode_handler();

/* Device not available exception handler*/
extern void device_na_handler();

/* Double fault exception handler*/
extern void double_fault_handler();

/* Coprocessor segment overrun exception handler*/
extern void seg_overrun_handler();

/* Invalid TSS exception handler*/
extern void invalid_tss_handler();

/* Segment not present exception handler*/
extern void seg_nopres_handler();

/* Stack segment fault exception handler*/
extern void stack_segfault_handler();

/* General protection fault exception handler*/
extern void gen_protect_flt_handler();

/* page fault exception handler*/
extern void pg_fault_handler();

/* x87 floating-point exception handler*/
extern void x87_fpe_handler();

/* Alignment check exception handler*/
extern void align_check_handler();

/* Machine check exception handler*/
extern void machine_check_handler();

/* SIMD floating point exception handler*/
extern void simd_fpe_handler();

/* Virualization exception handler*/
extern void virt_handler();

/* Control protection exception handler*/
extern void ctl_protect_handler();

/* Hypervisor Injection Exception handler*/
extern void hpi_handler();

/* VMM Communication Exception handler */
extern void vmm_comm_handler();

/* Security exception handler */
extern void security_handler();



/* Interrupt handlers. Do something in response to interrupt from a device */


/* RTC */
extern void rtc_handler();

/* Keyboard handler */
extern void kbd_handler();

/* PIT handler */
extern void pit_handler();

/* SYSTEM CALLS - See syscalls.h/c */


/* ----------------------------------------------------------------- */


/* Assembly wrappers around handlers.
* These functions are called within the C code and are implemented in idt_handlers_linkage.S.
* Push all necessary flags and other registers before calling their corresponding interrupt handler.
* TODO - Does this generalize to chained handlers? */

/* Divide by zero exception linkage */
extern void divide_zero_linkage();

/* Debug exception linkage */
extern void debug_linkage();

/* Non-maskable interrupt exception linkage */
extern void nmi_linkage();

/* Breakpoint exception linkage*/
extern void breakpoint_linkage();

/* Overflow exception linkage*/
extern void overflow_linkage();

/* Bound Range Exceeded linkage */
extern void bnd_rng_exceed_linkage();

/* Invalid opcode exception linkage */
extern void invalid_opcode_linkage();

/* Device not available exception linkage*/
extern void device_na_linkage();

/* Double fault exception linkage*/
extern void double_fault_linkage();

/* Coprocessor segment overrun exception linkage*/
extern void seg_overrun_linkage();

/* Invalid TSS exception linkage*/
extern void invalid_tss_linkage();

/* Segment not present exception linkage*/
extern void seg_nopres_linkage();

/* Stack segment fault exception linkage*/
extern void stack_segfault_linkage();

/* General protection fault exception linkage*/
extern void gen_protect_flt_linkage();

/* page fault exception linkage*/
extern void pg_fault_linkage();

/* x87 floating-point exception linkage*/
extern void x87_fpe_linkage();

/* Alignment check exception linkage*/
extern void align_check_linkage();

/* Machine check exception linkage*/
extern void machine_check_linkage();

/* SIMD floating point exception linkage*/
extern void simd_fpe_linkage();

/* Virualization exception linkage*/
extern void virt_linkage();

/* Control protection exception linkage*/
extern void ctl_protect_linkage();

/* Hypervisor Injection Exception linkage*/
extern void hpi_linkage();

/* VMM Communication Exception linkage */
extern void vmm_comm_linkage();

/* Security exception linkage */
extern void security_linkage();

/* linkage for interrupts */

extern void pit_linkage(); // PIT

extern void rtc_linkage(); // RTC

extern void kbd_linkage(); // Keyboard 

/* acts as dispatcher for system calls (see asm file) */
extern int32_t __attribute__((fastcall)) syscall_linkage(int32_t syscall_num);

#endif // IDT_FNCS_H
