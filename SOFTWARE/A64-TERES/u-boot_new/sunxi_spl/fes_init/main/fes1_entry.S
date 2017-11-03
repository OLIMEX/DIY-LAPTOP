



.globl _start
_start: b	reset

reset:
	stmfd sp!, {lr}
	bl  clear_bss
	bl  main
	ldmfd sp!, {lr}
	mov pc, lr

clear_bss:
	ldr	r0, =__bss_start
	ldr	r1, =__bss_end

	mov	r2, #0x00000000		/* clear			    */

clbss_l:
	str	r2, [r0]		/* clear loop...		    */
	add	r0, r0, #4
	cmp	r0, r1
	bne	clbss_l

	mov pc, lr
