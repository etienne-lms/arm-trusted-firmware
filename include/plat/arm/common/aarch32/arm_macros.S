/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __ARM_MACROS_S__
#define __ARM_MACROS_S__

#include <gic_common.h>
#include <gicv2.h>
#include <platform_def.h>

.section .rodata.gic_reg_name, "aS"

gicc_regs:
	.asciz "gicc_hppir", "gicc_ahppir", "gicc_ctlr", ""

gicd_pend_reg:
	.asciz "gicd_ispendr regs (Offsets 0x200 - 0x278)\n"	\
		" Offset:\t\t\tvalue\n"
newline:
	.asciz "\n"
spacer:
	.asciz ":\t\t0x"

	/* ---------------------------------------------
	 * The below utility macro prints out relevant GIC
	 * registers whenever an unhandled exception is
	 * taken in BL31 on ARM standard platforms.
	 * Expects: GICD base in x16, GICC base in x17
	 * Clobbers: r0 - r10, sp
	 * ---------------------------------------------
	 */
	.macro arm_print_gic_regs
print_gicv2:
	/* Load the gicc reg list to x6 */
	adr	r6, gicc_regs
	/* Load the gicc regs to gp regs used by str_in_crash_buf_print */
	mov_imm	r7, GICC_BASE
	ldr	r8, [r7, #GICC_HPPIR]
	ldr	r9, [r7, #GICC_AHPPIR]
	ldr	r10, [r7, #GICC_CTLR]
	/* Store to the crash buf and print to console */
	bl	str_in_crash_buf_print

print_gic_common:
	/* Print the GICD_ISPENDR regs */
	mov_imm	r6, GICD_BASE
	add	r7, r6, #GICD_ISPENDR
	adr	r4, gicd_pend_reg
	bl	asm_print_str
gicd_ispendr_loop:
	sub	r4, r7, r6
	cmp	r4, #0x280
	b.eq	exit_print_gic_regs
	bl	asm_print_hex

	adr	r4, spacer
	bl	asm_print_str

	ldr	r4, [x7], #8
	bl	asm_print_hex

	adr	r4, newline
	bl	asm_print_str
	b	gicd_ispendr_loop
exit_print_gic_regs:
	.endm

#endif /* __ARM_MACROS_S__ */

