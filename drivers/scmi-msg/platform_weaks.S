/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2019, Linaro Limited
 */
#include <asm_macros.S>

#include <drivers/scmi.h>

	/*
	 * Weak functions for SCMI message driver API the platform does
	 * not implement.
	 */

	.weak plat_scmi_clock_count
	.weak plat_scmi_clock_get_name
	.weak plat_scmi_clock_rates_array
	.weak plat_scmi_clock_rates_by_step
	.weak plat_scmi_clock_get_current_rate
	.weak plat_scmi_clock_set_current_rate
	.weak plat_scmi_clock_get_state
	.weak plat_scmi_clock_set_state

	.weak plat_scmi_rd_count
	.weak plat_scmi_rd_get_name
	.weak plat_scmi_rd_autonomous
	.weak plat_scmi_rd_set_state

	.globl plat_scmi_clock_count
	.globl plat_scmi_clock_get_name
	.globl plat_scmi_clock_rates_array
	.globl plat_scmi_clock_rates_by_step
	.globl plat_scmi_clock_get_current_rate
	.globl plat_scmi_clock_set_current_rate
	.globl plat_scmi_clock_get_state
	.globl plat_scmi_clock_set_state

	.globl plat_scmi_rd_count
	.globl plat_scmi_rd_get_name
	.globl plat_scmi_rd_autonomous
	.globl plat_scmi_rd_set_state

/* Unique weak function for unsupported handlers returning SCMI_NOT_SUPPORTED */
func scmi_weak_not_supported

plat_scmi_clock_set_current_rate:
plat_scmi_clock_get_state:
plat_scmi_clock_set_state:
plat_scmi_clock_rates_array:
plat_scmi_clock_rates_by_step:
plat_scmi_rd_autonomous:
plat_scmi_rd_set_state:

	ldr	r0, =SCMI_NOT_SUPPORTED
	bx	lr
endfunc scmi_weak_not_supported

/* Unique weak function for unsupported platform handlers returning 0 */
func scmi_msg_weak_null

plat_scmi_clock_count:
plat_scmi_clock_get_name:
plat_scmi_clock_get_current_rate:
plat_scmi_rd_count:
plat_scmi_rd_get_name:

	ldr	r0, =0
	bx	lr
endfunc scmi_msg_weak_null

/* Unique weak function for unsupported platform handlers returning void */
func scmi_msg_weak_void

	bx	lr
endfunc scmi_msg_weak_void
