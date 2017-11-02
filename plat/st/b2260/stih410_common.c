/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <arm_gic.h>
#include <arm/gicv2.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <mmio.h>
#include <platform_def.h>
#include <platform.h>
#include <xlat_tables.h>

#include "stih410_private.h"

/*
 * Default mapping for each BL: define internal boot RAM and external main RAM
 */
#define MAP_INTERNAL_MEMORY	MAP_REGION_FLAT(STIH410_SRAM_BASE, \
						STIH410_SRAM_SIZE, \
						MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_DDR_NS_RW		MAP_REGION_FLAT(STIH410_DDR_NS_BASE, \
						STIH410_DDR_NS_SIZE, \
						MT_MEMORY | MT_RW | MT_NS)

#define MAP_DDR_S_RW		MAP_REGION_FLAT(STIH410_DDR_S_BASE, \
						STIH410_DDR_S_SIZE, \
						MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_DEVICE0		MAP_REGION_FLAT(DEVICE0_BASE, \
						DEVICE0_SIZE, \
						MT_DEVICE | MT_RW | MT_SECURE)

/*
 * Table of regions for different BL stages to map using the MMU.
 */
#if defined(IMAGE_BL1)
static const mmap_region_t stih410_mmap[] = {
	MAP_INTERNAL_MEMORY,
	MAP_DEVICE0,
	{0}
};
#endif
#if defined(IMAGE_BL2)
static const mmap_region_t stih410_mmap[] = {
	MAP_INTERNAL_MEMORY,
	MAP_DDR_S_RW,
	MAP_DDR_NS_RW,
	MAP_DEVICE0,
	{0}
};
#endif
#if defined(IMAGE_BL32)
static const mmap_region_t stih410_mmap[] = {
	MAP_INTERNAL_MEMORY,
	MAP_DDR_S_RW,
	MAP_DEVICE0,
	{0}
};
#endif

void configure_mmu(void)
{
	mmap_add(stih410_mmap);
	init_xlat_tables();
	enable_mmu_secure(0);
}

void console_init(uintptr_t base_addr, unsigned int clk,
		  unsigned int baud_rate)
{
	/* nothing to do */
}

void console_uninit(void)
{
	/* nothing to do */
}

/* FIXME: from asc.S */
void __asc_xmit_char(char c);
void __asc_flush(void);

int console_putc(int c)
{
	if (c == '\n')
		__asc_xmit_char('\r');
	__asc_xmit_char(c);
	return 0;
}

void console_flush(void)
{
	__asc_flush();
}

uintptr_t plat_get_ns_image_entrypoint(void)
{
	return BL33_BASE;
}

uint64_t plat_get_syscnt_freq(void)
{
	// FIXME: dummy value
	return 0;
}

void plat_gic_init(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

void stih410_verbose_memory_layout(void)
{
	VERBOSE("SRAM:  [%x %x]\n", STIH410_SRAM_BASE,
				    STIH410_SRAM_BASE +
						STIH410_SRAM_SIZE);
#ifdef STIH410_XLAT_BASE
	VERBOSE("  xlat [%x %x]\n", STIH410_XLAT_BASE,
				    STIH410_XLAT_BASE +
						STIH410_BLX_XLAT_SIZE);
#endif
	VERBOSE("  BL32 [%x %x]\n", STIH410_BL32_SRAM_BASE,
				    STIH410_BL32_SRAM_BASE +
						STIH410_BL32_SRAM_SIZE);
	VERBOSE("   BL1 [%x %x %x]\n", STIH410_BL1_SRAM_BASE,
				       STIH410_BL1_SRAM_BASE +
						STIH410_BL1_SRAM_RO_SIZE,
				       STIH410_BL1_SRAM_BASE +
						STIH410_BL1_SRAM_SIZE);
	VERBOSE("   BL2 [%x %x %x]\n", STIH410_BL2_SRAM_BASE,
				       STIH410_BL2_SRAM_BASE +
						STIH410_BL2_SRAM_RO_SIZE,
				       STIH410_BL2_SRAM_BASE +
						STIH410_BL2_SRAM_SIZE);
	VERBOSE("DDR NS:  [%x %x]\n", STIH410_DDR_NS_BASE,
				      STIH410_DDR_NS_BASE +
						STIH410_DDR_NS_SIZE);
	VERBOSE("    BL33 [%x %x]\n", STIH410_BL33_ERAM_BASE,
				      STIH410_BL33_ERAM_BASE +
						STIH410_BL33_ERAM_SIZE);
	VERBOSE("DDR SEC: [%x %x]\n", STIH410_DDR_S_BASE,
				      STIH410_DDR_S_BASE +
						STIH410_DDR_S_SIZE);
#ifdef STIH410_BL32_ERAM_BASE
	VERBOSE("    BL32 [%x %x]\n", STIH410_BL32_ERAM_BASE,
				      STIH410_BL32_ERAM_BASE +
						STIH410_BL32_ERAM_SIZE);
#endif
}
