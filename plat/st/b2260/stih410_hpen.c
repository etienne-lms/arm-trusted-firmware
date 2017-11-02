/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <platform.h>
#include <platform_def.h>

#include "stih410_private.h"

struct stih410_hpen_ctrl {
	uintptr_t jumper1;
	uintptr_t jumper2;
	uintptr_t kicker;
	uintptr_t kicker_2stage;
	uint32_t cur_method;
	uintptr_t cur_kicker;
	uint32_t notif_exit;
	uint32_t core1_left;
	uint32_t core2_left;
	uint32_t core3_left;
};

void plat_hpen_kick(uintptr_t ep, uintptr_t next_kicker)
{
	struct stih410_hpen_ctrl *hpen =
				(struct stih410_hpen_ctrl *)STIH410_HPEN_BASE;

	assert(hpen->cur_method == 1);

	/* insure secondary is not waiting in target kicker */
	if (hpen->cur_kicker == next_kicker) {
		ERROR("TODO: rekick secondary in a tempoary hpen\n");
		plat_error_handler(-1);		// FIXME: errno
	}

	hpen->core1_left = STIH410_HPEN_RSTVAL;
	hpen->notif_exit = ~STIH410_HPEN_RSTVAL;
	*(uint32_t *)next_kicker = STIH410_HPEN_RSTVAL;
	hpen->kicker = next_kicker;
	dmb();
	*(uint32_t *)hpen->cur_kicker = (uint32_t)ep;
	dsb();
	sev();
}

