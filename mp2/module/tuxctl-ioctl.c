/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define LED_SHOW0	0xE7		//LED byte bit masks
#define LED_SHOW1	0x06
#define LED_SHOW2	0xCB
#define LED_SHOW3	0x8F
#define LED_SHOW4	0x2E
#define LED_SHOW5	0xAD
#define LED_SHOW6	0xED
#define LED_SHOW7	0x86
#define LED_SHOW8	0xEF
#define LED_SHOW9	0xAE
#define LED_SHOWA	0xEE
#define LED_SHOWB	0x6D
#define LED_SHOWC	0xE1
#define LED_SHOWD	0x4F
#define LED_SHOWE	0xE9
#define LED_SHOWF	0xE8
#define DECIMAL		0x10
#define HEXRANGE	  16


#define LOWER_4_MASK 0x0F

#define LSB_MASK	0x01

#define UP_MASK		0x10
#define LEFT_MASK	0x40
#define DOWN_MASK	0x20
#define RIGHT_MASK	0x80

#define INIT_PACKET	   2
#define LED_PACKSIZE   6

// static spinlock_t tuxctl_button_lock = SPIN_LOCK_UNLOCKED;

//unsigned char bioc_buttons_packet[3];		// global var to save button packet


unsigned char ack_signal = 0;				// save MTCP_ACK signal
unsigned long save_argument_state;			// save previous state of LED argument
unsigned char button_status;


unsigned char hex_values[HEXRANGE] = {LED_SHOW0,LED_SHOW1,LED_SHOW2,LED_SHOW3,	// HEX values table, siize 16 for hex 0-F
	LED_SHOW4,LED_SHOW5,LED_SHOW6,LED_SHOW7,
	LED_SHOW8,LED_SHOW9,LED_SHOWA,LED_SHOWB,
	LED_SHOWC,LED_SHOWD,LED_SHOWE,LED_SHOWF};

int ioctl_tux_init(struct tty_struct* tty);
int ioctl_tux_set_led(struct tty_struct* tty, unsigned long arg);
int ioctl_tux_buttons(struct tty_struct* tty, unsigned long arg);



#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
	unsigned char button_byte = 0x00;
	unsigned char buttons,direction;
	// unsigned long flags;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

	switch (a)
	{
	case MTCP_ACK: 
		ack_signal = 1;		//handleMTCP_ACK signal
		return;

	case MTCP_BIOC_EVENT:
		
		// bioc_buttons_packet[0] = a;		// assign BIOC_EVENT to save buttons
		// bioc_buttons_packet[1] = b;
		// bioc_buttons_packet[2] = c;

		// unsigned char button_byte = 0x00;		// initialize button status value

		//buttons = bioc_buttons_packet[1] & LOWER_4_MASK;
		//direction = bioc_buttons_packet[2] & LOWER_4_MASK;

		buttons = b & LOWER_4_MASK;
		direction = c & LOWER_4_MASK;
	
		button_byte = button_byte | buttons;			// set lower 4 bits to C|B|A|START

		//0x10 sets UP bit on button byte
		if((direction & LSB_MASK) == LSB_MASK){
			button_byte |= 0x10;	// if UP, then set corresponding bit
		}
		//0x40 sets LEFT bit on button byte
		if(((direction>>1) & LSB_MASK) == LSB_MASK){
			button_byte |= 0x40;    // if LEFT, set corresponding bit
		}
		//0x20 sets DOWN bit on button byte
		if(((direction>>2) & LSB_MASK) == LSB_MASK){
			button_byte |= 0x20;	// if DOWN, set corresponding bit
		}
		//0x80 sets RIGHT bit on button byte
		if(((direction>>3) & LSB_MASK) == LSB_MASK){
			button_byte |= 0x80;	// if RIGHT, set corresponding bit
		}

		button_status = button_byte;	// set local variable to global button status

		return;

	case MTCP_RESET:
		ioctl_tux_init(tty);							// initialize TUX on reset
		ioctl_tux_set_led(tty, save_argument_state);	// set LEDs to previous state
		return;

	default:
		return;
	}

    /*printk("packet : %x %x %x\n", a, b, c); */
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT:
		return ioctl_tux_init(tty);				// call init ioctl
	case TUX_BUTTONS:
		return ioctl_tux_buttons(tty,arg);		// call buttons ioctl
	case TUX_SET_LED:
		return ioctl_tux_set_led(tty,arg);		// call set led ioctl
	case TUX_LED_ACK:
		return 0;
	case TUX_LED_REQUEST:
		return 0;
	case TUX_READ_LED:
		return 0;
	default:
	    return -EINVAL;
    }
}
/*
* ioctl_tux_init()
* DESCRIPTION: initialization ioctl that initializes all the variables associated with driver
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: returns 0 for successfullu initializing all variables
* SIDE EFFECTS: creates initialization packet and MTCP_ACK signal
*/
int ioctl_tux_init(struct tty_struct* tty){
	unsigned char initialize_tux[INIT_PACKET];		// initialize package buffer array

	//unsigned char initialize_tux = MTCP_BIOC_ON;

	initialize_tux[0] = MTCP_BIOC_ON;	// assign opcode
	initialize_tux[1] = MTCP_LED_USR;	//put display into user mode
	
	ack_signal = 1;	// initialize MTCP_ACK signal
	button_status = 0x00;	// reset button status
	
	tuxctl_ldisc_put(tty,initialize_tux,INIT_PACKET);			// put packet to tux

	return 0;

}

/*
* ioctl_tux_set_led()
* DESCRIPTION: sets LED values on tux based on argument
* INPUTS: passes in hex number to display, the leds to show, and the decimals to show
* OUTPUTS: none
* RETURN VALUE: returns 0 for successfully setting LEDS
* SIDE EFFECTS: creates whole LED packet to be passed on to tux controller
*/
int ioctl_tux_set_led(struct tty_struct* tty, unsigned long arg){
	//int i;
	//int put_size;
	int num_leds = 0;
	unsigned char set_led_buffer[LED_PACKSIZE];
	uint16_t num_display = arg & 0xFFFF;	// get lower 16 bits, mask by 0xFFFF
	
	unsigned char update_leds = (arg >> 16) & LOWER_4_MASK;	// shift to get 3rd byter of argument
	unsigned char decimal_display = (arg >> 24) & LOWER_4_MASK;	// shift and mask to get 4th byte of argument

	save_argument_state = arg;				// save previous state in case of reset signal

	set_led_buffer[0] = MTCP_LED_SET;		// set opcodes as first byte passed in
	set_led_buffer[1] = LOWER_4_MASK;				// set LED indexes as next byte passed in

	// for(i = 2; i < 6; i++){												// initialize all buffer values to zero
	// 	set_led_buffer[i] = 0x00;
	// }

	if((update_leds & LSB_MASK) == LSB_MASK){
		num_leds+=1;
		set_led_buffer[2] = hex_values[num_display & LOWER_4_MASK];				// assign hex display to packet for LED0
		if(decimal_display & LSB_MASK){									// manually check if decimal bit is on for corresponding led
			set_led_buffer[2] |= 0x10;									// set dp bit high for corresponding byte
		}
	}
	else set_led_buffer[2] = 0x00;										//assign to zero if LED is off

	if(((update_leds>>1) & LSB_MASK) == LSB_MASK){						// assign hex display to packet for LED1
		num_leds+=1;													// bytes to get number of leds to set,
		set_led_buffer[3] = hex_values[(num_display >> 4) & LOWER_4_MASK];		//decimal value to show, and led number to display
		if((decimal_display >> 1) & LSB_MASK){
			set_led_buffer[3] |= 0x10;
		}
	}
	else set_led_buffer[3] = 0x00;

	if(((update_leds>>2) & LSB_MASK) == LSB_MASK){						// same logic as above,repeated for next
		num_leds+=1;													// increment number of LEDs set	
		set_led_buffer[4] = hex_values[(num_display >> 8) & LOWER_4_MASK];	//assign LED2 to corresponding hex value
		if((decimal_display >> 2) & LSB_MASK){								//shift by 8 to get upper byte of hex value to display
			set_led_buffer[4] |= 0x10;
		}
	}
	else set_led_buffer[4] = 0x00;

	if(((update_leds>>3) & LSB_MASK) == LSB_MASK){						// same as above
		num_leds+=1;
		set_led_buffer[5] = hex_values[(num_display >> 12) & LOWER_4_MASK];		//shift by 12 to get upper hex of upper byte to display
		if((decimal_display >> 3) & LSB_MASK){									//shift 3 to get corresponding LED bit
			set_led_buffer[5] |= 0x10;											//set LED3 packet value, add decimal if needed
		}
	}
	else set_led_buffer[5] = 0x00;

	// put_size = num_leds+2;												// initialize put size based on size of first two bytes
	tuxctl_ldisc_put(tty,set_led_buffer,LED_PACKSIZE);						// send packet
	return 0;
}


/*
* ioctl_tux_buttons()
* DESCRIPTION: takes button byte and assigns it to the pointer
* INPUTS: passes in pointer to the button values pressed on tux
* OUTPUTS: none
* RETURN VALUE: returns 0 for successfully copying button status to ptr, zero is ptr is invalid
* SIDE EFFECTS: none
*/

int ioctl_tux_buttons(struct tty_struct* tty, unsigned long arg){

	int copy_ptr;	// variable to cast argument to and check NULL

	unsigned long* check_ptr = (unsigned long *) arg;
	if(check_ptr == NULL) return -EINVAL;	//checks if ptr is NULL, returns invalid if it is
	
	copy_ptr = copy_to_user(check_ptr,&button_status,1);	//uses copy to user to set byte to the ptr
	
	if(copy_ptr != 0) return -EINVAL;	// if copy to user fails, return invalid

	else return 0;	//return zero for successfully assigning pointer
}

