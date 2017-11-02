/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <mmio.h>
#include <pl310.h>
#include <platform_def.h>

/*
 * PL310 TAG RAM Control Register
 *
 * bit[10:8]:1 - 2 cycle of write accesses latency
 * bit[6:4]:1 - 2 cycle of read accesses latency
 * bit[2:0]:1 - 2 cycle of setup latency
 */
#define PL310_TAG_RAM_CTRL_INIT		0x00000111

/*
 * PL310 DATA RAM Control Register
 *
 * bit[10:8]:2 - 3 cycle of write accesses latency
 * bit[6:4]:2 - 3 cycle of read accesses latency
 * bit[2:0]:2 - 3 cycle of setup latency
 */
#define PL310_DATA_RAM_CTRL_INIT	0x00000222

/*
 * PL310 Auxiliary Control Register
 *
 * I/Dcache prefetch enabled (bit29:28=2b11)
 * NS can access interrupts (bit27=1)
 * NS can lockown cache lines (bit26=1)
 * Pseudo-random replacement policy (bit25=0)
 * Force write allocated (default)
 * Shared attribute internally ignored (bit22=1, bit13=0)
 * Parity disabled (bit21=0)
 * Event monitor disabled (bit20=0)
 * Platform fmavor specific way config:
 * - way size (bit19:17)
 * - way associciativity (bit16)
 * Store buffer device limitation enabled (bit11=1)
 * Cacheable accesses have high prio (bit10=0)
 * Full Line Zero (FLZ) disabled (bit0=0)
 */
#define PL310_AUX_CTRL_INIT		0x3C480800

/*
 * PL310 Prefetch Control Register
 *
 * Double linefill disabled (bit30=1)
 * I/D prefetch enabled (bit29:28=2b11)
 * Double linefill on WRAP read disable (bit27=1)
 * Prefetch drop enabled (bit24=1)
 * Incr double linefill disable (bit23=1)
 * Prefetch offset = 8 (bits4-0=7)
 */
#define PL310_PREFETCH_CTRL_INIT	0x79800007

/*
 * PL310 Power Register
 *
 * Dynamic clock gating enabled
 * Standby mode enabled
 */
#define PL310_POWER_CTRL_INIT		0x00000003

static uintptr_t stih410_pl310_base(void)
{
	/* TODO: current implementation assumes MMU off or linear mapping */
	return STIH410_PL310_BASE;
}

void stih410_setup_pl310(void)
{
	uintptr_t pl310_base = stih410_pl310_base();

	VERBOSE("enable and lock PL310\n");

	pl310_disable(pl310_base);

	/* config PL310 */
	mmio_write_32(pl310_base + PL310_TAG_RAM_CTRL,
			PL310_TAG_RAM_CTRL_INIT);
	mmio_write_32(pl310_base + PL310_DATA_RAM_CTRL,
			PL310_DATA_RAM_CTRL_INIT);
	mmio_write_32(pl310_base + PL310_AUX_CTRL,
			PL310_AUX_CTRL_INIT);
	mmio_write_32(pl310_base + PL310_PREFETCH_CTRL,
			PL310_PREFETCH_CTRL_INIT);
	mmio_write_32(pl310_base + PL310_POWER_CTRL,
			PL310_POWER_CTRL_INIT);

	pl310_invalidate_by_way(pl310_base);
	pl310_enable(pl310_base);
	pl310_lock_all_ways(pl310_base);
	pl310_clean_invalidate_by_way(pl310_base);
}

void plat_flush_outer_cache(void)
{
	pl310_clean_invalidate_by_way(stih410_pl310_base());
}
