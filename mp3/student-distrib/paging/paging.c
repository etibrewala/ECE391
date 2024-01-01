#include "paging.h"

pagedir_entry_t pd[PAGEDIR_SIZE];
page_table_entry_t pt0[PAGETABLE_SIZE];
page_table_entry_t pt1[PAGETABLE_SIZE];

/*
 * void paging_init(void):
 *    DESCRIPTION: initalize page directory and page table entries and enable in x86
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Initialize entries in pd and pt0 arrays and set registers cr0, cr3, cr4
 */
void paging_init()
{
    unsigned int j;

    /* initialize page directory entries 
     * Note: all entries in pd and pt0 are set to 0 in x86_desc.S
     */
    for (j = 0; j < PAGEDIR_SIZE; j++) {
	ZERO_PAGEDIR_MB(pd[j].mb);
    }

    /* first 4MB block in physical mem. Uses 4KB pages */
    pd[0].kb.present = 1;
    pd[0].kb.pt_baseaddr = ((unsigned int)pt0) >> 12;
    pd[0].kb.rw = 1;

    /* 2nd 4MB physical mem block for kernel */
    pd[1].mb.present = 1;
    pd[1].mb.ps1 = 1;
    pd[1].mb.rw = 1;
    pd[1].mb.global = 1;
    pd[1].mb.page_baseaddr_bit31_22 = KERNEL_ADDR >> 22;
    
    // set up process image page (base address changed dynamically as new processes created)
    ZERO_PAGEDIR_MB(pd[PROCESS_DIR_IDX].mb);
    pd[PROCESS_DIR_IDX].mb.present = 1;
    pd[PROCESS_DIR_IDX].mb.ps1 = 1;
    pd[PROCESS_DIR_IDX].mb.rw = 1;
    pd[PROCESS_DIR_IDX].mb.us = 1;

    // initializing page table 0 and 1
    for (j = 0; j < PAGETABLE_SIZE; j++)
    {
        ZERO_PAGETAB_ENTRY(pt0[j]);
        ZERO_PAGETAB_ENTRY(pt1[j]);
        pt0[j].rw = 1;
        pt1[j].rw = 1;
        pt0[j].page_baseaddr = j;
    }
    // set video mem page to present
    pt0[VIDEO >> 12].present = 1;

    // assembly stuff
    load_page_directory((unsigned int*)pd);
    allow_mixed_pages();
    enable_paging();
}


inline uint32_t get_pd_idx(uint32_t vmem_addr) {
    return vmem_addr >> 22;
}


inline uint32_t get_pt_idx(uint32_t vmem_addr) {
    return (vmem_addr >> 12) & 0x3FF;
}
