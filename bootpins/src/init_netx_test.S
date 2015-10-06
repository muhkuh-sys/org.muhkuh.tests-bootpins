
	.section .init_code, "ax"
	.arm

	.extern test

	.global start
	.global stack_top

@--------------------------------------

  .func start_init_s

start:
	stmfd   sp!, {r0, r1, r2, r4, r5, lr}

	@ Save pointer to the netx parameter block in r5.
	mov     r5, r0

	@ Save debug console stack position in R4.
	mov     r4, sp

	@ Set own stack position.
	ldr     r2, =stack_top
	mov     sp, r2

	@ Clear the .bss section.
	mov     r0, #0
	ldr     r1, =__bss_start__
	ldr     r2, =__bss_end__
clear_bss:
	cmp     r1, r2
	strlo   r0, [r1], #4
	blo     clear_bss

	@ Call the C routine.
	mov     r0, r5
	blx     test

	@ Store the result in the first element of the parameter block.
	str     r0, [r5]

	@ Restore the debug console stack position.
	mov     sp, r4

	@ Return to the ROM code.
	@ NOTE: this also does a 16/32 bit exchange.
	ldmfd   sp!, {r0, r1, r2, r4, r5, pc}

  .endfunc

@--------------------------------------

  .end

