/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "interrupts/idt_handlers.h"
#include "lib.h"
#include "i8259.h"
#include "devices/init_devices.h"
#include "debug.h"
#include "tests.h"
#include "paging/paging.h"
#include "filesystems/filesystem.h"
#include "interrupts/syscalls.h"
#include "process/process.h"
#include "process/sched.h"

#define RUN_TESTS 0

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags, bit)   ((flags) & (1 << (bit)))


/* flag to indicate when no programs running */
uint8_t sentinel = 1;

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void entry(unsigned long magic, unsigned long addr) {

    multiboot_info_t *mbi;

    /* do all kernel stuff in default terminal 0 */
    curr_term = 0;
    
    /* Clear the screen. */
    clear();

    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: 0x%#x\n", (unsigned)magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf("flags = 0x%#x\n", (unsigned)mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG(mbi->flags, 0))
        printf("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG(mbi->flags, 1))
        printf("boot_device = 0x%#x\n", (unsigned)mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG(mbi->flags, 2))
        printf("cmdline = %s\n", (char *)mbi->cmdline);

    if (CHECK_FLAG(mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;
        while (mod_count < mbi->mods_count) {
            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            printf("First few bytes of module:\n");
            for (i = 0; i < 16; i++) {
                printf("0x%x ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
            mod++;
        }
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5)) {
        printf("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG(mbi->flags, 5)) {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);
        printf("elf_sec: num = %u, size = 0x%#x, addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned)elf_sec->num, (unsigned)elf_sec->size,
                (unsigned)elf_sec->addr, (unsigned)elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG(mbi->flags, 6)) {
        memory_map_t *mmap;
        printf("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned)mbi->mmap_addr, (unsigned)mbi->mmap_length);
        for (mmap = (memory_map_t *)mbi->mmap_addr;
                (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *)((unsigned long)mmap + mmap->size + sizeof (mmap->size)))
            printf("    size = 0x%x, base_addr = 0x%#x%#x\n    type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned)mmap->size,
                    (unsigned)mmap->base_addr_high,
                    (unsigned)mmap->base_addr_low,
                    (unsigned)mmap->type,
                    (unsigned)mmap->length_high,
                    (unsigned)mmap->length_low);
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity = 0x0;
        the_ldt_desc.opsize      = 0x1;
        the_ldt_desc.reserved    = 0x0;
        the_ldt_desc.avail       = 0x0;
        the_ldt_desc.present     = 0x1;
        the_ldt_desc.dpl         = 0x0;
        the_ldt_desc.sys         = 0x0;
        the_ldt_desc.type        = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity   = 0x0;
        the_tss_desc.opsize        = 0x0;
        the_tss_desc.reserved      = 0x0;
        the_tss_desc.avail         = 0x0;
        the_tss_desc.seg_lim_19_16 = TSS_SIZE & 0x000F0000;
        the_tss_desc.present       = 0x1;
        the_tss_desc.dpl           = 0x0;
        the_tss_desc.sys           = 0x0;
        the_tss_desc.type          = 0x9;
        the_tss_desc.seg_lim_15_00 = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000;
        ltr(KERNEL_TSS);
    }
    //===================================================
    //OUR CODE
    //===================================================
    /* Construct an IDT table*/
    {

        /*Initialize all entries of the IDT to default values (present = 0)*/
        int idt_init;
        for(idt_init = 0; idt_init < NUM_VEC; idt_init++){
            
            idt[idt_init].seg_selector = KERNEL_CS;

            idt[idt_init].reserved4 = 0;
	        // init these 3 to 1 to set to trap gate
            idt[idt_init].reserved3 = 1;
            idt[idt_init].reserved2 = 1;
            idt[idt_init].reserved1 = 1;
            idt[idt_init].reserved0 = 0;

            idt[idt_init].size = 1;    //each gate is 32 bits
            idt[idt_init].dpl = 0;
            idt[idt_init].present = 0;
        }

        //initialize IDT entries 0x00-0x1F (present = 1)
        for(idt_init = 0x00; idt_init < 0x1F; idt_init++){
            idt[idt_init].present = 1;
        }
	
	// change to interrupt gates (kbd, rtc, pit)
	idt[0x20].reserved3 = 0;
	idt[0x21].reserved3 = 0;
	idt[0x28].reserved3 = 0;

	idt[0x20].present = 1;	/* pit interrupts */
        idt[0x21].present = 1;	/* keyboard interrupts */
        idt[0x28].present = 1;	/* RTC interrupts */

	// system calls (has lowest privledge level 3)
        idt[0x80].present = 1;
        idt[0x80].dpl = 3;

        /* Set all exception handlers in the IDT*/
        SET_IDT_ENTRY(idt[0x0], divide_zero_linkage);
        SET_IDT_ENTRY(idt[0x1], debug_linkage);
        SET_IDT_ENTRY(idt[0x2], nmi_linkage);
        SET_IDT_ENTRY(idt[0x3], breakpoint_linkage);
        SET_IDT_ENTRY(idt[0x4], overflow_linkage);
        SET_IDT_ENTRY(idt[0x5], bnd_rng_exceed_linkage);
        SET_IDT_ENTRY(idt[0x6], invalid_opcode_linkage);
        SET_IDT_ENTRY(idt[0x7], device_na_linkage);
        SET_IDT_ENTRY(idt[0x8], double_fault_linkage);
        SET_IDT_ENTRY(idt[0x9], seg_overrun_linkage);
        SET_IDT_ENTRY(idt[0xA], invalid_tss_linkage);
        SET_IDT_ENTRY(idt[0xB], seg_nopres_linkage);
        SET_IDT_ENTRY(idt[0xC], stack_segfault_linkage);
        SET_IDT_ENTRY(idt[0xD], gen_protect_flt_linkage);
        SET_IDT_ENTRY(idt[0xE], pg_fault_linkage);
        SET_IDT_ENTRY(idt[0x10], x87_fpe_linkage);
        SET_IDT_ENTRY(idt[0x11], align_check_linkage);
        SET_IDT_ENTRY(idt[0x12], machine_check_linkage);
        SET_IDT_ENTRY(idt[0x13], simd_fpe_linkage);
        SET_IDT_ENTRY(idt[0x14], virt_linkage);
        SET_IDT_ENTRY(idt[0x15], ctl_protect_linkage);
        SET_IDT_ENTRY(idt[0x1C], hpi_linkage);
        SET_IDT_ENTRY(idt[0x1D], vmm_comm_linkage);
        SET_IDT_ENTRY(idt[0x1E], security_linkage);

        /* Set all interrupt handlers in the IDT */
	SET_IDT_ENTRY(idt[0x20], pit_linkage);
        SET_IDT_ENTRY(idt[0x21], kbd_linkage);
        SET_IDT_ENTRY(idt[0x28], rtc_linkage);

        /* System Call handler
		 * note: argument to linkage passed through EAX by whoever
		 * initiated system call in user space */
        SET_IDT_ENTRY(idt[0x80], syscall_linkage);
    }

    /* Init the PIC */
    //disable_all_irq();
    i8259_init();

    /* initialize devices. Turn on IRQs for these devices */
    /*IRQ2 is enabled to account for secondary PIC*/
    enable_irq(2);
    init_pit();
    init_rtc();
    init_kbd();

    module_t* ext2_filesys = (module_t*)(mbi->mods_addr);
    init_ext2_filesys(ext2_filesys->mod_start);
    
    paging_init();

    init_pcb_arr();
    // initialize three terminals
    (void)init_term(0);
    (void)init_term(1);
    (void)init_term(2);

    clear();
    // ==============================================================

    /* Enable interrupts */
    /* Do not enable the following until after you have set up your
     * IDT correctly otherwise QEMU will triple fault and simple close
     * without showing you any output */
    // printf("Enabling Interrupts\n");
    sti();

#ifdef RUN_TESTS
    /* Run tests */
    //launch_tests();
#endif
    /* Execute the first program ("shell") ... */
    sentinel = 0;
    //sys_execute("shell");
    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile (".1: hlt; jmp .1;");
}
