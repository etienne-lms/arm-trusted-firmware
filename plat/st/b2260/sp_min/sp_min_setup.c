/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <arm_gic.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <cortex_a9.h>
#include <debug.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <string.h>
#include <xlat_tables.h>

#include "../include/platform_def.h"
#include "../stih410_private.h"

#if RESET_TO_SP_MIN
#error b2260 does not support RESET_TO_SP_MIN
#endif

static entry_point_info_t bl33_image_ep_info;

/*
 * The next 3 constants identify the extents of the code, RO data region and the
 * limit of the BL3-1 image.  These addresses are used by the MMU setup code and
 * therefore they must be page-aligned.  It is the responsibility of the linker
 * script to ensure that __RO_START__, __RO_END__ & __BL31_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL32_RO_BASE (unsigned long)(&__RO_START__)
#define BL32_RO_LIMIT (unsigned long)(&__RO_END__)
#define BL32_END (unsigned long)(&__BL31_END__)

#if USE_COHERENT_MEM
/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL32_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL32_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)
#endif



/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for the
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *sp_min_plat_get_bl33_ep_info(void)
{
	entry_point_info_t *next_image_info;

	next_image_info = &bl33_image_ep_info;

	/*
	 * None of the images on the ARM development platforms can have 0x0
	 * as the entrypoint
	 */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}


#define SCU_CTRL		0x00
#define SCU_CONFIG		0x04
#define SCU_POWER		0x08
#define SCU_INV_SEC		0x0C
#define SCU_FILT_SA		0x40
#define SCU_FILT_EA		0x44
#define SCU_SAC			0x50
#define SCU_NSAC		0x54
#define SCU_ERRATA744369	0x30

static void configure_scu(void)
{
	mmio_write_32(STIH410_SCU_BASE + SCU_SAC, 0x3);
	mmio_write_32(STIH410_SCU_BASE + SCU_NSAC, 0x333);
	mmio_write_32(STIH410_SCU_BASE + SCU_FILT_EA, 0xc0000000);
	mmio_write_32(STIH410_SCU_BASE + SCU_FILT_SA, 0x40000000);
	mmio_write_32(STIH410_SCU_BASE + SCU_CTRL, 0x00000065);
}

void sp_min_early_platform_setup(void *from_bl2,
				 void *plat_params_from_bl2)
{
	stih410_verbose_memory_layout();

	write_sctlr(read_sctlr() | SCTLR_RR_BIT);

	write_actlr(0x00000041);
	write_nsacr(0x00020C00);
	write_pcr(read_pcr() | 1);

	configure_scu();

	/* Check params passed from BL2 should not be NULL */
	bl_params_t *params_from_bl2 = (bl_params_t *)from_bl2;

	assert(params_from_bl2 != NULL);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params) {
		if (bl_params->image_id == BL33_IMAGE_ID) {
			bl33_image_ep_info = *bl_params->ep_info;
			break;
		}

		bl_params = bl_params->next_params_info;
	}

	if (bl33_image_ep_info.pc == 0) {
		ERROR("bl33 entrypoint is not set\n");
		panic();
	}

	/* Imprecise aborts can be trapped by NonSecure */
	write_scr(read_scr() | SCR_AW_BIT);

	/* unmask secure interrupts when running in non secure state */
	bl33_image_ep_info.spsr &= SPSR_FIQ_BIT;
}

/*******************************************************************************
 * Perform platform specific setup for SP_MIN
 ******************************************************************************/
void sp_min_platform_setup(void)
{
	/* Initialize the gic cpu and distributor interfaces */
	platform_gic_driver_init();
	platform_gic_init();

	stih410_setup_pl310();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this only initializes the MMU
 ******************************************************************************/
void sp_min_plat_arch_setup(void)
{
	/*
	 * Common configure_mmu defines the whole ERAM as read/write.
	 * Details here the read-only/exec parts: BL31 (monitor).
	 */
	mmap_add_region(BL32_RO_BASE, BL32_RO_BASE,
			BL32_RO_LIMIT - BL32_RO_BASE,
			MT_MEMORY | MT_RO | MT_SECURE);

	configure_mmu();
}


// FIXME: cleanup this
#define SYS_COUNTER_FREQ_IN_TICKS	((1000 * 1000 * 1000) / 16)
unsigned int plat_get_syscnt_freq2(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}


