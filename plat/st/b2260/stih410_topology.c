/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <platform.h>
#include <psci.h>

const unsigned char stih410_power_domain_tree_desc[] = {
	PLATFORM_CLUSTER_COUNT,
	PLATFORM_CORE_COUNT,
};

/* This function returns the platform topology */
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return stih410_power_domain_tree_desc;
}

#if ENABLE_PLAT_COMPAT
unsigned int plat_get_aff_count(unsigned int aff_lvl, unsigned long mpidr)
{
	switch (aff_lvl) {
	case MPIDR_AFFLVL0:
		return PLATFORM_CORE_COUNT;
	case MPIDR_AFFLVL1:
		return PLATFORM_CLUSTER_COUNT;
	default:
		return PLATFORM_SYSTEM_COUNT;
	}
}

unsigned int plat_get_aff_state(unsigned int aff_lvl, unsigned long mpidr)
{
	return aff_lvl <= MPIDR_AFFLVL0 ? PSCI_AFF_PRESENT : PSCI_AFF_ABSENT;
}
#endif

int plat_setup_topology(void)
{
	return 0;
}

/*
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 */
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK))
		return -1;

	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;

	if (cluster_id >= PLATFORM_CLUSTER_COUNT)
		return -1;

	/*
	 * Validate cpu_id by checking whether it represents a CPU in
	 * one of the two clusters present on the platform.
	 */
	if (cpu_id >= stih410_power_domain_tree_desc[cluster_id])
		return -1;

	return (cluster_id * 4) + cpu_id;
}
