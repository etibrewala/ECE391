
# mp1.S - missile-command version
# Good luck, and have fun!

.data
	
    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  = 0
	X	  = 4
	Y	  = 8
	VX	  = 12
	VY	  = 16
	DEST_X	  = 20
	DEST_Y	  = 24
	EXPLODED  = 28
	C	  = 32

    # Character to draw for an explosion - '@'

	EXPLOSION = 64
	ROW_OFFSET = 160
	COL_OFFSET = 2
	SPACECHAR = 32  # ' ' character for remove from screen
	CROSSHAIR = 43	# '+' character
	ACCESS_UPPER = 16 # shift value to access upper 16 bits
	SIZE_STRUCT = 33  # number of bytes in missile struct
	SIZE_STATUS = 4
	STAT_B1 = 1
	STAT_B2 = 2
	STAT_B3 = 4

	BASE1_1 = 3876	# First video memory location of base1
	BASE1_2 = 3878	# increments by 2 for each following char
	BASE1_3	= 3880
	BASE1_4	= 3882
	BASE1_5	= 3884

	BASE2_1	= 3916	# first video memory location of base2
	BASE2_2	= 3918
	BASE2_3 = 3920
	BASE2_4 = 3922
	BASE2_5 = 3924

	BASE3_1 = 3956	# first video memory location of base3
	BASE3_2 = 3958
	BASE3_3 = 3960
	BASE3_4 = 3962
	BASE3_5 = 3964

# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score


# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs

notifyuserflag:		.long 0x0

.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
		movl    vmem_base_addr(,1),%edx
		movb    %cl,(%edx,%eax,1)
		ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
	pushl %ebp
	movl %esp, %ebp

	pushl %ebx						# callee - save registers
	pushl %esi
	pushl %edi

	call update_missiles

	movl notifyuserflag, %ebx		# check notify_user flag set in update_missiles
	cmpl $0, %ebx
	je draw_bases
	call mp1_notify_user

draw_bases:
	movl base_alive, %ebx     		# ptr to base_alive array for drawing
	
	# ================================================================
	# DRAWING BASE 1 =================================================
	# ================================================================
	pushl %ebx
	andl $0x000000ff, %ebx			# check status of base1 by checking non zero byte
	movl %ebx, %esi
	popl %ebx

	cmpl $0, %esi
	je load_dead_base1
	
	load_alive_base1:
		movb base_pic, %cl			# move character to cl
		movl $BASE1_1, %eax			# video location to draw char
		pushl %edx					# save edx b/c gets clobbered
		call mp1_poke				# mp1_poke call to draw char
		popl %edx					# restore edx after clobber

		movb base_pic+1, %cl
		movl $BASE1_2, %eax			# (19,24)
		pushl %edx
		call mp1_poke
		popl %edx

		movb base_pic+2, %cl
		movl $BASE1_3, %eax			# (20,24)
		pushl %edx
		call mp1_poke
		popl %edx
		
		movb base_pic+3, %cl
		movl $BASE1_4, %eax			# (21,24)
		pushl %edx
		call mp1_poke
		popl %edx

		movb base_pic+4, %cl
		movl $BASE1_5, %eax			# (22,24)
		pushl %edx
		call mp1_poke
		popl %edx
		jmp draw_base_2

	load_dead_base1:
		movb dead_base_pic, %cl
		movl $BASE1_1, %eax			# (18,24)
		pushl %edx
		call mp1_poke
		popl %edx

		movb dead_base_pic+1, %cl
		movl $BASE1_2, %eax			# (19,24)
		pushl %edx
		call mp1_poke
		popl %edx


		movb dead_base_pic+2, %cl
		movl $BASE1_3, %eax			# (20,24)
		pushl %edx
		call mp1_poke
		popl %edx
		
		movb dead_base_pic+3, %cl
		movl $BASE1_4, %eax			# (21,24)
		pushl %edx
		call mp1_poke
		popl %edx

		movb dead_base_pic+4, %cl
		movl $BASE1_5, %eax			# (22,24)
		pushl %edx
		call mp1_poke
		popl %edx
	# =======================================================================
	# DRAWING BASE 2
	# =======================================================================
	draw_base_2:
		pushl %ebx
		andl $0x0000ff00, %ebx			# check status of base2
		movl %ebx, %esi
		popl %ebx

		cmpl $0, %esi
		je load_dead_base2
		
		load_alive_base2:
			movb base_pic, %cl
			movl $BASE2_1, %eax			# (38,24)
			pushl %edx
			call mp1_poke
			popl %edx

			movb base_pic+1, %cl
			movl $BASE2_2, %eax			# (39,24)
			pushl %edx
			call mp1_poke
			popl %edx


			movb base_pic+2, %cl
			movl $BASE2_3, %eax			# (40,24)
			pushl %edx
			call mp1_poke
			popl %edx
			
			movb base_pic+3, %cl
			movl $BASE2_4, %eax			# (41,24)
			pushl %edx
			call mp1_poke
			popl %edx

			movb base_pic+4, %cl
			movl $BASE2_5, %eax			# (42,24)
			pushl %edx
			call mp1_poke
			popl %edx
			jmp draw_base_3

		load_dead_base2:
			movb dead_base_pic, %cl
			movl $BASE2_1, %eax			# (38,24) video mem offset 
			pushl %edx
			call mp1_poke
			popl %edx

			movb dead_base_pic+1, %cl
			movl $BASE2_2, %eax			# (39,24)
			pushl %edx
			call mp1_poke
			popl %edx


			movb dead_base_pic+2, %cl			# third
			movl $BASE2_3, %eax			# (40,24)
			pushl %edx
			call mp1_poke
			popl %edx
			
			movb dead_base_pic+3, %cl			# fourth
			movl $BASE2_4, %eax			# (41,24)
			pushl %edx
			call mp1_poke
			popl %edx

			movb dead_base_pic+4, %cl			# fifth
			movl $BASE2_5, %eax			# (42,24)
			pushl %edx
			call mp1_poke
			popl %edx
	# =====================================================================
	# DRAW BASE 3
	# =====================================================================
	draw_base_3:
		pushl %ebx
		andl $0x00ff0000, %ebx
		movl %ebx, %esi
		popl %ebx

		cmpl $0, %esi
		je load_dead_base3
		
		load_alive_base3:
			movb base_pic, %cl
			movl $BASE3_1, %eax			# (58,24)
			pushl %edx
			call mp1_poke
			popl %edx

			movb base_pic+1, %cl
			movl $BASE3_2, %eax			# (59,24)
			pushl %edx
			call mp1_poke
			popl %edx

			movb base_pic+2, %cl
			movl $BASE3_3, %eax			# (60,24)
			pushl %edx
			call mp1_poke
			popl %edx
			
			movb base_pic+3, %cl
			movl $BASE3_4, %eax			# (61,24)
			pushl %edx
			call mp1_poke
			popl %edx

			movb base_pic+4, %cl
			movl $BASE3_5, %eax			# (62,24)
			pushl %edx
			call mp1_poke
			popl %edx
			jmp redraw_crosshairs
		
		load_dead_base3:
			movb dead_base_pic, %cl
			movl $BASE3_1, %eax			# (58,24)
			pushl %edx
			call mp1_poke
			popl %edx

			movb dead_base_pic+1, %cl
			movl $BASE3_2, %eax			# (59,24)
			pushl %edx	
			call mp1_poke
			popl %edx


			movb dead_base_pic+2, %cl
			movl $BASE3_3, %eax			# (60,24)
			pushl %edx
			call mp1_poke
			popl %edx
			
			movb dead_base_pic+3, %cl
			movl $BASE3_4, %eax			# (61,24)
			pushl %edx
			call mp1_poke
			popl %edx

			movb dead_base_pic+4, %cl
			movl $BASE3_5, %eax			# (62,24)
			pushl %edx
			call mp1_poke
			popl %edx
	# ===============================================================
	# REDRAW CROSSHAIRS
	# ===============================================================
	redraw_crosshairs:
		# Registers:
		# EAX, ECX, EDX
		movl crosshairs_y, %ecx
		movl crosshairs_x, %edx

		imull $ROW_OFFSET, %ecx
		movl %ecx, %eax
		imull $COL_OFFSET, %edx
		addl %edx, %eax
		movb $CROSSHAIR, %cl
		call mp1_poke

		popl %edi
		popl %esi
		popl %ebx

		leave
		ret

# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention

.globl mp1_ioctl
mp1_ioctl:
		# Registers
		# ECX - holds argument passed to ioctl functions
		# EDX - holds cmd to jump to

		movl 8(%esp), %edx			# cmd

		cmpl $0, %edx				# check cmd bounds
		jb return_ioctl_error
		cmpl $4, %edx
		ja return_ioctl_error

		jmp *jump_table(,%edx,4)    # jump to subfunction

		return_ioctl_error:			# return -1 if error
			movl $0, %eax
			subl $1, %eax
		
		mp1_ioctl_done:
			ret


# ----------------- Functions private to this file -------------------
# void update_missiles();
# Inputs: No inputs, function directly accesses mp1_missile_list
# Outputs: NO outputs, function directly handles linked list data and or deletion
update_missiles:
	pushl %ebp						# initialize stack/base ptrs
	movl %esp, %ebp

	pushl %eax						# callee - save registers
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi

	movl mp1_missile_list, %esi		# esi - head of missile linked list
	movl $0, notifyuserflag			# reset notify user
	
	update_missile_loop:
		cmpl $0, %esi				# checks if curr node if NULL
		je return_update_missile

		movl EXPLODED(%esi), %edi
		cmpl $0, %edi
		jne missile_explosion_protocol

		# drawing " " character in current position
		movl Y(%esi), %edx
		movl X(%esi), %ecx

		shrl $ACCESS_UPPER, %edx
		shrl $ACCESS_UPPER, %ecx	
		imull $ROW_OFFSET, %edx
		movl %edx, %eax
		imull $COL_OFFSET, %ecx
		addl %ecx, %eax
		movb $SPACECHAR, %cl
		
		pushl %edx
		call mp1_poke
		popl %edx

		# change position of missile
		movl X(%esi), %edx
		movl VX(%esi), %ecx

		addl %edx, %ecx				# store sum in ecx
		movl %ecx, X(%esi)			# update X val

		movl Y(%esi), %edx
		movl VY(%esi), %ecx

		addl %edx, %ecx				# store sum in ecx
		movl %ecx, Y(%esi)			# update Y val

		# shift bits and check for bounds
		movl X(%esi), %edx
		movl Y(%esi), %ecx

		shrl $ACCESS_UPPER, %edx				# shift to get X video mem
		shrl $ACCESS_UPPER, %ecx				# shift to get Y video mem

		cmpl $80, %edx						# bounds check for missile
		jge finish_exploding
		cmpl $0, %edx
		jl finish_exploding

		cmpl $25, %ecx
		jge finish_exploding
		cmpl $0, %ecx
		jl finish_exploding

	# bounds check complete, destination check
	destination_check:
		movl X(%esi), %edx
		movl DEST_X(%esi), %ecx
		shrl $ACCESS_UPPER, %edx
		cmpl %edx, %ecx
		jne draw_new_missile				# if X not equal, not at destination

		movl Y(%esi), %edx
		movl DEST_Y(%esi), %ecx
		shrl $ACCESS_UPPER, %edx
		cmpl %edx, %ecx
		je missile_explosion_protocol
	
	# all checks complete, draw new missile position
	draw_new_missile:
		movl X(%esi), %edx
		movl Y(%esi), %edi

		shrl $ACCESS_UPPER, %edx
		shrl $ACCESS_UPPER, %edi
		
		movl C(%esi), %ecx
		
		imull $ROW_OFFSET, %edi				# row major calcultion for video mem
		movl %edi, %eax
		imull $COL_OFFSET, %edx
		addl %edx, %eax

		pushl %edx
		call mp1_poke
		popl %edx

		# finished drawing missile char, move to next missile
		jmp iterate_to_next_missile

	missile_explosion_protocol:
		pushl %esi					# push current missile arg
		call missile_explode
		movl %eax, %ecx
		popl %esi

		cmpl $0, %ecx				# notify user check
		je decrement_explosion		# returns non-zero, notify users
		addl $1, (notifyuserflag)	# set notify user flag

	decrement_explosion:
		movl EXPLODED(%esi), %ecx	# move in EXPLODED value
		subl $1, %ecx				# decrement value
		cmpl $0, %ecx
		je finish_exploding			# if zero, erase from screen
		
		# Otherwise, draw explosion character
		movl %ecx, EXPLODED(%esi)	# reassign explosion
		
		movl X(%esi), %edx
		movl Y(%esi), %edi
		
		shrl $ACCESS_UPPER, %edx				# shift to get X video mem
		shrl $ACCESS_UPPER, %edi				# shift to get Y video mem

		imull $ROW_OFFSET, %edi
		movl %edi, %eax
		imull $COL_OFFSET, %edx
		addl %edx, %eax
		movb $EXPLOSION, %cl				# draw EXPLOSION character

		pushl %edx
		call mp1_poke
		popl %edx

		jmp iterate_to_next_missile

	finish_exploding:
		# check if current node is head
		movl X(%esi), %edx
		movl Y(%esi), %ecx

		shrl $ACCESS_UPPER, %edx
		shrl $ACCESS_UPPER, %ecx

		imull $ROW_OFFSET, %ecx
		movl %ecx, %eax
		imull $COL_OFFSET, %edx
		addl %edx, %eax
		movb $SPACECHAR, %cl
		
		pushl %edx
		call mp1_poke
		popl %edx

		# missile remove logic
		movl mp1_missile_list, %ebx
		cmpl %ebx, %esi				  # check if curr node is head
		je remove_head_node
	
	find_previous_node:				  # find previous node if not removing head node
		cmpl NEXT(%ebx), %esi
		je remove_middle_node
		movl NEXT(%ebx), %ebx
		jmp find_previous_node
	
	remove_middle_node:				  # case 1: removing middle node/tail node
		movl NEXT(%esi), %ecx			# save next node to temp
		movl %ecx, NEXT(%ebx)			# connect prev node to next node
		
		pushl %esi						# free current node
		call mp1_free
		popl %esi

		movl NEXT(%ebx), %esi			# increment curr node
		jmp update_missile_loop			# curr automatically incremented, jump to top
	
	remove_head_node:				  # case 2: remove head node
		movl NEXT(%esi), %edx		  # save next node to register 
		movl %edx, mp1_missile_list	  # set register to mp1_missile_list

		pushl %esi					  # free current node
		call mp1_free
		popl %esi

		movl mp1_missile_list, %esi		# make curr node head
		jmp update_missile_loop			# curr automatically incremented, jump to top

	iterate_to_next_missile:
		movl NEXT(%esi), %esi			# incerment curr node
		jmp update_missile_loop

	return_update_missile:				# restore saved registers
		popl %edi
		popl %esi
		popl %edx
		popl %ecx
		popl %ebx
		popl %eax

		leave
		ret
# int mp1 ioctl startgame (unsigned long ignore);
# Inputs: Random input, not used by function
# Outputs: No output, intializes all game variables
# Description: Start game is used to initialize all the game variables and center crosshair.
mp1_ioctl_startgame:
	pushl %ebp
	movl %esp, %ebp
	
	pushl %ebx
	pushl %esi
	pushl %edi

	# initialize all the variables
	movl $40, crosshairs_x		 # crosshairs_x = 40
	movl $12, crosshairs_y		 # crosshairs_y = 12

	movl $0, mp1_score			 # score = 0

	movl $0x00ffffff, base_alive # initialize base_alive array (all non zeros)

	movl $0, mp1_missile_list	 # mp1_missile_list = NULL

	return_start_game:			 # restore callee registers and leave/ret ioctl
		pushl %edi
		pushl %esi
		pushl %ebx
		leave
		ret

# int mp1 ioctl addmissile (struct missile* user missile);
# Input: pointer to user missile being added to list
# Output: returns 0 if node successfully added, -1 if adding node fails
# Description: Takes ptr to a user missile stores it in memory and adds it to mp1_missile_list.
mp1_ioctl_addmissile:
	# Arguments: pointer to user_missile
	pushl %ebp
	movl %esp, %ebp

	pushl %ebx						# callee saved registers
	pushl %esi
	pushl %edi

	movl 8(%ebp), %ebx				# ebx - ptr to user_missile

	pushl $SIZE_STRUCT   			# argument for malloc (n bytes)
	call mp1_malloc					# allocate space in memory, returns ptr to that space
	addl $4, %esp					# pop junk register
	
	cmpl $0, %eax
	je return_addmissile_fail

	movl %eax, %esi					# save memory pointer

	pushl $SIZE_STRUCT				# argument n bytes
	pushl %ebx						# argument *user_missile
	pushl %esi						# argument *to allocated memory
	call mp1_copy_from_user			# returns 0 is successful
	popl %esi
	addl $8, %esp

	cmpl $0, %eax					# checks if memcpy() successful
	jne memory_copy_fail

	addmissile_linkedlist:
		movl mp1_missile_list, %ecx		# ecx is mp1_missile_list ptr
		movl %ecx, NEXT(%esi)			# next ptr of new node is curent head
		movl %esi, mp1_missile_list		# reassign mp1_missile_list as head

		movl $0, %eax					# return 0 for success
		jmp return_addmissile	

	memory_copy_fail:
		pushl %esi						# mp1_free argument
		call mp1_free
		popl %esi

	return_addmissile_fail:
		movl $-1, %eax			# return -1 if fails
	
	return_addmissile:
		popl %edi			    # restore callee registers
		popl %esi
		popl %ebx

		leave
		ret
# int mp1 ioctl movexhairs (unsigned long xhair delta packed);
# Input: 1 16-bit unsigned long that gives X and Y coordinates of crosshairs.
# Output: Always returns 0, never fails.
# Description: Updates the position of the crosshair by changing the X and Y video position.
mp1_ioctl_movexhairs:
	# Registers:
	# EBX - random register
	# ECX - holds input argument
	# EDX - random register
	# ESI - holds crosshairs_y
	# EDI - holds crosshairs_x

	pushl %ebp
	movl %esp, %ebp

	pushl %esi						# callee save registers
	pushl %edi						
	pushl %ebx	
	pushl %edx

	movl crosshairs_y, %esi  		# esi <-- crosshairs_y
	movl crosshairs_x, %edi			# edi <-- crosshairs_x

	movswl 8(%ebp), %ebx
	movswl 10(%ebp), %edx

	# ESI - holds crosshairs_y
	# EDI - holds crosshairs_x

	pushl %esi					# push curr Y to stack
	pushl %edi					# push curr X to stack

	addl %esi, %edx
	addl %edi, %ebx

	# bounds check for crosshair
	cmpl $80, %ebx					# x >= 80 
	jge dont_update_crosshairs
	cmpl $0, %ebx					# x < 80
	jl dont_update_crosshairs
	cmpl $25, %edx					# Y >= 25
	jge dont_update_crosshairs
	cmpl $0, %edx					# Y < 0
	jl dont_update_crosshairs

	update_crosshairs:
		popl %edi				# restore X
		popl %esi 				# restore Y

		pushl %edx				# save current Y (in case dont_update)
		pushl %ebx				# current X

		movl %esi, %edx				# temp store crosshairs_y in edx
		movl %edi, %ebx				# temp store crosshairs_x in ebx

		imull $ROW_OFFSET, %edx		# row/major order to calculate video mem location
		movl %edx, %eax				
		imull $COL_OFFSET, %ebx
		addl %ebx, %eax				# store in eax for mp1_poke

		movb $SPACECHAR, %cl

		pushl %edx
		call mp1_poke
		popl %edx

		jmp draw_crosshairs

	dont_update_crosshairs:
		popl %edi					# restore curr X
		popl %esi					# restore curr Y

		movl %esi, %edx				# temp store crosshairs_y in edx
		movl %edi, %ebx				# temp store crosshairs_x in ebx

		imull $ROW_OFFSET, %edx
		movl %edx, %eax				
		imull $COL_OFFSET, %ebx				
		addl %ebx, %eax				# sum of video mem in eax

		movb $CROSSHAIR, %cl

		pushl %edx
		call mp1_poke
		popl %edx
		jmp done_movexhairs

	draw_crosshairs:
		# ESI has new Y value
		# EDI has new X value
		
		popl %edi
		popl %esi

		movl %esi, %edx				# temp store crosshairs_y in edx
		movl %edi, %ebx				# temp store crosshairs_x in ebx

		imull $ROW_OFFSET, %edx
		movl %edx, %eax	
		imull $COL_OFFSET, %ebx
		addl %ebx, %eax				# sum of video mem in eax

		movb $CROSSHAIR, %cl

		pushl %edx
		call mp1_poke
		popl %edx

		movl %edi, crosshairs_x
		movl %esi, crosshairs_y

	done_movexhairs:				# restore callee registers
		popl %edx
		popl %ebx
		popl %edi
		popl %esi

		movl $0, %eax				# return 0

    	leave
		ret
# int mp1 ioctl getstatus (unsigned long* user status);
# Input: Unsigned long ptr to a 32-bit integer in userspace.
# Output: Returns 0 if copy to userspace successful, -1 if fails.
# Description: This function gets base status and score of game and places the status in memory in userspace.
mp1_ioctl_getstatus:

		pushl %ebp						# initialize stack/base ptr
		movl %esp, %ebp
	
		pushl %esi						# callee save registers
		pushl %edi						
		pushl %ebx
		pushl %ecx

		movl mp1_score, %esi
		movl base_alive, %ecx
		movl $0, %ebx					# register for status

		andl $0x000000ff, %ecx
		cmpl $0, %ecx
		je check_city_2
		addl $STAT_B3, %ebx					# add 100
	
	check_city_2:
		movl base_alive, %ecx
		andl $0x0000ff00, %ecx
		cmpl $0, %ecx
		je check_city_3
		addl $STAT_B2, %ebx					# add 10
	
	check_city_3:
		movl base_alive, %ecx
		andl $0x00ff0000, %ecx
		cmpl $0, %ecx
		je score_handling
		addl $STAT_B1, %ebx					# add 001
	
	score_handling:
		shll $ACCESS_UPPER, %ebx
		addw %si, %bx			    # zero extend and append score to lower 16				

	pushl $SIZE_STATUS
	call mp1_malloc
	addl $4, %esp

	movl %ebx, (%eax)
	movl %eax, %edi

	movl 8(%ebp), %ecx				# move arg pointer to ecx

	pushl $SIZE_STATUS				# n argument on function
	pushl %edi						# *from argument on function
	pushl %ecx						# *to argument on function
	call mp1_copy_to_user
	popl %ecx						# restore args to junk reg
	addl $8, %esp

	cmpl $0, %eax					# check return val mp1_copy_to_user
	je return_getstatus

	movl $0xffffffff, %eax			# if error return negative 1

	return_getstatus:
		popl %ecx
		popl %ebx
		popl %edi
		popl %esi

		leave
		ret

# int mp1 ioctl endgame (unsigned long ignore);
# Input: Random/garbage input
# Output: No output, function directly deletes missiles.
# Description: Iterates through linked list and deletes all the nodes.
mp1_ioctl_endgame:

	movl mp1_missile_list, %ebx	        # ebx is now ptr to mp1_missile_list
	
	free_memory_loop:
		movl NEXT(%ebx), %esi			# temp = missile -> next

		pushl %ebx						# save temp
		call mp1_free					# free temp
		popl %ebx

		movl %esi, %ebx					# missile = temp
		cmpl $0, %ebx					# if null, at end of list, return
		jne free_memory_loop
		ret

jump_table:
.long mp1_ioctl_startgame, mp1_ioctl_addmissile, mp1_ioctl_movexhairs, mp1_ioctl_getstatus, mp1_ioctl_endgame
