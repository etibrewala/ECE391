#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesystems/filesystem.h"
#include "devices/terminaldriver.h"
#include "interrupts/idt_handlers.h"
#include "devices/init_devices.h"
#include "devices/rtcdrivers.h"

/* set GRAPHICS to 1 to include print statements for large files/images (i.e., frame1) in test cases */
#define GRAPHICS 1

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	   //original value was int $15
	asm volatile("int $15");
}

//=================================================================================
/* Checkpoint 1 tests */
//=================================================================================
/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}



int idt_div_zero_trigger_test() {
    TEST_HEADER;

    int a, zero, b;
    a = 2;
    zero = 0;
    b = a / zero;
    return b;
}

int overflow_test() {
    TEST_HEADER;

    int a;
    a = 0xFFFFFFFF;
    int one;
	one = 1;
    a = a + one;
    return FAIL;
}

// Test to see if we can access the kernel memory at its location
// Expected return value: PASS
int kernelexist_test() {
	TEST_HEADER;
	char* location0 = (char*)0x400000; //physical address where kernel starts
	char testing0;
	testing0 = *location0;
	return PASS;
}

// Test to see if we can access the kernel memory at its location
// Expected return value: PASS
int kernelexistdone_test() {
	TEST_HEADER;
	char* location0 = (char*)0x7FFFFF; //physical address where kernel ends
	char testing0;
	testing0 = *location0;
	return PASS;
}

// Test to see if we can access the video memory at its location
// Expected return value: PASS
int videomemexist_test() {
	TEST_HEADER;
	char* location1 = (char*)0xB8000; //physical address where video memory starts
	char testing1;
	testing1 = *location1;
	return PASS;
}

// Test to see if we can access the video memory at its location
// Expected return value: PASS
int videomemexistdone_test() {
	TEST_HEADER;
	char* location1 = (char*)0xB8FFF; //physical address where video memory starts
	char testing1;
	testing1 = *location1;
	return PASS;
}

// Test to see if we can access the one below the lower bound of kernel memory address
// Expected return value: FAIL
int kernelexistlower_test() {
	TEST_HEADER;
	char* location0 = (char*)0x3FFFFF; //physical address where kernel starts
	char testing0;
	testing0 = *location0;
	return FAIL;
}

// Test to see if we can access the one above the higher bound of kernel memory address
// Expected return value: FAIL
int kernelexisthigher_test() {
	TEST_HEADER;
	char* location0 = (char*)0x800000; //physical address one above kernel ending
	char testing0;
	testing0 = *location0;
	return FAIL;
}

// Test lowerbound of videomemory
// Expected return value: FAIL
int videomemexistlower_test() {
	TEST_HEADER;
	char* location2 = (char*)0xB7FFF;//physical address one below video memory start
	char testing2;
	testing2 = *location2;
	return FAIL;
}

// Test upperbound of videomemory
// Expected return value: FAIL
int videomemexisthigher_test() {
	TEST_HEADER;
	char* location3 = (char*)0xB9000;//physical address one above video memory ending
	char testing3;
	testing3 = *location3;
	return FAIL;
}

// // Test location zero
// // Expected return value: FAIL
// int zero() {
// 	TEST_HEADER;
// 	char* location3 = (char*)0x0;//physical address of 0
// 	char testing3;
// 	testing3 = *location3;
// 	return FAIL;
// }

// // Test location 1 above 4 GiB bound
// // Expected return value: FAIL
// int max() {
// 	TEST_HEADER;
// 	char* location3 = (char*)0x100000000;//physical address one above 4 GiB bound
// 	char testing3;
// 	testing3 = *location3;
// 	return FAIL;
// }


// Test to see if we can access a page which does not exist yet
// will FAIL if page fault not thrown
int imaginemem_test() {
	TEST_HEADER;
	char* location4 = (char*)0x30000;//physical address of random spot
	char testing4;
	testing4 = *location4;
	return FAIL;
}

// Test random exception on IDT
// Expected return value: raise exception
int assertion_failure1_test() {
	asm volatile("int $7"); // raise device_na exception
	return 1;
}

// Test random exception on IDT
// Expected return value: raise exception
int assertion_failure2_test(){
	asm volatile("int $12"); //raise stack_segfault exception
	return 1;
}

// Test system call 0x80
int system_call_fail_test(){
	asm volatile("int $0x80"); //raise stack_segfault exception
	return 1;
}
//====================================================================

//====================================================================
/* Checkpoint 2 tests */
//====================================================================

int test_dir_read() {
	curr_pid = 0;
	int fd = fs_open(".");
    int test_ret;
    int8_t buf[FILENAME_LEN+1];
#if (GRAPHICS == 1)
	int i;
    clear();
#endif
    while ((test_ret = directory_read(fd, buf, 0)) != 0) {
		buf[FILENAME_LEN] = '\0';
#if (GRAPHICS == 1)
		dentry_t d;
		read_dentry_by_name(buf, &d);
		uint32_t nspace = 35 - strlen(buf);
		printf("File Name:");
		for (i = 0; i < nspace; ++i) {
			putc(' ');
		}
		printf("%s, ", buf);
		printf("File Type: %d, ", d.filetype);
		printf("File Size: %d\n", fs_inode_arr[d.inode_num].length);
#endif	
    }
	fs_close(fd);
    curr_pid = -1;
    return test_ret == 0 ? PASS : FAIL;
}


int test_dir_write() {
    int res = directory_write(0, NULL, 0);
    return res == -1 ? PASS : FAIL;
}


int test_file_open_bad() {
    // try to open nonexistent file
    int32_t res = fs_open("kaboom.txt");
    return res == -1 ? PASS : FAIL;
}


int test_file_open_good() {
    // try to open existing file
    int i;
    const int8_t* fname = "created.txt";
    int32_t fd = fs_open(fname);
    // unable to open file
    if (fd < 0) {
		return FAIL;
    }
    // only fd entries 0, 1, 2 should be in use
    if (!FD_FLAG_INUSE(pcb_arr[curr_pid]->fd_arr[fd].flags)) {
		return FAIL;
    }
    for (i = 2; i < MAXFILES_PER_TASK; ++i) {
	if (i == fd) {
	    continue;
	}
	if (FD_FLAG_INUSE(pcb_arr[curr_pid]->fd_arr[i].flags)) {
	    return FAIL;
	}
    }
    dentry_t file_entry;
    read_dentry_by_name(fname, &file_entry);
    // check inode num is correct
    if (file_entry.inode_num != pcb_arr[curr_pid]->fd_arr[fd].inode_num) {
		return FAIL;
    }

    fs_close(fd);
    
    return PASS;
}

// CHANGE FILE CLOSE TESTS. FUNCTIONS PERFORMS DIFFERENTLY NOW
int test_file_close_defaults() {
    int32_t res1 = fs_close(0);
    int32_t res2 = fs_close(1);
    return res1 < 0 && res2 < 0;
}


int test_file_close_actual_file() {
    const int8_t* fname = "created.txt";
    int32_t fd = fs_open(fname);
    int32_t res = fs_close(fd);
    // close should succeed
    if (res < 0) return FAIL;
    // make sure file descriptor no longer in use
    if (FD_FLAG_INUSE(pcb_arr[curr_pid]->fd_arr[fd].flags)) {
		return FAIL;
    }
    return PASS;
}


// test reading an entire file with read length greater than file size
int test_read_past_file() {
    const int8_t* fname = "frame1.txt";
    int32_t fd = fs_open(fname);
    inode_t inode = fs_inode_arr[pcb_arr[curr_pid]->fd_arr[fd].inode_num];
    uint32_t file_size = inode.length;

    printf("file size: %d\n", file_size);
    int8_t buf[4096];
    int read_bytes = file_read(fd, buf, file_size+50);
    if (read_bytes == -1) {
		return FAIL;
    }
#if (GRAPHICS == 1)
    clear();
    printf("File Contents:\n%s\n", buf);
#endif
    fs_close(fd);
    return read_bytes == file_size ? PASS : FAIL;
}

// test reading a file from position after end of file
int test_read_after_eof() {
    const int8_t* fname = "frame1.txt";
    int32_t fd = fs_open(fname);
    inode_t inode = fs_inode_arr[pcb_arr[curr_pid]->fd_arr[fd].inode_num];
    uint32_t file_size = inode.length;

    int8_t buf[4096];
    int read_bytes = file_read(fd, buf, file_size+50);
    read_bytes = file_read(fd, buf, 1);
    fs_close(fd);
    return read_bytes == 0 ? PASS : FAIL;
}

// test reading 0 bytes from file
int test_read_nothing() {
    const int8_t* fname = "frame1.txt";
    int32_t fd = fs_open(fname);

    int8_t buf[4096];
    int read_bytes = file_read(fd, buf, 0);
    fs_close(fd);
    return read_bytes == 0 ? PASS : FAIL;
}

// test reading part of file from start
int test_read_file_partial_start() {
    const int8_t* fname = "frame1.txt";
    int32_t fd = fs_open(fname);

    int8_t buf[4096];
    int read_bytes = file_read(fd, buf, 100);
#if (GRAPHICS == 1)
    clear();
    printf("Firt 100 byte of file:\n%s\n", buf);
#endif
    fs_close(fd);
    return read_bytes == 100 ? PASS : FAIL;
}

// test if reading from file updates curr read position
int test_read_file_update_read_pos() {
    const int8_t* fname = "frame1.txt";
    int32_t fd = fs_open(fname);

    int8_t buf[4096];
    (void)file_read(fd, buf, 100);
    if (pcb_arr[curr_pid]->fd_arr[fd].read_pos != 100) {
		return FAIL;
    }
    
    fs_close(fd);
    return PASS;
}


// test reading big file across multiple data blocks in its entirety
int test_read_large_file() {
    const int8_t* fname = "verylargetextwithverylongname.tx";
    int32_t fd = fs_open(fname);
    if (fd == -1) {
		return FAIL;
    }
    inode_t inode = fs_inode_arr[pcb_arr[curr_pid]->fd_arr[fd].inode_num];
    uint32_t file_size = inode.length;

    printf("file size: %d\n", file_size);
    int8_t buf[2*4096];
    int read_bytes = file_read(fd, buf, file_size);
    if (read_bytes == -1) {
			return FAIL;
    }
#if (GRAPHICS == 1)
    clear();
    printf("File Contents:\n%s\n", buf);
#endif
    fs_close(fd);
    return read_bytes == file_size ? PASS : FAIL;
}

// test reading fish (large file)
int test_read_fish() {
    const int8_t* fname = "fish";
    int32_t fd = fs_open(fname);
    if (fd == -1) {
		return FAIL;
    }
    uint32_t file_size = get_file_size(fd);

    
#define FISH_SIZE 36164 // fish file is 36164 bytes long
   
    int8_t buf[FISH_SIZE + 1];
    buf[FISH_SIZE] = '\n';
    int read_bytes = file_read(fd, buf, file_size);

#if (GRAPHICS == 1)
    clear();
    printf("fish file size: %d\n", file_size);
    printf("fish header: ");
    int i;
    for (i = 0; i < 5; ++i) {
		putc(buf[i]);
    }
	putc('\n');
#endif    
    fs_close(fd);
    return read_bytes == file_size ? PASS : FAIL;
}


// test reading ls
int test_read_ls() {
    const int8_t* fname = "ls";
    int32_t fd = fs_open(fname);
    if (fd == -1) {
		return FAIL;
    }
    uint32_t file_size = get_file_size(fd);

#define LS_SIZE 5350 // ls file is 5350 bytes long
    
    int8_t buf[LS_SIZE+1];
    buf[LS_SIZE] = '\n';
    int read_bytes = file_read(fd, buf, file_size);

#if (GRAPHICS == 1)
    clear();
    printf("ls file size: %d\n", file_size);
    printf("ls contents: ");
    int i;
    for (i = 0; i < 5; ++i) {
		putc(buf[i]);
    }
    printf("\nmagic number:\n");
    for (i = LS_SIZE-50; i <= LS_SIZE; ++i) {
		putc(buf[i]);
    }
#endif
    fs_close(fd);
    return read_bytes == file_size ? PASS : FAIL;
}


// test reading grep
int test_read_grep() {
    const int8_t* fname = "grep";
    int32_t fd = fs_open(fname);
    if (fd == -1) {
		return FAIL;
    }
    uint32_t file_size = get_file_size(fd);

#define GREP_SIZE 6150 // ls file is 5350 bytes long
    
    int8_t buf[GREP_SIZE+1];
    buf[GREP_SIZE] = '\n';
    int read_bytes = file_read(fd, buf, file_size);

#if (GRAPHICS == 1)
    clear();
    printf("grep file size: %d\n", file_size);
    printf("grep contents: ");
    int i;
    for (i = 0; i < 5; ++i) {
		putc(buf[i]);
    }
    printf("\nmagic number:\n");
    for (i = GREP_SIZE-50; i <= GREP_SIZE; ++i) {
		putc(buf[i]);
    }
#endif
    fs_close(fd);
    return read_bytes == file_size ? PASS : FAIL;
}


int test_file_write() {
    int res = file_write(0, NULL, 0);
    return res == -1 ? PASS : FAIL;
}

//=====================================================================

int terminaltest()
{
	char buf[128];
	while(1)
	{
		uint32_t reading = terminal_read(1,buf,128);
		(void)terminal_write(1,buf,reading);
		if (reading > 0)
		{
			putc('\n');
		}
	}

	return PASS;
}


int test_rtc_open() {
	clear();
	rtc_open(NULL);
	int i = 0;
	for (i = 0; i < 100; i++){
		rtc_read(NULL, NULL, NULL);
		printf("%d", i);
	}
	putc('\n');
	rtc_close(NULL);

	return PASS;
}


int test_rtc_write() {
	clear();
	rtc_open(NULL);
	int i = 0;
	uint32_t freq = 2;
	for (i = 0; i < 10; i++){
		int j = 0;
		for (j = 0; j < 10; j++) {
			rtc_read(NULL, NULL, NULL);
			printf("%d", j);
		}
		putc('\n');
		freq = freq << 1; 
		rtc_write(NULL, (void*)freq, NULL);	
	}
	
	rtc_close(NULL);

	return PASS;
}

int test_rtc_not_power_2() {
	rtc_open(NULL);
	int32_t rt_val = rtc_write(NULL, (void *)5, NULL);	
	rtc_close(NULL);

	return rt_val = -1 ? PASS : FAIL;
}

int test_rtc_greater_1024() {
	rtc_open(NULL);
	int32_t rt_val = rtc_write(NULL, (void *)1030, NULL);	
	rtc_close(NULL);

	return rt_val = -1 ? PASS : FAIL;
}

//=====================================================================

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests() {
	// TEST_OUTPUT("idt_test: ", idt_test());
	// TEST_OUTPUT("kernel starting: ", kernelexist_test());
	// TEST_OUTPUT("kernel ending: ", kernelexistdone_test());
	// TEST_OUTPUT("videomemexist starting: ", videomemexist_test());
	// TEST_OUTPUT("videomemexist ending: ", videomemexistdone_test());
	// TEST_OUTPUT("kernel test lower: ", kernelexistlower_test());
	// TEST_OUTPUT("kernel test higher: ", kernelexisthigher_test());
	// TEST_OUTPUT("videomemexist test lower: ", videomemexistlower_test());
	// TEST_OUTPUT("videomemexist test higher: ", videomemexisthigher_test());
	// TEST_OUTPUT("imaginemem test: ", imaginemem_test());
	// TEST_OUTPUT("idt exception device_na: ", assertion_failure1_test());
	// TEST_OUTPUT("idt exception stack_segfault: ", assertion_failure2_test());
	// TEST_OUTPUT("idt_div_zero_trigger_test", idt_div_zero_trigger_test());
	// TEST_OUTPUT("overflow test: ", overflow_test());
	// TEST_OUTPUT("System call:", system_call_fail_test());
	// TEST_OUTPUT("zero test: ", zero());
	// TEST_OUTPUT("max test: ", max());

	/* Checkpoint 2 tests */
  
     //TEST_OUTPUT("test directory read: ", test_dir_read());
	// TEST_OUTPUT("test directory write: ", test_dir_write());
	// TEST_OUTPUT("test open nonexistent file: ", test_file_open_bad());
  //TEST_OUTPUT("test open good file: ", test_file_open_good());
	// TEST_OUTPUT("test closing stdin/stdout: ", test_file_close_defaults());
	// TEST_OUTPUT("test closing an actual file: ", test_file_close_actual_file());
	//TEST_OUTPUT("test reading an entire file past end: ", test_read_past_file());
	// TEST_OUTPUT("test update read position: ", test_read_file_update_read_pos());
	 //TEST_OUTPUT("test reading an entire very large file: ", test_read_large_file());
	// TEST_OUTPUT("test reading ls exec file", test_read_ls());
	//TEST_OUTPUT("test reading grep exec file", test_read_grep());
	// TEST_OUTPUT("test reading fish: ", test_read_fish());
	// TEST_OUTPUT("test reading a file after EOF: ", test_read_after_eof());	
	// TEST_OUTPUT("test reading 0 bytes from file: ", test_read_nothing());
	// TEST_OUTPUT("test part of file from start: ", test_read_file_partial_start());
	// TEST_OUTPUT("test writing to file: ", test_file_write());
	TEST_OUTPUT("test rtc open: ", test_rtc_open());
	//TEST_OUTPUT("test rtc write: ", test_rtc_write());
	// TEST_OUTPUT("test rtc not power 2: ", test_rtc_not_power_2());
	// TEST_OUTPUT("test rtc greater 1024: ", test_rtc_greater_1024());
	//terminaltest();
}
