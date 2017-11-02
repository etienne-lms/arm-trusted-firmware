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
#include <desc_image_load.h>
#include <optee_utils.h>
#include <platform.h>
#include <platform_def.h>
#include <string.h>
#include <xlat_tables.h>

#include "stih410_private.h"

#if !LOAD_IMAGE_V2
#error Not LOAD_IMAGE_V2
#endif

/*
 * The next 2 constants identify the extents of the code & RO data region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __RO_START__ and __RO_END__ linker symbols refer to page-aligned addresses.
 */
#define BL2_RO_BASE (unsigned long)(&__RO_START__)
#define BL2_RO_LIMIT (unsigned long)(&__RO_END__)

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo_t bl2_tzram_layout __aligned(PLATFORM_CACHE_LINE_SIZE);

meminfo_t *bl2_plat_sec_mem_layout(void)
{
	return &bl2_tzram_layout;
}

/*******************************************************************************
 * BL1 has passed the extents of the trusted SRAM that should be visible to BL2
 * in x0. This memory layout is sitting at the base of the free trusted SRAM.
 * Copy it to a safe loaction before its reclaimed by later BL2 functionality.
 ******************************************************************************/
void bl2_early_platform_setup(meminfo_t *mem_layout)
{
	stih410_verbose_memory_layout();

	/* Setup the BL2 memory layout */
	bl2_tzram_layout.total_base = STIH410_SRAM_BASE;
	bl2_tzram_layout.total_size = STIH410_SRAM_SIZE;
}

/*******************************************************************************
 * Perform platform specific setup. For now just initialize the memory location
 * to use for passing arguments to BL31.
 ******************************************************************************/
void bl2_platform_setup(void)
{
	INFO("BL2 runs SP_MIN setup\n");
	stih410_io_setup();
}

/*******************************************************************************
 * This function flushes the data structures so that they are visible
 * in memory for the next BL image.
 ******************************************************************************/
void plat_flush_next_bl_params(void)
{
	bl_mem_params_node_t *bl_mem_params =
					get_bl_mem_params_node(BL32_IMAGE_ID);

	assert(bl_mem_params);

	plat_hpen_kick(bl_mem_params->ep_info.pc, STIH410_HPEN_NS_KICKER);

	flush_bl_params_desc();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl2_plat_arch_setup(void)
{
	/*
	 * Common configure_mmu defines the whole ERAM as read/write.
	 * Details here the read-only/exec parts.
	 */

	/* prevent BL2 read-only corruption */
	mmap_add_region(BL2_RO_BASE, BL2_RO_BASE,
			BL2_RO_LIMIT - BL2_RO_BASE,
			MT_MEMORY | MT_RO | MT_SECURE);

	/* prevent BL1 corruption: monitor is used to exit to BL32 */
	mmap_add_region(BL1_RO_BASE, BL1_RO_BASE,
			BL1_RO_LIMIT - BL1_RO_BASE,
			MT_MEMORY | MT_RO | MT_SECURE);
	mmap_add_region(BL1_RW_BASE, BL1_RW_BASE,
			BL1_RW_LIMIT - BL1_RW_BASE,
			MT_MEMORY | MT_RO | MT_SECURE);

	/* prevent corruption of preloaded BL32 */
	mmap_add_region(STIH410_BL32_SRAM_BASE, STIH410_BL32_SRAM_BASE,
			STIH410_BL32_SRAM_SIZE,
			MT_MEMORY | MT_RO | MT_SECURE);

	configure_mmu();
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

	assert(bl_mem_params);

	switch (image_id) {
	case BL32_IMAGE_ID:
		break;

	case BL33_IMAGE_ID:
		bl_mem_params->ep_info.spsr =
				SPSR_MODE32(MODE32_svc,
					plat_get_ns_image_entrypoint() & 0x1,
					SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS);
		break;

#ifdef SCP_BL2_BASE
	case SCP_BL2_IMAGE_ID:
		/* The subsequent handling of SCP_BL2 is platform specific */
		err = plat_arm_bl2_handle_scp_bl2(&bl_mem_params->image_info);
		if (err)
			WARN("Failure in platform handling of SCP_BL2 image.\n");
		break;
#endif
	}

	return err;
}

/*******************************************************************************
 * This function returns the list of loadable images.
 ******************************************************************************/
bl_load_info_t *plat_get_bl_image_load_info(void)
{
	return get_bl_load_info_from_mem_params_desc();
}

/*******************************************************************************
 * This function returns the list of executable images.
 ******************************************************************************/
bl_params_t *plat_get_next_bl_params(void)
{
	return get_next_bl_params_from_mem_params_desc();
}
