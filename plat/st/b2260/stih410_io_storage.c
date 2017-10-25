/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
#include <io_driver.h>
#include <io_dummy.h>
#include <io_memmap.h>
#include <io_storage.h>
#include <platform_def.h>
#include <string.h>
#include <tbbr/tbbr_img_def.h>
#include <utils.h>

/* IO devices: all dummy (image are already loaded by earlier boot stage */

static const io_dev_connector_t *dummy_dev_con;
static uintptr_t dummy_dev_handle;
static uintptr_t dummy_dev_spec;

static io_block_spec_t bl2_block_spec = {
	.offset = STIH410_BL2_SRAM_BASE,
	.length = STIH410_BL2_SRAM_SIZE,
};

#if !defined(AARCH32_SP_OPTEE)
static io_block_spec_t bl32_block_spec = {
	.offset = STIH410_BL32_SRAM_BASE,
	.length = STIH410_BL32_SRAM_SIZE,
};
#endif

#if defined(AARCH32_SP_OPTEE)
/* bl32 header get preloaded in optee sram at offset 128kB */
static io_block_spec_t bl32_block_spec = {
	.offset = PLAT_TZRAM2_BASE + 0x00020000,
	.length = 0x1000,
};
/* io_dummy landing address are set at runtime */
static io_block_spec_t bl32_extra1_block_spec;
static io_block_spec_t bl32_extra2_block_spec;
#endif /* AARCH32_SP_OPTEE */

static io_block_spec_t bl33_block_spec = {
	.offset = STIH410_BL33_ERAM_BASE,
	.length = STIH410_BL33_ERAM_SIZE,
};

/*internal function declaration*/
static int open_dummy(const uintptr_t spec);

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

static const struct plat_io_policy policies[] = {
	[BL2_IMAGE_ID] = {
		&dummy_dev_handle,
		(uintptr_t)&bl2_block_spec,
		open_dummy
	},
	[BL32_IMAGE_ID] = {
		&dummy_dev_handle,
		(uintptr_t)&bl32_block_spec,
		open_dummy
	},
#if defined(AARCH32_SP_OPTEE)
	[BL32_EXTRA1_IMAGE_ID] = {
		&dummy_dev_handle,
		(uintptr_t)&bl32_extra1_block_spec,
		open_dummy
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		&dummy_dev_handle,
		(uintptr_t)&bl32_extra2_block_spec,
		open_dummy
	},
#endif
	[BL33_IMAGE_ID] = {
		&dummy_dev_handle,
		(uintptr_t)&bl33_block_spec,
		open_dummy
	},
};

static int open_dummy(const uintptr_t spec)
{
	int rc;

	rc = io_dev_init(dummy_dev_handle, (uintptr_t)NULL);
	if (!rc)
		INFO("Using Dummy\n");
	return rc;
}

void stih410_io_setup(void)
{
	int rc;

	/* Register the IO devices on this platform */
	rc = register_io_dev_dummy(&dummy_dev_con);
	assert(!rc);

	/* Open connections to devices and cache the handles */
	rc = io_dev_open(dummy_dev_con, dummy_dev_spec, &dummy_dev_handle);
	assert(!rc);

	(void)rc;
}


/*
 * Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy.
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int rc;
	const struct plat_io_policy *policy;

	assert(image_id < ARRAY_SIZE(policies));

	policy = &policies[image_id];
	rc = policy->check(policy->image_spec);
	if (rc == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	}

	return rc;
}

void stih410_postload_set_image_location(unsigned int image_id,
					 size_t offset, size_t length)
{
	io_block_spec_t *io_blk;

	assert(image_id < ARRAY_SIZE(policies));

	io_blk = (io_block_spec_t *)policies[image_id].image_spec;
	io_blk->offset = offset;
	io_blk->length = length;
}
