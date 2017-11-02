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
#include <platform_def.h>
#include <platform.h>
#include <string.h>
#include <xlat_tables.h>

#include "stih410_private.h"

static meminfo_t bl1_xram_layout;

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &bl1_xram_layout;
}

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	stih410_verbose_memory_layout();

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_xram_layout.total_base = STIH410_SRAM_BASE;
	bl1_xram_layout.total_size = STIH410_SRAM_SIZE;
}

/*******************************************************************************
 * Function which will evaluate how much of the trusted ram has been gobbled
 * up by BL1 and return the base and size of whats available for loading BL2.
 * Its called after coherency and the MMU have been turned on.
 ******************************************************************************/
void bl1_platform_setup(void)
{
	stih410_io_setup();
}

/*******************************************************************************
 * Perform the very early platform specific architecture setup here. At the
 * moment this only does basic initialization. Later architectural setup
 * (bl1_arch_setup()) does not do anything platform specific.
 ******************************************************************************/
void bl1_plat_arch_setup(void)
{
	/*
	 * Common configure_mmu defines the whole ERAM as read/write.
	 * Details here the read-only/exec parts.
	 */

	/* prevent corruption of BL1 read-only */
	mmap_add_region(BL1_RO_BASE, BL1_RO_BASE,
			BL1_RO_LIMIT - BL1_RO_BASE,
			MT_MEMORY | MT_RO | MT_SECURE);

	/* prevent corruption of preloaded BL2 */
	mmap_add_region(STIH410_BL2_SRAM_BASE, STIH410_BL2_SRAM_BASE,
			STIH410_BL2_SRAM_SIZE,
			MT_MEMORY | MT_RO | MT_SECURE);
	/* prevent corruption of preloaded BL32 */
	mmap_add_region(STIH410_BL32_SRAM_BASE, STIH410_BL32_SRAM_BASE,
			STIH410_BL32_SRAM_SIZE,
			MT_DEVICE | MT_RO | MT_SECURE);

	configure_mmu();
}

/*******************************************************************************
 * Before calling this function BL2 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL2 and set SPSR and security state.
 * On Juno we are only setting the security state, entrypoint
 ******************************************************************************/
void bl1_plat_set_bl2_ep_info(image_info_t *bl2_image,
			      entry_point_info_t *bl2_ep)
{
	SET_SECURITY_STATE(bl2_ep->h.attr, SECURE);
	bl2_ep->spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM, SPSR_E_LITTLE,
				   DISABLE_ALL_EXCEPTIONS);
}

