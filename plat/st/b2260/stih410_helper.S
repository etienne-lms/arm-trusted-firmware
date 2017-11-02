/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <platform_def.h>

	.globl	platform_mem_init
	.globl	plat_report_exception
	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_reset_handler
	.globl	platform_get_core_pos
	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc

func platform_mem_init
	/* nothing to do */
	bx lr
endfunc platform_mem_init

func plat_report_exception
	/* TODO */
	bx lr
endfunc plat_report_exception

func plat_reset_handler
	/* b2260 specific: switch off MMU/Cache set by "ROM" bootloader */
	ldcopr	r0, SCTLR
	bic r0, #(SCTLR_M_BIT | SCTLR_C_BIT)
	stcopr	r0, SCTLR
	isb

	bx lr
endfunc plat_reset_handler

	/* ---------------------------------------------------------------------
	 * unsigned long plat_get_my_entrypoint(void);
	 *
	 * Main job of this routine is to distinguish between a cold and warm
	 * boot.
	 *
	 * TODO: warm boot (currently supports only cold boot)
	 * ---------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	mov	r0, #0
	bx	lr
endfunc plat_get_my_entrypoint

	/* --------------------------------------------------------------------
	 * void plat_secondary_cold_boot_setup(void);
	 *
	 * TODO
	 * --------------------------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	b	.
endfunc plat_secondary_cold_boot_setup

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary(void);
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	ldcopr	r0, MPIDR
	ldr	r1, =(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	and	r0, r1
	cmp	r0, #0
	moveq	r0, #1
	movne	r0, #0
	bx	lr
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 *  int platform_get_core_pos(int mpidr);
	 *
	 *  FIXME: deprecated. to be removed.
	 *
	 *  Return CorePos = (ClusterId * 4) + CoreId
	 * -----------------------------------------------------
	 */
func platform_get_core_pos
	and	r1, r0, #MPIDR_CLUSTER_MASK
	and	r0, r0, #MPIDR_CPU_MASK
	add	r0, r1, r0, LSR #8
	bx 	lr
endfunc platform_get_core_pos

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	ldcopr	r0, MPIDR
	b	platform_get_core_pos
endfunc plat_my_core_pos

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 *
	 * Initialize the crash console without a C Runtime stack.
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	bx	lr
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 *
	 * Print a character on the crash console without a C Runtime stack.
	 * Clobber list : r0 - r3
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	b	__asc_xmit_char
endfunc plat_crash_console_putc

