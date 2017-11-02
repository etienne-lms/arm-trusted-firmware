/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <console.h>
#include <debug.h>
#include <platform_def.h>
#include <runtime_svc.h>

/*
 * The code below shows how one can register handlers for some SMCs.
 * As BL32 can live together with an OP-TEE, trap those OP-TEE SMCs.
 */
static int32_t pltf_fast_smc_setup(void)
{
	INFO("SP_MIN SMC: fast_smc setup\n");
	return 0;
}

uintptr_t pltf_smc_handler(
			uint32_t smc_fid,
			u_register_t x1, u_register_t x2,
			u_register_t x3, u_register_t x4,
			void *cookie, void *handle,
			u_register_t flags)
{
	INFO("SP_MIN SMC: NS %d, ID %x, Args %x %x %x %x\n",
			is_caller_non_secure(flags) ? 1 : 0,
			smc_fid, x1, x2, x3, x4);
	INFO("SP_MIN SMC: cookie %p handle %p, flags %x\n",
			cookie, handle, flags);

	SMC_RET1(handle, SMC_UNK);
}

DECLARE_RT_SVC(
	plft_smc_fast,
	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_FAST,
	pltf_fast_smc_setup,
	pltf_smc_handler
);

DECLARE_RT_SVC(
	plft_smc_std,
	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_YIELD,
	NULL,
	pltf_smc_handler
);
