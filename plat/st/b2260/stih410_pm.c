/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <arch_helpers.h>
#include <arm_gic.h>
#include <debug.h>
#include <errno.h>
#include <platform_def.h>
#include <psci.h>

static void stih410_cpu_standby(plat_local_state_t cpu_state)
{
	/* TODO */
}

static int stih410_pwr_domain_on(u_register_t mpidr)
{
	/* TODO */
	return PSCI_E_SUCCESS;
}

static void stih410_pwr_domain_off(const psci_power_state_t *target_state)
{
	/* TODO */
}

static void stih410_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	/* TODO */
}

static void stih410_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* TODO */
}


static void stih410_pwr_domain_suspend_finish(const psci_power_state_t *state)
{
	/* TODO */
}

static void __dead2 stih410_system_off(void)
{
	ERROR("System Off: operation not handled.\n");
	panic();
}

static void __dead2 stih410_system_reset(void)
{
	ERROR("System Reset: operation not handled.\n");
	panic();
}

static int stih410_validate_power_state(unsigned int power_state,
				psci_power_state_t *req_state)
{
	return PSCI_E_SUCCESS;
}

static int stih410_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/* TODO: Check entrypoint value */
	return PSCI_E_SUCCESS;
}

static int stih410_node_hw_state(u_register_t target_cpu,
			     unsigned int power_level)
{
	/*
	 * The format of 'power_level' is implementation-defined, but 0 must
	 * mean a CPU. We also allow 1 to denote the cluster
	 */
	if (power_level != MPIDR_AFFLVL0 && power_level != MPIDR_AFFLVL1)
		return PSCI_E_INVALID_PARAMS;

	/* TODO: get the HW state of the cluster or cpu */
	return HW_ON;
}


/*******************************************************************************
 * Export the platform handlers via plat_arm_psci_pm_ops. The ARM Standard
 * platform layer will take care of registering the handlers with PSCI.
 ******************************************************************************/
const plat_psci_ops_t stih410_psci_pm_ops = {
	.cpu_standby = stih410_cpu_standby,
	.pwr_domain_on = stih410_pwr_domain_on,
	.pwr_domain_off = stih410_pwr_domain_off,
	.pwr_domain_suspend = stih410_pwr_domain_suspend,
	.pwr_domain_on_finish = stih410_pwr_domain_on_finish,
	.pwr_domain_suspend_finish = stih410_pwr_domain_suspend_finish,
	.system_off = stih410_system_off,
	.system_reset = stih410_system_reset,
	.validate_power_state = stih410_validate_power_state,
	.validate_ns_entrypoint = stih410_validate_ns_entrypoint,
	.get_node_hw_state = stih410_node_hw_state
};
/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
				const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &stih410_psci_pm_ops;

	return 0;
}
