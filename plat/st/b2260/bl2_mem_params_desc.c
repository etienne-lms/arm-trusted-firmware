/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <desc_image_load.h>
#include <platform_def.h>
#include <platform.h>

/*******************************************************************************
 * Following descriptor provides BL image/ep information that gets used
 * by BL2 to load the images and also subset of this information is
 * passed to next BL image. The image loading sequence is managed by
 * populating the images in required loading order. The image execution
 * sequence is managed by populating the `next_handoff_image_id` with
 * the next executable image id.
 ******************************************************************************/
static bl_mem_params_node_t bl2_mem_params_descs[] = {
#ifdef SCP_BL2_BASE
	/* Fill SCP_BL2 related information if it exists */
	{
		.image_id = SCP_BL2_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
					VERSION_2, entry_point_info_t,
					SECURE | NON_EXECUTABLE),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
					VERSION_2, image_info_t, 0),

		.image_info.image_base = SCP_BL2_BASE,
		.image_info.image_max_size = PLAT_CSS_MAX_SCP_BL2_SIZE,

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
#endif /* SCP_BL2_BASE */

	/* Fill BL32 related information */
	{
		.image_id = BL32_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
					VERSION_2, entry_point_info_t,
					SECURE | EXECUTABLE | EP_FIRST_EXE),

		.ep_info.spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM,
					SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
					VERSION_2, image_info_t,
					IMAGE_ATTRIB_PLAT_SETUP),

#if defined(AARCH32_SP_OPTEE)
		/* optee header is loaded is SYSRAM below BL2 */
		.image_info.image_base = STIH410_BL32_SRAM_BASE +
								(128 * 1024),
		.image_info.image_max_size = STIH410_BL32_SRAM_SIZE -
								(128 * 1024),
#else
		/* bl32 goes to the target location */
		.ep_info.pc = BL32_BASE,
		.image_info.image_base = BL32_BASE,
		.image_info.image_max_size = BL32_LIMIT - BL32_BASE,
#endif
		.next_handoff_image_id = BL33_IMAGE_ID,
	},

#if defined(AARCH32_SP_OPTEE)
	/* Fill BL32 external 1 image related information */
	{
		.image_id = BL32_EXTRA1_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				  VERSION_2, entry_point_info_t,
				  SECURE | NON_EXECUTABLE),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				  VERSION_2, image_info_t,
				  IMAGE_ATTRIB_SKIP_LOADING),

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
	/* Fill BL32 external 2 image related information */
	{
		.image_id = BL32_EXTRA2_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				  VERSION_2, entry_point_info_t,
				  SECURE | NON_EXECUTABLE),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				  VERSION_2, image_info_t,
				  IMAGE_ATTRIB_SKIP_LOADING),

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
#endif /* AARCH32_SP_OPTEE */

	/* Fill BL33 related information */
	{
		.image_id = BL33_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
					VERSION_2, entry_point_info_t,
					NON_SECURE | EXECUTABLE),
#ifdef PRELOADED_BL33_BASE
		.ep_info.pc = PRELOADED_BL33_BASE,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
					VERSION_2, image_info_t,
					IMAGE_ATTRIB_SKIP_LOADING),
#else
		.ep_info.pc = PLAT_STIH410_NS_IMAGE_OFFSET,
		.ep_info.spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM,
					SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
					VERSION_2, image_info_t, 0),

		.image_info.image_base = PLAT_STIH410_NS_IMAGE_OFFSET,
		.image_info.image_max_size = PLAT_STIH410_NS_IMAGE_MAXSIZE,
#endif /* PRELOADED_BL33_BASE */

		.next_handoff_image_id = INVALID_IMAGE_ID,
	}
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs)
