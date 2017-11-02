/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __STIH410_PRIVATE_H__
#define __STIH410_PRIVATE_H__

#include <bl_common.h>

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void stih410_io_setup(void);
void configure_mmu(void);

/* Declarations for security.c */
void plat_security_setup(void);

/* gic plat init*/
void plat_gic_init(void);

void plat_hpen_kick(uintptr_t ep, uintptr_t next_kicker);

/*******************************************************************************
 * This structure represents the superset of information that is passed to
 * BL31 e.g. while passing control to it from BL2 which is bl31_params
 * and bl31_plat_params and its elements
 ******************************************************************************/
typedef struct bl2_to_bl31_params_mem {
	bl_params_t bl31_params;
	image_info_t bl31_image_info;
	image_info_t bl32_image_info;
	image_info_t bl33_image_info;
	entry_point_info_t bl33_ep_info;
	entry_point_info_t bl32_ep_info;
	entry_point_info_t bl31_ep_info;
} bl2_to_bl31_params_mem_t;

void platform_gic_driver_init(void);
void platform_gic_init(void);
void platform_gic_cpuif_enable(void);
void platform_gic_cpuif_disable(void);
void platform_gic_pcpu_init(void);

void stih410_setup_pl310(void);

void stih410_verbose_memory_layout(void);

void stih410_postload_set_image_location(unsigned int image_id,
					 size_t offset, size_t length);

#endif /* __STIH410_PRIVATE_H__ */
