#include "idt_handlers.h"
#include "syscalls.h"
#include "../lib.h"
#include "../i8259.h"
#include "../process/sched.h"
#include "../devices/rtcdrivers.h"

#define PRINT_HANDLER(task) printf("EXCEPTION: " task "error\n")

// keep track of whether RTC has had an interrupt
volatile uint32_t rtc_flag = 0;

unsigned int nterm_started = 0;

/*divide_zero_handler()
* DESCRIPTION: Prints the divide by zero exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: prints the exception
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void divide_zero_handler() {
    //PRINT_HANDLER("divide_zero");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*debug_handler()
* DESCRIPTION: Prints the debug handler exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void debug_handler() {
    //PRINT_HANDLER("debug");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*nmi_handler()
* DESCRIPTION: Prints the non maskable inturrupt exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void nmi_handler() {
    //PRINT_HANDLER("nmi");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*breakpoint_handler()
* DESCRIPTION: Prints the breakpoint handler exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void breakpoint_handler() {
    //PRINT_HANDLER("breakpoint");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*overflow_handler()
* DESCRIPTION: Prints the overflow handler exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void overflow_handler() {
    //PRINT_HANDLER("overflow");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*bnd_rng_exceed()
* DESCRIPTION: Prints the bounds range exceed exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void bnd_rng_exceed_handler() {
    //PRINT_HANDLER("bnd_rng_exceed");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*invalid_opcode_handler()
* DESCRIPTION: Prints the invalid opcode handler exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void invalid_opcode_handler() {
    //PRINT_HANDLER("invalid_opcode");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*device_na_handler()
* DESCRIPTION: Prints the device n/A handler exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void device_na_handler() {
    //PRINT_HANDLER("device_na");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*double_fault_handler()
* DESCRIPTION: Prints the double fault handler exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void double_fault_handler() {
    //PRINT_HANDLER("double_fault");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*seg_overrun_handler()
* DESCRIPTION: Prints the segment overrun handler exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void seg_overrun_handler() {
    //PRINT_HANDLER("seg_overrun");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*invalid_tss_handler()
* DESCRIPTION: Prints the invalid tss handler exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void invalid_tss_handler() {
    //PRINT_HANDLER("invalid_tss");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*seg_nopres_handler()
* DESCRIPTION: Prints exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void seg_nopres_handler() {
    //PRINT_HANDLER("seg_nopres");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*stack_segfault_handler()
* DESCRIPTION: Prints the stack segfault_handler exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void stack_segfault_handler() {
    //PRINT_HANDLER("stack_segfault");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*gen_protect_flt_handler()
* DESCRIPTION: Prints the exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void gen_protect_flt_handler() {
    //PRINT_HANDLER("gen_protect_flt");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*pg_fault_handler()
* DESCRIPTION: Prints the page fault handler exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void pg_fault_handler() {
    PRINT_HANDLER("pg");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt(1);
}

/*x87_fpe_handler()
* DESCRIPTION: Prints the exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void x87_fpe_handler() {
    //PRINT_HANDLER("x87_fpe");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*align_check_handler()
* DESCRIPTION: Prints the align check handler exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void align_check_handler() {
    //PRINT_HANDLER("align_check");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*machine_check_handler()
* DESCRIPTION: Prints the machine check handler exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void machine_check_handler() {
    //PRINT_HANDLER("machine_check");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*simd_fpe_handler()
* DESCRIPTION: Prints the exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void simd_fpe_handler() {
    //PRINT_HANDLER("simd_fpe");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*virt_handler()
* DESCRIPTION: Prints the exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void virt_handler() {
    //PRINT_HANDLER("virt");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*ctl_protect_handler()
* DESCRIPTION: Prints the exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void ctl_protect_handler() {
    //PRINT_HANDLER("ctl_protect");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}

/*hpi_handler()
* DESCRIPTION: Prints the exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void hpi_handler() {
    //PRINT_HANDLER("hpi");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}


/* vmm_comm_handler()
* DESCRIPTION: Prints the exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void vmm_comm_handler() {
    //PRINT_HANDLER("vmm_comm");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t) 69);
}


/* security_handler()
* DESCRIPTION: Prints the security handler exception and emulates blue screen of death by infinitly looping
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: the problem
*/
void security_handler() {
    //PRINT_HANDLER("security");
    pcb_arr[curr_pid]->exception_flag = 1;
    sys_halt((uint8_t)69);
}


/* Interrupt Handlers */

/* rtc_handler()
* DESCRIPTION: processes rtc interrupts
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: handles rtc, sends EOI when done
*/
void rtc_handler() {
    //test_interrupts();

    pid_t pid;
    for (pid = 0; pid < NUM_PROCESS; ++pid) {
	if (pcb_arr[pid] != NULL && pcb_arr[pid]->state == ACTIVE) {
	    pcb_arr[pid]->rtc_interrupt_cnt++;
	}
    }
    
    if (pcb_arr[curr_pid]->rtc_interrupt_cnt >= pcb_arr[curr_pid]->rtc_counter) {
	rtc_flag = 1;   // raise RTC flag when ready for virtual interrupt
	pcb_arr[curr_pid]->rtc_interrupt_cnt = 0;
    }

    /* We read register C to see what type of interrupt occured.
    * If register C not read RTC will not send future interrupts */
    outb(0x0C, RTC_INDEX);	/* select register C on RTC */
    (void)inb(RTC_DATA);	/* throw away info about interrupt */
    send_eoi(RTC_IRQ);
}


/*kbd_handler()
* DESCRIPTION: processes keyboard interrupts
* INPUTS: none
* OUTPUTS: none
* RETURN VALUE: none
* SIDE EFFECTS: handles keyboard, sends EOI when done
*/
void kbd_handler() {
    static unsigned int capslock = 0;
    
    terminals[curr_term].key_flags = unset_bit(terminals[curr_term].key_flags, ENTER_FLAG_BITNUM);
    
    /* array of characters corresponding to the scancode as the index */
    /* only characters in the scancode 1 are included for checkpoint 1 purposes*/
    char lower_case[] = {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 
    '\0', '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 
    'm', ',', '.', '/', '\0', '*', '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
    '\0', '\0', '\0', '\0', '\0', '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', 
    '.', '\0', '\0', '\0', '\0', '\0'};

    char upper_case[] = {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 
    '\0', '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 
    'M', ',', '.', '/', '\0', '*', '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
    '\0', '\0', '\0', '\0', '\0', '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', 
    '.', '\0', '\0', '\0', '\0', '\0'};

    char schar[] = {'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 
    '\0', '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', '\0', '|', 'Z', 'X', 'C', 'V', 'B', 'N', 
    'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
    '\0', '\0', '\0', '\0', '\0', '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', 
    '.', '\0', '\0', '\0', '\0', '\0'};

    /* saves the scancode from the keyboard port*/
    uint8_t scan_code;
    scan_code = inb(KBD_PORT);
    
    /* If scancode is within our defined character values in the array, then it is printed*/
    uint8_t lower_case_key = lower_case[scan_code];
    uint8_t upper_case_key = upper_case[scan_code];
    uint8_t schar_case_key = schar[scan_code];

    //SHIFT RELEASE LOGIC
    if ((scan_code == LSHIFT_RELEASE) || (scan_code == RSHIFT_RELEASE)) {
	terminals[curr_term].key_flags = unset_bit(terminals[curr_term].key_flags, SHIFT_FLAG_BITNUM);
    }

    //BACKSPACE LOGIC
    if (scan_code == BACKSPACE_PRESSED) {
	terminals[curr_term].key_flags = set_bit(terminals[curr_term].key_flags, BKSPC_FLAG_BITNUM);
    } else if (scan_code == BACKSPACE_RELEASE) {
        // release
	terminals[curr_term].key_flags = unset_bit(terminals[curr_term].key_flags, BKSPC_FLAG_BITNUM);
    }

    //TAB LOGIC
    if (scan_code == TAB_PRESSED) {
	terminals[curr_term].key_flags = set_bit(terminals[curr_term].key_flags, TAB_FLAG_BITNUM);
    } else if (scan_code == TAB_RELEASE) {
        // release
	terminals[curr_term].key_flags = unset_bit(terminals[curr_term].key_flags, TAB_FLAG_BITNUM);
    }

    //ENTER LOGIC
    if (scan_code == ENTER_PRESSED) {
	terminals[curr_term].key_flags = set_bit(terminals[curr_term].key_flags, ENTER_FLAG_BITNUM);
    }

    //CTRL LOGIC
    if (scan_code == LEFTCTRL_PRESSED) {
	terminals[curr_term].key_flags = set_bit(terminals[curr_term].key_flags, CTRL_FLAG_BITNUM);
    } else if (scan_code == LEFTCTRL_RELEASE) {
        // release
	terminals[curr_term].key_flags = unset_bit(terminals[curr_term].key_flags, CTRL_FLAG_BITNUM);
    }

    // ALT LOGIC
    if (scan_code == LEFTALT_PRESSED) {
	terminals[curr_term].key_flags = set_bit(terminals[curr_term].key_flags, ALT_FLAG_BITNUM);
    } else if (scan_code == LEFTALT_RELEASE) {	
	terminals[curr_term].key_flags = unset_bit(terminals[curr_term].key_flags, ALT_FLAG_BITNUM);
    }

    uint8_t shift = get_bit(terminals[curr_term].key_flags, SHIFT_FLAG_BITNUM);
    uint8_t backspace = get_bit(terminals[curr_term].key_flags, BKSPC_FLAG_BITNUM);
    uint8_t tab = get_bit(terminals[curr_term].key_flags, TAB_FLAG_BITNUM);
    uint8_t ctrl = get_bit(terminals[curr_term].key_flags, CTRL_FLAG_BITNUM);
    uint8_t alt = get_bit(terminals[curr_term].key_flags, ALT_FLAG_BITNUM);
    
    /* keyboard commands logic (ctrl-L, ctrl-C, etc.) */
    if ((ctrl == 1) && (scan_code == L_PRESS)) {
	// CTRL-L logic to clear screen
	clear();
	terminals[curr_term].keybufcnt = 0;
	memset(terminals[curr_term].keybuf, '\0', KEYBUF_MAX_SIZE);
    } else if ((ctrl == 1) && (scan_code == C_PRESS)) {
	// CTRL-C logic to halt current program
	// DOES NOT WORK. NEEDS SIGNALS IMPLEMENTED
	/* memset(terminals[curr_term].keybuf, '\0', KEYBUF_MAX_SIZE); */
	/* terminals[curr_term].keybufcnt = 0; */
	/* if (curr_pid >= 0) { // if there is a process running terminate it */
	/*     send_eoi(1); */
	/*     sys_halt(1); */
	/* } */
    } else if (alt == 1) {
	// ALT commands (switching active terminal)
	switch (scan_code) {
	    case (F1_PRESSED):
		switch_terminal(0);
		break;
	    case (F2_PRESSED):
		switch_terminal(1);
		break;
	    case (F3_PRESSED):
		switch_terminal(2);
		break;
	    default:
		// do nothing
		break;
	}
    }
    // PRESSING LOGIC (lots of cases to consider...)
    else if (get_bit(terminals[curr_term].key_flags, ENTER_FLAG_BITNUM)) { // if enter pressed
	terminals[curr_term].keybuf[terminals[curr_term].keybufcnt++] = '\n';
	putc_term('\n', curr_term);
	terminals[curr_term].prev_keybufcnt = terminals[curr_term].keybufcnt;
	terminals[curr_term].keybufcnt = 0; 
    } else if (backspace == 1) { // handle backspace pressed
	if (terminals[curr_term].keybufcnt != 0) {
	    putc_term('\b', curr_term);
	    terminals[curr_term].keybuf[--terminals[curr_term].keybufcnt] = '\0';
	}
    } else if (terminals[curr_term].keybufcnt < KEYBUF_MAX_SIZE-1) { // if buffer not full, handle other key presses (-1 to leave space for \n)
        if (tab == 1) {
            if (terminals[curr_term].keybufcnt < KEYBUF_MAX_SIZE - TABSIZE - 1) { // if can fit a tab, add it to buffer
                putc_term('\t', curr_term);
                terminals[curr_term].keybuf[terminals[curr_term].keybufcnt++] = ' ';
                terminals[curr_term].keybuf[terminals[curr_term].keybufcnt++] = ' ';
                terminals[curr_term].keybuf[terminals[curr_term].keybufcnt++] = ' ';
                terminals[curr_term].keybuf[terminals[curr_term].keybufcnt++] = ' ';
            }
        } else {
            if (scan_code <= F12_PRESSED) {
                if(scan_code == CAPSLOCK_PRESSED && capslock == 0) {
		    // toggle caps lock on
		    capslock = 1;
                } else if (scan_code == CAPSLOCK_PRESSED && capslock == 1) {
		    // toggle caps lock off
		    capslock = 0;
                } else if (((scan_code == LEFTSHIFT_PRESSED) || (scan_code == RIGHTSHIFT_PRESSED)) && (shift == 0)) {
		    // shift pressed
		    terminals[curr_term].key_flags = set_bit(terminals[curr_term].key_flags, SHIFT_FLAG_BITNUM);
                }
		else {
                    // support for keys we don't need for now... does nothing for now but adds a long list of ifs :)
                    if (scan_code == LEFTALT_PRESSED) {
                        // left alt press
                    } else if (scan_code == LEFTCTRL_PRESSED) {
                        // left control press
                    } else if (scan_code == LEFTSHIFT_PRESSED) {
                        // left shift press
                    } else if (scan_code == RIGHTSHIFT_PRESSED) {
                        // right shift press
                    } else if (scan_code == CAPSLOCK_PRESSED) {
                        // caps lock press
                    } else if (scan_code == LEFTALT_RELEASE) {
                        // left alt release
                    } else if (scan_code == LEFTCTRL_RELEASE) {
                        // left control release
                    } else if (scan_code == LSHIFT_RELEASE) {
                        // left shift release
                    } else if (scan_code == RSHIFT_RELEASE) {
                        // right shift release
                    } else if (scan_code == CAPSLOCK_RELEASE) {
                        // caps lock release
                    } else if (scan_code == ESCAPE_PRESSED) {

                    } else if (scan_code == F1_PRESSED) {
                    
                    } else if (scan_code == F2_PRESSED) {
                    
                    } else if (scan_code == KEYPADMULT_PRESSED) {

                    } else if (scan_code == F3_PRESSED) {
                    
                    } else if (scan_code == F4_PRESSED) {
                    
                    } else if (scan_code == F5_PRESSED) {

                    } else if (scan_code == F6_PRESSED) {

                    } else if (scan_code == F7_PRESSED) {

                    } else if (scan_code == F8_PRESSED) {

                    } else if (scan_code == F9_PRESSED) {

                    } else if (scan_code == F10_PRESSED) {

                    } else if (scan_code == NUMLOCK_PRESSED) {

                    } else if (scan_code == SCROLLLOCK_PRESSED) {
                    
                    } else if (scan_code == KEYPAD7_PRESSED) {

                    } else if (scan_code == KEYPAD8_PRESSED) {
                    
                    } else if (scan_code == KEYPAD9_PRESSED) {

                    } else if (scan_code == KEYPADMINUS_PRESSED) {

                    } else if (scan_code == KEYPAD4_PRESSED) {
                    
                    } else if (scan_code == KEYPAD5_PRESSED) {

                    } else if (scan_code == KEYPAD6_PRESSED) {
                    
                    } else if (scan_code == KEYPADPLUS_PRESSED) {
                    
                    } else if (scan_code == KEYPAD1_PRESSED) {

                    } else if (scan_code == KEYPAD2_PRESSED) {

                    } else if (scan_code == KEYPAD3_PRESSED) {

                    } else if (scan_code == KEYPAD0_PRESSED) {
                    
                    } else if (scan_code == KEYPADPERIOD_PRESSED) {
                    
                    } else if (scan_code == F11_PRESSED) {
                    
                    } else if (scan_code == F12_PRESSED) {
                        
                    } 
		    else {
                        if ((capslock == 1 || shift == 1) && ctrl == 0) {
                            if (capslock == 1 && shift == 1) {
                                // caps lock pressed and want special char
                                if ((scan_code == BACKTICK_PRESSED) || (scan_code == ONE_PRESSED) || (scan_code == TWO_PRESSED) || (scan_code == THREE_PRESSED) || (scan_code == FOUR_PRESSED) || (scan_code == FIVE_PRESSED) || (scan_code == SIX_PRESSED) || (scan_code == SEVEN_PRESSED) || (scan_code == EIGHT_PRESSED) || (scan_code == NINE_PRESSED) || (scan_code == ZERO_PRESSED) || (scan_code == MINUS_PRESSED) || (scan_code == EQUAL_PRESSED) || (scan_code == LEFTBRACKET_PRESSED) || (scan_code == RIGHTBRACKET_PRESSED) || (scan_code == BACKSLASH_PRESSED) || (scan_code == SEMICOLON_PRESSED) || (scan_code == SINGLEQUOTE_PRESSED) || (scan_code == COMMA_PRESSED) || (scan_code == PERIOD_PRESSED) || (scan_code == FORWARDSLASH_PRESSED))
                                {
                                    putc_term(schar_case_key, curr_term);
                                    terminals[curr_term].keybuf[terminals[curr_term].keybufcnt] = schar_case_key;
                                    terminals[curr_term].keybufcnt++;
                                } else {
                                    putc_term(lower_case_key, curr_term);
                                    terminals[curr_term].keybuf[terminals[curr_term].keybufcnt] = lower_case_key;
                                    terminals[curr_term].keybufcnt++;
                                }
                            } else if (shift == 1) {
                                putc_term(schar_case_key, curr_term);
                                terminals[curr_term].keybuf[terminals[curr_term].keybufcnt] = schar_case_key;
                                terminals[curr_term].keybufcnt++;
                            } else {
                                putc_term(upper_case_key, curr_term);
                                terminals[curr_term].keybuf[terminals[curr_term].keybufcnt] = upper_case_key;
                                terminals[curr_term].keybufcnt++;
                            }
                        } else {
                            putc_term(lower_case_key, curr_term);
                            terminals[curr_term].keybuf[terminals[curr_term].keybufcnt] = lower_case_key;
                            terminals[curr_term].keybufcnt++;
                        }
                    }
                }
            }
        }
    }

    /*Sends en of interrupt signal for IRQ1*/
    send_eoi(KBD_IRQ);
}


void pit_handler() {
    /* first 3 interrupts start a shell in each terminal.
     * rest of interrupts trigger scheduler */
    switch (nterm_started) {
	case (0):
	    switch_terminal(0);	    
	    nterm_started++;
	    clear();
	    send_eoi(PIT_IRQ);
	    start_process("shell", 0);
	    break;
	case (1):
	    nterm_started++;
	    send_eoi(PIT_IRQ);
	    start_process("shell", 1);
	    break;
	case (2):
	    nterm_started++;
	    send_eoi(PIT_IRQ);
	    start_process("shell", 2);
	    break;
    case (3):
        nterm_started++;
        // fall-through
    default:
	    send_eoi(PIT_IRQ);
	    schedule();
    }
}

