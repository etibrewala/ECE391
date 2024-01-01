#include "rtcdrivers.h"
#include "../lib.h"


/* check if input is power of 2 */
static uint8_t is_power_of_2(int32_t);

/* compute RTC rate corresponding to given frequency */
static uint32_t compute_rtc_rate_from_freq(int32_t freq);

/* rtc_open(uint8_t*)
 * 
 * DESCRIPTION:   Open the RTC and set the frequency to 2Hz
 * INPUTS:        fname - ignore(?)
 * OUTPUTS:       none
 * RETURNS:       0
 * SIDE EFFECTS:  Changes RTC rate (value in register A). Initialize virtualization variables
 */
int32_t rtc_open(const int8_t* fname) {
    // set default rate to 2Hz
    pcb_arr[curr_pid]->rtc_counter = RTC_MAX_FREQ / 2;
    pcb_arr[curr_pid]->rtc_interrupt_cnt = 0;
    return 0;
}


/* rtc_read(int32_t, void*, int32_t)
 * 
 * DESCRIPTION:   Waits for an interrupt and then returns 0
 * INPUTS:        ignore all
 * OUTPUTS:       none
 * RETURNS:       0 on rtc interrupt
 * SIDE EFFECTS:  Changes rtc flag
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    // some basic synchronization
    while (rtc_flag == 0);   /* wait for rtc interrupt */
    rtc_flag = 0;	     /* reset RTC flag to 0 */
    return 0;		     /* return 0 when rtc interrupt received */
}


/* rtc_write(int32_t, const void*, int32_t)
 * 
 * DESCRIPTION:   Set rate of periodic interrupts for RTC
 * INPUTS:        fd     - ignore(?)
                  buf    - interrupt rate in Hz to set RTC to
 * OUTPUTS:       none
 * RETURNS:       0 on succeess, -1 if frequency is not a power of two or greater than 1024
 * SIDE EFFECTS:  Changes virtual rtc frequency
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    if (buf == NULL) return -1;
    int32_t write_freq =  *((int32_t *) buf);     /* convert write value to integer */
    if (write_freq > 1024 || !is_power_of_2(write_freq)) {
	    return -1;
    }
    pcb_arr[curr_pid]->rtc_counter = RTC_MAX_FREQ / write_freq;
    pcb_arr[curr_pid]->rtc_interrupt_cnt = 0;
    return 0;
}


/* rtc_close(int32_t)
 * 
 * DESCRIPTION:   close RTC, disable future writes/reads from it
 *                note: this function pretty  much does nothing, the parent fnc that
 *                      interacts with the fd_arr will do most of the work (i think)
 * INPUTS:        fd - ignore(?)
 * OUTPUTS:       none
 * RETURNS:       0
 * SIDE EFFECTS:  none
 */
int32_t rtc_close(int32_t fd) {
    return 0;
}


/////////////////// HELPER FUNCTIONS ////////////////////

/* is_power_of_2(uint32_t)
 * DESCRIPTION:     check if input is power of 2
 * INPUTS:          n - number to check
 * OUTPUTS:         none
 * RETURNS;         0 if n not a power of 2, nonzero value otherwise
 * SIDE EFFECTS:    none
 */
uint8_t is_power_of_2(int32_t n) {
    return (n & (n - 1)) == 0;
}


/* compute_rtc_rate_from_freq(uint32_t)
 * DESCRIPTION:     compute the corresponding rtc rate from given frequency
 * INPUTS:          freq - frequency to convert to rate. Assumes it's a power of 2
 * OUTPUTS:         none
 * RETURNS;         rate for RTC converted from given frequency
 * SIDE EFFECTS:    none
 */
uint32_t compute_rtc_rate_from_freq(int32_t freq) {
    /* freq = 2^n = 2^(15 - (rate-1))    =>    rate = 16 - n
     * where n = where first 1 occurs in binary repr of freq */
    uint32_t n = 0;
    while ((freq >> n) != 1) ++n;
    return 16 - n;
}
