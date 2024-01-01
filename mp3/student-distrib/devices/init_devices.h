#ifndef INIT_DEV_H

#include "../lib.h"

#define INIT_DEV_H

#define RTC_IRQ 8
#define KBD_IRQ 1
#define PIT_IRQ 0

#define RTC_INDEX 0x70
#define RTC_DATA 0X71

#define PIT_CHAN0_DATA 0x40
#define PIT_MODE_REG 0x43
#define PIT_COUNTER 12000

/* initialize the RTC enabling periodic interrupts */
extern void init_rtc();

/* initializing the keyboard device to allow of keyboard interrupts */
extern void init_kbd();

/* initialize PIT setting interrupt frequency, channel, and mode */
extern void init_pit();

#endif
