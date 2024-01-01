#define ASM 1

.text

.globl init_paging_structs, load_page_directory, enable_paging, allow_mixed_pages

/*
 * load_page_directory(pagedir_entry_t* pd)
 * Loads the address of the page directory array into register cr3.
 * INPUTS       : pd   - address of start of page directory
 * OUTPUTS      : none
 * SIDE EFFECTS : modify cr3
 */
load_page_directory:
    pushl %ebp
    movl %esp, %ebp
    movl 8(%ebp), %eax
    movl %eax, %cr3
    leave	
    ret

/*
 * allow_mixed_pages()
 * Allow mixed pages for paging
 * INPUTS       : none
 * OUTPUTS      : none
 * SIDE EFFECTS : modify cr4
 */
allow_mixed_pages:
    pushl %ebp
    movl %esp, %ebp
    movl %cr4, %eax
    orl $0x00000010, %eax
    movl %eax, %cr4
    leave	
    ret

/*
 * enable_paging()
 * Enable paging on device
 * INPUTS       : none
 * OUTPUTS      : none
 * SIDE EFFECTS : modify cr0
 */
enable_paging:
    pushl %ebp
    movl %esp, %ebp
    movl %cr0, %eax
    orl $0x80000001, %eax
    movl %eax, %cr0
    leave	
    ret
