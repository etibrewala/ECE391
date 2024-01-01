#ifndef PAGING_H
#define PAGING_H

#define PAGEDIR_SIZE 1024	/* size of the non 4 mb page directory */
#define PAGETABLE_SIZE 1024	/* size of the page table */
#define PAGE_SIZE_4KB 4096		/* size of an individual page */
#define PAGE_SIZE_4MB 0x400000
#define KERNEL_ADDR 0x400000
#define KERNEL_END_ADDR (KERNEL_ADDR + PAGE_SIZE_4MB)
#define PROCESS_IMG_ADDR 0x08000000
#define PROCESS_DIR_IDX (PROCESS_IMG_ADDR >> 22)

#define USER_VIDEO1 0x8400000 // right after user program addr block
#define USER_VIDEO2 0x8401000
#define USER_VIDEO3 0x8402000
#ifndef ASM

#define ZERO_PAGEDIR_KB(name)     \
    name.present = 0       ;\
    name.rw = 0            ;\
    name.us = 0            ;\
    name.pwt = 0           ;\
    name.pcd = 0           ;\
    name.accessed = 0      ;\
    name.avl1 = 0          ;\
    name.ps0 = 0           ;\
    name.avl2 = 0          ;\
    name.pt_baseaddr = 0    \


#define ZERO_PAGEDIR_MB(name)         \
    name.present = 0                 ;\
    name.rw = 0                      ;\
    name.us = 0                      ;\
    name.pwt = 0                     ;\
    name.pcd = 0                     ;\
    name.accessed = 0                ;\
    name.d = 0                       ;\
    name.ps1 = 0                     ;\
    name.global = 0                  ;\
    name.avl = 0                     ;\
    name.pat = 0                     ;\
    name.page_baseaddr_bit39_32 = 0  ;\
    name.rsvd = 0                    ;\
    name.page_baseaddr_bit31_22 = 0   \


#define ZERO_PAGETAB_ENTRY(name)      \
    name.present = 0                 ;\
    name.rw = 0                      ;\
    name.us = 0                      ;\
    name.pwt = 0                     ;\
    name.pcd = 0                     ;\
    name.accessed = 0                ;\
    name.d = 0                       ;\
    name.global = 0                  ;\
    name.avl = 0                     ;\
    name.page_baseaddr = 0            \

#include "../types.h"
#include "../lib.h"

// Implemented in paging.c //

/* initalize page directory and page table entries and enable in x86  */
extern void paging_init();

// Functions implemented in paging_asm.S //

/* load address of page directory array into registers */
extern void load_page_directory(unsigned int*);

/* enable paging in x86 */
extern void enable_paging();

/* enable mixed pages in x86 */
extern void allow_mixed_pages();

extern inline uint32_t get_pd_idx(uint32_t vmem_addr);

extern inline uint32_t get_pt_idx(uint32_t vmem_addr);

// Data structures for page directory entries and page table entries //

////////////////////////////////////////////////// PAGE DIRECTORY 4KB STRUCT /////////////////////////////////////////////////////
typedef union pagedirkb_entry_t {
    uint32_t val;
    struct {
		uint32_t present             : 1; /* whether page tables/pages present for this entry */
		uint32_t rw                  : 1; /* read/write permissions flags */
		uint32_t us                  : 1; /* User/supervisor access control bit */
		uint32_t pwt                 : 1; /* write-through */
		uint32_t pcd                 : 1; /* cache disable */
		uint32_t accessed            : 1; /* whether the page directory was accessed in memory translation */
		uint32_t avl1                : 1; /* available (?) */
		uint32_t ps0                 : 1; /* page size for entry. Always 0 for 4kb dir entries */
		uint32_t avl2                : 4; /* idk why there's two AVL entries */
		uint32_t pt_baseaddr         : 20; /* addr to start of page table */
    } __attribute__ ((packed));
} pagedirkb_entry_t;

////////////////////////////////////////////////// PAGE DIRECTORY 4MB STRUCT /////////////////////////////////////////////////////
/* note these don't point to a page table; point directly to a page */
typedef union pagedirmb_entry_t {
    uint32_t val;
    struct {
		uint32_t present                : 1;
		uint32_t rw                     : 1;
		uint32_t us                     : 1;
		uint32_t pwt                    : 1;
		uint32_t pcd                    : 1;
		uint32_t accessed               : 1;
		uint32_t d                      : 1;
		uint32_t ps1                    : 1; /* page size for entry. Always 1 for 4MB entries */
		uint32_t global                 : 1; /* tells processor to not invalidate TLB entry on reload of CR3 */
		uint32_t avl                    : 3;
		uint32_t pat                    : 1; /* page attribute table (set to 0 for now...) */
		uint32_t page_baseaddr_bit39_32 : 8; /* used for metadata */
		uint32_t rsvd                   : 1; /* reserved for CPU use (set to 0) */
		uint32_t page_baseaddr_bit31_22 : 10; /* addr to start of 4MB page */
    } __attribute__ ((packed));
} pagedirmb_entry_t;

////////////////////////////////////////////////// GENERAL PAGE DIR ENTRY TYPE /////////////////////////////////////////////
typedef union pagedir_entry_t {
    pagedirkb_entry_t kb;
    pagedirmb_entry_t mb;
} pagedir_entry_t;

////////////////////////////////////////////////// PAGE TABLE ENTRY STRUCT /////////////////////////////////////////////////////
typedef union page_table_entry_t {
    uint32_t val;
    struct {
	uint32_t present         : 1;
	uint32_t rw              : 1;
	uint32_t us              : 1;
	uint32_t pwt             : 1;
	uint32_t pcd             : 1;
	uint32_t accessed        : 1;
	uint32_t d               : 1;
	uint32_t pat             : 1;
	uint32_t global          : 1;
	uint32_t avl             : 3;
	uint32_t page_baseaddr   : 20;
    } __attribute__ ((packed));
} page_table_entry_t;

/* only 1 page directory. All memory can be accessed through it */
extern pagedir_entry_t pd[PAGEDIR_SIZE] __attribute__((aligned (PAGE_SIZE_4KB)));

/* one page table for now for first 4MB in mem. Add more as needed by programs */
extern page_table_entry_t pt0[PAGETABLE_SIZE] __attribute__((aligned (PAGE_SIZE_4KB)));
// added for vidmap
extern page_table_entry_t pt1[PAGETABLE_SIZE] __attribute__((aligned (PAGE_SIZE_4KB)));

#endif // ASM

#endif // PAGING_H
