/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arch.h>
#include <common_def.h>
#include <tbbr/tbbr_img_def.h>	/* xxx_IMAGE_ID */

#include "../stih410_def.h"

#define PLAT_MAX_PWR_LVL	1

/******************************************************************************
 * Platform binary types for linking
 *****************************************************************************/
#define PLATFORM_LINKER_FORMAT          "elf32-littlearm"
#define PLATFORM_LINKER_ARCH            arm

/******************************************************************************
 * Generic platform constants
 *****************************************************************************/

/* Size of cacheable stacks */
#define PLATFORM_STACK_SIZE		0x600

#define PLATFORM_CACHE_LINE_SIZE	32
#define PLATFORM_SYSTEM_COUNT		1
#define PLATFORM_CLUSTER_COUNT		1
#define PLATFORM_CORE_COUNT		2

#define PLATFORM_NUM_AFFS		(PLATFORM_CLUSTER_COUNT + \
						PLATFORM_CORE_COUNT)
#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4

/* Memory layout related to the BLs */
#define BL1_RO_BASE			STIH410_BL1_SRAM_BASE
#define BL1_RO_LIMIT			(STIH410_BL1_SRAM_BASE + \
						STIH410_BL1_SRAM_RO_SIZE)
#define BL1_RW_BASE			(STIH410_BL1_SRAM_BASE + \
						STIH410_BL1_SRAM_RO_SIZE)
#define BL1_RW_LIMIT			(STIH410_BL1_SRAM_BASE + \
						STIH410_BL1_SRAM_SIZE)

#define BL2_BASE			STIH410_BL2_SRAM_BASE
#define BL2_LIMIT			(STIH410_BL2_SRAM_BASE + \
						STIH410_BL2_SRAM_SIZE)
#define BL2_RW_BASE			(STIH410_BL2_SRAM_BASE + \
						STIH410_BL2_SRAM_RO_SIZE)

#ifdef AARCH32_SP_OPTEE
#define BL32_BASE			STIH410_BL32_ERAM_BASE
#define BL32_LIMIT			(STIH410_BL32_ERAM_BASE + \
						STIH410_BL32_ERAM_SIZE)
#else
#define BL32_BASE			STIH410_BL32_SRAM_BASE
#define BL32_LIMIT			(STIH410_BL32_SRAM_BASE + \
						STIH410_BL32_SRAM_SIZE)
#endif

#define BL33_BASE			STIH410_BL33_ERAM_BASE
#define BL33_LIMIT			(STIH410_DDR_NS_BASE + \
						STIH410_DDR_NS_SIZE)

#define PLAT_STIH410_NS_IMAGE_OFFSET	BL33_BASE
#define PLAT_STIH410_NS_IMAGE_MAXSIZE	STIH410_BL33_ERAM_SIZE

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 32)

#if defined(IMAGE_BL1)
#define MAX_XLAT_TABLES			3
#elif defined(IMAGE_BL2)
#define MAX_XLAT_TABLES			4
#elif defined(IMAGE_BL32)
#define MAX_XLAT_TABLES			4
#else
#define MAX_XLAT_TABLES			3
#endif
#define MAX_MMAP_REGIONS		9

#define PLAT_BASE_XLAT_BASE		STIH410_BLX_XLAT_L1_BASE
#define PLAT_BASE_XLAT_SIZE		STIH410_BLX_XLAT_L1_SIZE
#define PLAT_XLAT_BASE			STIH410_BLX_XLAT_L2_BASE
#define PLAT_XLAT_SIZE			STIH410_BLX_XLAT_L2_SIZE

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT		5
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

/*
 * Secure Interrupt: based on the standard ARM mapping
 */
#define ARM_IRQ_SEC_PHY_TIMER		29

#define ARM_IRQ_SEC_SGI_0		8
#define ARM_IRQ_SEC_SGI_1		9
#define ARM_IRQ_SEC_SGI_2		10
#define ARM_IRQ_SEC_SGI_3		11
#define ARM_IRQ_SEC_SGI_4		12
#define ARM_IRQ_SEC_SGI_5		13
#define ARM_IRQ_SEC_SGI_6		14
#define ARM_IRQ_SEC_SGI_7		15

/*
 * Define a list of Group 1 Secure and Group 0 interrupt properties as per GICv2
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLATFORM_G1S_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY,	\
					      grp, GIC_INTR_CFG_LEVEL),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY,	\
					  grp, GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY,	\
					  grp, GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY,	\
					  grp, GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY,	\
					  grp, GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY,	\
					  grp, GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY,	\
					  grp, GIC_INTR_CFG_EDGE)

#define PLATFORM_G0_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY,	\
					  grp, GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,	\
					  grp, GIC_INTR_CFG_EDGE)

/*
 * Power
 */
#define ARM_LOCAL_STATE_RUN	0
#define ARM_LOCAL_STATE_RET	1
#define ARM_LOCAL_STATE_OFF	2

/*
 * This macro defines the deepest retention state possible. A higher state
 * id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE		ARM_LOCAL_STATE_RET

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE		ARM_LOCAL_STATE_OFF

#if !USE_COHERENT_MEM
/*******************************************************************************
 * Size of the per-cpu data in bytes that should be reserved in the generic
 * per-cpu data structure.   euh.... this might not be really useful.
 ******************************************************************************/
#define PLAT_PCPU_DATA_SIZE	2
#endif

#endif /* __PLATFORM_DEF_H__ */
