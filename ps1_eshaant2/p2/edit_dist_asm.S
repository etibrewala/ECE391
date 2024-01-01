/*
 * tab:2
 *
 * edit_dist_asm.S - inplementation of computing edit distance between two (linked) lists
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Xiang Li
 * Version:       1
 * Creation Date: Sat Aug 20 2022
 * History:
 *    XL    1    Sat Aug 20 2022
 *        First written.
*/

.data
  UNUSED  = 0
  VAL   = 4
  NEXT = 8

.text
.global edit_dist_asm

# Edit Dist ASM (Assembly)
# Declaration is in edit_dist.h
#
# Registers:
#   EAX - Return Value
#   EBX - root1
#   ESI - root2
#   you may choose how to store your local/temp variables
#

edit_dist_asm:
  pushl %ebp        
  movl %esp, %ebp
  # --- STUDENT CODE STARTS HERE ---
  pushl %esi            # push root2 to stack
  pushl %ebx            # push root1 to stack
  pushl %edx            # push recursive variable to stack
  subl $12, %esp        # make room for local variables

  movl 8(%ebp), %ebx    # assign old root1
  movl 12(%ebp), %esi   # assign old root2

  cmpl $0, %esi         # check if root2 is null
  jne ROOT1_NULL_CHECK  # jmp to root1 null check if root!=NULL
  cmpl $0, %ebx         # check if root1 is null
  je RETURN_0           # (root1==NULL && root2==NULL) ret 0

ROOT1_NULL_CHECK:
  cmpl $0, %ebx
  je RETURN_ROOT2_NEXT

ROOT2_NULL_CHECK:
  cmpl $0, %esi
  je RETURN_ROOT1_NEXT

ROOT_VAL_CHECK:
  movl VAL(%esi), %edx
  cmpl VAL(%ebx), %edx
  je RETURN_ROOT1_ROOT2_NEXT

INSERT:
  pushl %esi
  pushl NEXT(%ebx)
  call edit_dist_asm
  popl %edx
  popl %edx
  movl %eax, -16(%ebp)

DELETE:
  pushl NEXT(%esi)
  pushl %ebx
  call edit_dist_asm
  popl %edx
  popl %edx
  movl %eax, -20(%ebp)

REPLACE:
  pushl NEXT(%esi)
  pushl NEXT(%ebx)
  call edit_dist_asm
  popl %edx
  popl %edx
  movl %eax, -24(%ebp)

INSERT_DELETE:
  movl -20(%ebp), %edx
  cmpl -16(%ebp), %edx
  ja DELETE_REPLACE
  movl -24(%ebp), %edx
  cmpl -16(%ebp), %edx
  jbe RETURN_INSERT1

DELETE_REPLACE:
  movl -20(%ebp), %edx
  cmpl -24(%ebp), %edx
  jbe RETURN_DELETE1
  jmp RETURN_REPLACE1

RETURN_ROOT2_NEXT:               # 1 + edit_dist_c(root1, root2->next);
  pushl NEXT(%ESI)
  pushl %EBX
  CALL edit_dist_asm
  popl %EDX             # throw away the two arguments from stack
  popl %EDX
  ADDL $1, %EAX
  JMP DONE

RETURN_ROOT1_NEXT:               # 1 + edit_dist_c(root1->next, root2);
  pushl %ESI
  pushl NEXT(%EBX)
  CALL edit_dist_asm
  popl %EDX             # throw away the two arguments from stack
  popl %EDX
  ADDL $1, %EAX
  JMP DONE

RETURN_ROOT1_ROOT2_NEXT:               # edit_dist_c(root1->next, root2->next);
  pushl NEXT(%ESI)
  pushl NEXT(%EBX)
  CALL edit_dist_asm
  popl %EDX             # throw away the two arguments from stack
  popl %EDX
  JMP DONE

RETURN_0:
  movl $0, %eax
  jmp DONE

RETURN_INSERT1:
  movl -16(%ebp), %eax
  incl %eax
  jmp DONE

RETURN_DELETE1:
  movl -20(%ebp), %eax
  incl %eax
  jmp DONE

RETURN_REPLACE1:
  movl -24(%ebp), %eax
  incl %eax
  jmp DONE

DONE:
  addl $12, %esp
  popl %edx
  popl %edx
  popl %edx



  
  # --- STUDENT CODE ENDS HERE ---
  leave
  ret



