/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"


/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* static uint8_t disable_cnt; */

/* void disable_all_irq() { */
/*     if (disable_cnt != 0) return; */
/*     disable_cnt = 1; */
/*     master_mask = inb(MASTER_8259_DATA); */
/*     slave_mask = inb(MASTER_8259_DATA); */
/*     outb(0xFF, MASTER_8259_DATA); */
/*     outb(0xFF, SLAVE_8259_DATA); */
/* } */

/* void enable_all_irq()  { */
/*     if (disable_cnt == 0) return; */
/*     disable_cnt = 0; */
/*     outb(master_mask, MASTER_8259_DATA); */
/*     outb(slave_mask, SLAVE_8259_DATA); */
/* } */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    //disable_cnt = 0;
    // disable all IRQs
    outb(0xFF, MASTER_8259_DATA);
    outb(0xFF, SLAVE_8259_DATA);
    
    master_mask = inb(MASTER_8259_DATA);

    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW4, MASTER_8259_DATA);

    outb(master_mask, MASTER_8259_DATA);
    
    slave_mask = inb(SLAVE_8259_DATA);

    outb(ICW1,SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);

    outb(slave_mask, SLAVE_8259_DATA);
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    uint16_t pic_port;
    uint8_t pic_data;

    // determine which PIC the interrupt is coming from
    if(irq_num < 8) {
        pic_port = MASTER_8259_DATA;
    } else {
        pic_port = SLAVE_8259_DATA;
        irq_num -= 8;
    }

    pic_data = inb(pic_port) & ~(1 << irq_num); // enable the mask
    outb(pic_data, pic_port);
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    uint16_t pic_port;
    uint8_t pic_data;

    // determine which PIC the interrupt is coming from
    if(irq_num < 8) {
        pic_port = MASTER_8259_DATA;
    } else {
        pic_port = SLAVE_8259_DATA; 
        irq_num -= 8;
    }

    pic_data = inb(pic_port) | (1 << irq_num); //disable the mask
    outb(pic_data,pic_port);
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if (irq_num >= 8) {
        outb(EOI | SLAVE_IRQ, MASTER_8259_PORT);
        uint32_t irq_on_slave = irq_num - 8;
        outb(EOI | irq_on_slave, SLAVE_8259_PORT);
        return;
    }
    outb(EOI | irq_num, MASTER_8259_PORT);
}
