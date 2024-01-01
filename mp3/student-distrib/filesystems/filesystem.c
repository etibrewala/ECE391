#include "filesystem.h"

/* static jump tables for each device type's operations */

static struct file_ops rtc_ops = {
    .read = rtc_read,
    .write = rtc_write,
    .open = rtc_open,
    .close = rtc_close
};

static struct file_ops dir_ops = {
    .read = directory_read,
    .write = directory_write,
    .open = directory_open,
    .close = directory_close
};

static struct file_ops fops = {
    .read = file_read,
    .write = file_write,
    .open = file_open,
    .close = file_close
};

static struct file_ops stdin_ops = {
    .read = terminal_read,
    .write = badcall_write,
    .open = badcall_open,
    .close = badcall_close
};

static struct file_ops stdout_ops = {
    .read = badcall_read,
    .write = terminal_write,
    .open = badcall_open,
    .close = badcall_close
};

static struct file_ops badcall_ops = {
    .read = badcall_read,
    .write = badcall_write,
    .open = badcall_open,
    .close = badcall_close
};

/* helper functions */

static int32_t find_file_index(const int8_t* fname);
static int32_t find_open_fd(void);
static int8_t filenames_equal(const int8_t* a, const int8_t* b);

boot_block_t* fs_boot_block = NULL;
inode_t* fs_inode_arr = NULL;
uint32_t fs_data_blocks = NULL;

uint32_t directory_index = 0;   //index to iterate through subsequent directory_read calls


void init_ext2_filesys(uint32_t boot_block_start) {
    fs_boot_block = (boot_block_t*) boot_block_start;     // cast boot block pointer to struct pointer
    fs_inode_arr = (inode_t*)((uint32_t)fs_boot_block + sizeof(boot_block_t));  // start of inode array in memory after boot block
    fs_data_blocks = (uint32_t)fs_inode_arr + (sizeof(inode_t) * (fs_boot_block->inode_count));  // start of data blocks in memory
}


/* read_dentry_by_name(int8_t* fname, dentry_t dentry)
 * DESCRIPTION: Search through the directory's files for a file with the given name.
 *              If the file is found, then read it's info to given dentry.
 * INPUTS:      fname  - name of the file to search for
 *              dentry - directory entry to read contents of file into. Acts as a buffer to write to.
 * OUTPUTS:     none
 * RETURNS 0 on successfully finding and reading file, -1 otherwise.
 * SIDE EFFECTS: Writes to dentry pointed to by input dentry 
 */
int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry){
    if (fname == NULL || dentry == NULL) return -1;
    int32_t dir = find_file_index(fname);
    if (dir < 0) {
	    return dir;
    }
    return read_dentry_by_index(dir, dentry);
}


/* read_dentry_by_index
* DESCRIPTION:  reads each element of the directory entry at the given index in the boot block
* INPUTS:       uint32_t index - index number of where to find the direcory in the boot block
*               dentry_t* dentry - pointer to a dentry_t variable to write the contents of file to
* OUTPUTS:      integer that determines validity of read  
* RETURN VALUE: 0 if read is successful, -1 is read fails
* SIDE EFFECTS: populates a temporary dentry_t variable with the specifed directory contents
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    // do we want to copy contents or just have it point there?
    if (dentry == NULL) return -1;
    memcpy(dentry->filename, fs_boot_block->dir_entries[index].filename, FILENAME_LEN);
    dentry->filetype = fs_boot_block->dir_entries[index].filetype;
    dentry->inode_num = fs_boot_block->dir_entries[index].inode_num;
    memcpy(dentry->reserved, fs_boot_block->dir_entries[index].reserved, DIR_ENTRY_RESERVED);
    return 0;
}


/* read_data
* DESCRIPTION:  reads the data in each file by parsing through the data blocks.
* INPUTS:       uint32_t inode - inode number of the file to read from
*               uint32_t offset - offset into data to start reading from
*               int8_t* buf - buffer to store data read
*               uint32_t length - number of bytes of data being read
* OUTPUTS:      none
* RETURN VALUE: -1 if bad data block, 0 if end of file, otherwise length of bytes read by the function
* SIDE EFFECTS: none
*/
int32_t read_data(uint32_t inode, uint32_t offset, int8_t* buf, int32_t length){
    if (buf == NULL) return -1;
    uint32_t length_curr_file = fs_inode_arr[inode].length; /* num bytes of file */
    uint32_t curr_block;                                    /* curr block to read from in inode */
    uint32_t curr_byte;                                     /* curr byte to read from in data block */
    uint32_t block_number;				    /* block number where data is stored for given inode */
    int32_t curr_read;					    /* pointer to start of where we want to read */
    uint32_t nblock;					    /* number of blocks read covers */
    uint32_t write_pos;					    /* curr position to start writing in buffer (number of bytes written so far) */
    uint32_t return_val;                                    /* return value used to return either 0 (if end of file reached) or number of bytes read*/
    int i;						    /* loop counter */

    if (inode >= fs_boot_block->inode_count) {
	    return -1;
    }
    if (offset >= length_curr_file) {
	    // offset if beyond end of file
	    return 0;
    }
    if (length+offset > length_curr_file) { // can't read more than the file contains
	    length = length_curr_file - offset;
        return_val = 0;
    } else {
        return_val = length;
    }

    nblock = (length+(offset % DATABLOCK_SIZE)) / DATABLOCK_SIZE + 1;
    curr_block = offset / 4096;
    curr_byte = offset % 4096;
    write_pos = 0;
    block_number = fs_inode_arr[inode].data_blocks[curr_block];
    curr_read = (int32_t)fs_data_blocks + (block_number * DATABLOCK_SIZE) + curr_byte;
    // handle reading first block where we may not be reading the entire thing
    if (DATABLOCK_SIZE - curr_byte > length) {
        memcpy(buf, (uint32_t*)curr_read, length);
	    return length;
    }
    memcpy(buf, (uint32_t*)curr_read, DATABLOCK_SIZE - curr_byte); // read till end of first datablock
    write_pos = DATABLOCK_SIZE - curr_byte;
    curr_byte = 0; // at start of next block
    curr_block++;
    block_number = fs_inode_arr[inode].data_blocks[curr_block]; //get curr block num
    curr_read = (int32_t)fs_data_blocks + (block_number * DATABLOCK_SIZE);
    for (i = 1; i < nblock-1; i++) {
        // read intermediary blocks in their entirety
        memcpy(buf + write_pos, (uint32_t*)curr_read, DATABLOCK_SIZE);
        write_pos += DATABLOCK_SIZE;
        curr_block++;
        block_number = fs_inode_arr[inode].data_blocks[curr_block];
        curr_read = (int32_t)fs_data_blocks + (block_number * DATABLOCK_SIZE);
    }
    // handle reading last block where we may not be reading the entire thing (like the first one)
    memcpy(buf + write_pos, (uint32_t*)curr_read, length - write_pos); 
    
    return return_val;      // returns number of bytes copied into buffer
}


int32_t fs_close(int32_t fd) {
    if (fd <= 1 || fd >= MAXFILES_PER_TASK || !FD_FLAG_INUSE(pcb_arr[curr_pid]->fd_arr[fd].flags)) // can't close stdin/stdout (or invalid fd)
	    return -1;
    UNSET_FD_FLAG_INUSE(pcb_arr[curr_pid]->fd_arr[fd].flags);
    return pcb_arr[curr_pid]->fd_arr[fd].ops_jtab.close(fd);
}


int32_t fs_open(const int8_t* fname) {
    if (fname == NULL || strlen(fname) > FILENAME_LEN) return -1;
    int32_t open_fd = find_open_fd();
    if (open_fd < 0) {
	    return -1;
    }

    dentry_t opened_file;
    int32_t res = read_dentry_by_name(fname, &opened_file);
    if (res < 0) {
	    return -1;
    }

    // fill entry in fd array
    SET_FD_FLAG_INUSE(pcb_arr[curr_pid]->fd_arr[open_fd].flags);
    pcb_arr[curr_pid]->fd_arr[open_fd].inode_num = opened_file.inode_num;
    pcb_arr[curr_pid]->fd_arr[open_fd].read_pos = 0;

    // fill operations jump table
    switch (opened_file.filetype) {
	case (DEVICE):
	    fill_rtc_ops(&pcb_arr[curr_pid]->fd_arr[open_fd].ops_jtab);
	    res = rtc_open(fname);
	    break;
	case (DIRECTORY):
	    fill_dir_ops(&pcb_arr[curr_pid]->fd_arr[open_fd].ops_jtab);
	    res = directory_open(fname);
	    break;
	case (FILE):
	    fill_file_ops(&pcb_arr[curr_pid]->fd_arr[open_fd].ops_jtab);
	    res = file_open(fname);
	    break;
    }
    if (res < 0) {
	return -1;
    }
    return open_fd;
}


int32_t fs_read(int32_t fd, void* buf, int32_t nbytes) {
    if (buf == NULL || fd < 0 || fd >= MAXFILES_PER_TASK || !FD_FLAG_INUSE(pcb_arr[curr_pid]->fd_arr[fd].flags)) {
	return -1;
    }
    int32_t read_bytes = pcb_arr[curr_pid]->fd_arr[fd].ops_jtab.read(fd, buf, nbytes);
    return read_bytes;
}


int32_t fs_write(int32_t fd, const void* buf, int32_t nbytes) {
    if (buf == NULL || fd < 0 || fd >= MAXFILES_PER_TASK || !FD_FLAG_INUSE(pcb_arr[curr_pid]->fd_arr[fd].flags))
      return -1;
    return pcb_arr[curr_pid]->fd_arr[fd].ops_jtab.write(fd, buf, nbytes);
}


/* int32_t directory_open(uint32_t)
* DESCRIPTION:  opens a directory file based on the name
* INPUTS:       const int8_t* fname - name of the directory to open
* OUTPUTS:      none
* RETURN VALUE: returns 0, already in directory
* SIDE EFFECTS: none
*/
int32_t directory_open(const int8_t* fname) {
    // does nothing... nothing special needs to be done for directories
    return 0;
}


/* int32_t directory_read(int8_t*, int8_t*, uint32_t)
* DESCRIPTION:  Read next entry in the current directory.
* INPUTS:       fd      - fd of directory to read
*               buf     - the buffer to write contents to
*               nbytes  - number of bytes to read
*               
* OUTPUTS:      none
* RETURN VALUE: return number of bytes read, return -1 on error
* SIDE EFFECTS: 
*/
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
    if (buf == NULL || fd < 0 || fd >= MAXFILES_PER_TASK || !FD_FLAG_INUSE(pcb_arr[curr_pid]->fd_arr[fd].flags)) return -1;
    dentry_t directory_file;
    /* Checks if end of directory is reached on read and returns 0*/
    if (pcb_arr[curr_pid]->fd_arr[fd].read_pos >= fs_boot_block->direntry_count) {
        return 0;
    }

    /* Populates the buffer with the corresponding file name at the given directory index form the given index*/
    if(read_dentry_by_index(pcb_arr[curr_pid]->fd_arr[fd].read_pos, &directory_file) == 0) {
        strncpy(buf, directory_file.filename, FILENAME_LEN);
    } else {
        return -1;
    }

    /* Increments the variable that keeps track of the position in the directory for subsequent reads*/
    pcb_arr[curr_pid]->fd_arr[fd].read_pos++;

    /* returns the length of the filename */
    return FILENAME_LEN;
}


/* directory_write
* DESCRIPTION:  write function for directories in file system, does nothing in our OS
* INPUTS:       ignore
* OUTPUTS:      none
* RETURN VALUE: -1
* SIDE EFFECTS: none
*/
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}


/* directory_close
* DESCRIPTION:  closes the directory, does nothing in our OS
* INPUTS:       none
* OUTPUTS:      returns whether or close was successful
* RETURN VALUE: 0 if file opened, -1 otherwise.
* SIDE EFFECTS: none
*/
int32_t directory_close(int32_t fd) {
    if (fd <= 1 || fd >= MAXFILES_PER_TASK || !FD_FLAG_INUSE(pcb_arr[curr_pid]->fd_arr[fd].flags)) // can't close stdin/stdout (or invalid fd)
	return -1;
    return 0;
}


/* file_open
* DESCRIPTION:  opens the file
* INPUTS:       const int8_t* fname - file name
* OUTPUTS:      returns whether or file open was successful
* RETURN VALUE: 0 if file opened, -1 otherwise.
* SIDE EFFECTS: none
*/
int32_t file_open(const int8_t* fname) {
    // does nothing... nothing special to do for files
    return 0;
}


/* file_read(int32_t, int8_t*, int32_t)
 * DESCRIPTION: Reads nbytes from file associated with given fd into given buffer
 * INPUTS:      fd    - File descriptor for file to read
 *              buf   - buffer to read into
 *              nbytes - number of bytes to read
 * OUTPUTS: none
 * RETURNS: If the initial read position is past EOF return 0, -1 if read failrs, else the number of bytes read.
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    if (buf == NULL || fd < 0 || fd >= MAXFILES_PER_TASK || !FD_FLAG_INUSE(pcb_arr[curr_pid]->fd_arr[fd].flags)) return -1;
    uint32_t inode = pcb_arr[curr_pid]->fd_arr[fd].inode_num;
    uint32_t file_size = fs_inode_arr[inode].length; // size of file in bytes
    
    /* if 0 bytes were given to read, return 0 */
    if (nbytes == 0) {
        return 0;
    }
    /* if curr read pos beyond end of file return 0 */
    if (pcb_arr[curr_pid]->fd_arr[fd].read_pos >= file_size) {
	return 0;
    }
    
    /* Reads data and returns number of bytes read if successful */
    int32_t read_bytes = read_data(inode, pcb_arr[curr_pid]->fd_arr[fd].read_pos, buf, nbytes);
    if (read_bytes == -1) { // read failed
        return -1;
    }

    /* Increments the variable that keeps track of the position in the file for subsequent reads*/
    if (read_bytes == 0) {
        // reached end of file. Set file position past end of file
        read_bytes = file_size - pcb_arr[curr_pid]->fd_arr[fd].read_pos;
        pcb_arr[curr_pid]->fd_arr[fd].read_pos = file_size+1;
    } else {
        // otherwise move read position forward
        pcb_arr[curr_pid]->fd_arr[fd].read_pos += read_bytes;
    }
    return read_bytes;
}

/* file_write
* DESCRIPTION:  write function for files in file system, does nothing
* INPUTS:       none
* OUTPUTS:      returns whether or not write was successful 
* RETURN VALUE: -1
* SIDE EFFECTS: none
*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    // read only... do nothing for now
    return -1;
}


/* file_close
* DESCRIPTION:  closes file in file system, does nothing
* INPUTS:       int32_t fd - file descriptor
* OUTPUTS:      returns whether or not close was successful
* RETURN VALUE: 0 if successful, -1 otherwise
* SIDE EFFECTS: none
*/
int32_t file_close(int32_t fd) {
    if (fd < 0 || fd >= MAXFILES_PER_TASK) return -1;
    return 0;
}

///////////// HELPER FUNCTIONS ////////////////////////

/* find_file_index(int8_t* fname)
 * DESCRIPTION: Find the index of file with given name in filesystem.
 * INPUTS:      fname  - name of the file to search for
 * RETURNS:     0 on successfully finding file, -1 otherwise.
 */
int32_t find_file_index(const int8_t* fname) {
    if (fname == NULL) return -1;
    uint32_t dir;
    for (dir = 0; dir < fs_boot_block->direntry_count; dir++) {
        if (filenames_equal(fname, fs_boot_block->dir_entries[dir].filename)) {
            return dir;
        }
    }
    return -1;
}


/* int32_t find_open_fd(void)
 * DESCRIPTION: Find an available file descripter (int) if one exists.
 * INPUTS: none
 * OUTPUTS: none
 * RETURNS: An available fd if one is avaiable, otherwise -1.
 * SIDE EFFECTS: none
 */
int32_t find_open_fd() {
    int i;
    for (i = 0; i < MAXFILES_PER_TASK; ++i) {
        if (!FD_FLAG_INUSE(pcb_arr[curr_pid]->fd_arr[i].flags)) {
            return i;
        }
    }
    return -1;
}

/* int8_t filenames_equal(uint8_t* a, uint8_t* b)
 * Inputs:        a = destination of string a
 *                b = destination of string b
 * Return Value:  Nonzero value if the two strings are equal, otherwise 0.
*/
int8_t filenames_equal(const int8_t* a, const int8_t* b) {
    if (a == NULL || b == NULL) return -1;
    uint32_t a_len = min(strlen(a), FILENAME_LEN);
    uint32_t b_len = min(strlen(b), FILENAME_LEN);
    return (a_len == b_len) && (strncmp(a, b, a_len) == 0);
}


/* get_file_size
* DESCRIPTION:  gets file size
* INPUTS:       int32_t fd - file descriptor pointing to open file
* OUTPUTS:      none
* RETURN VALUE: returns file size
* SIDE EFFECTS: none
*/
uint32_t get_file_size(int32_t fd) {
    return fs_inode_arr[pcb_arr[curr_pid]->fd_arr[fd].inode_num].length;
}


////////////// BAD CALLS (dummy functions) ////////////////

/* badcall_read
 * DESCRIPTION: A bad function call.
 * INPUTS:      ignore
 * OUTPUTS:     none
 * RETURNS:     -1
 * SIDE EFFECTS: none
 */
int32_t badcall_read(int32_t fd, void* buf, int32_t nbytes) {
	return -1;
}

int32_t badcall_write(int32_t fd, const void* buf, int32_t nbytes) {
	return -1;
}

int32_t badcall_open(const int8_t* fname) {
	return -1;
}

int32_t badcall_close(int32_t fd) {
	return -1;
}

////////////// Helpers to fill file operation jump tables ////////////////

void fill_rtc_ops(struct file_ops* ops_jtab) {
    *ops_jtab = rtc_ops;
}

void fill_dir_ops(struct file_ops* ops_jtab) {
    *ops_jtab = dir_ops;
}

void fill_file_ops(struct file_ops* ops_jtab) {
    *ops_jtab = fops;
}

void fill_stdin_ops(struct file_ops* ops_jtab) {
    *ops_jtab = stdin_ops;
}

void fill_stdout_ops(struct file_ops* ops_jtab) {
    *ops_jtab = stdout_ops;
}

void fill_badcall_ops(struct file_ops* ops_jtab) {
    *ops_jtab = badcall_ops;
}
