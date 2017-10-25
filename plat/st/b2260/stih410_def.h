/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __STIH410_DEF_H__
#define __STIH410_DEF_H__

/*
 * Default to b2260 settings
 *
 * #define B2260
 * #define B2120
 */
#define B2260

#ifndef SMALL_PAGE_SIZE
#define SMALL_PAGE_SHIFT	12
#define SMALL_PAGE_SIZE		(1 << SMALL_PAGE_SHIFT)
#define SMALL_PAGE_MASK		(SMALL_PAGE_SIZE - 1)
#endif

/*
 * Some of the definitions in this file use the 'ull' suffix in order to avoid
 * subtle integer overflow errors due to implicit integer type promotion when
 * working with 32-bit values.
 *
 * The TSP linker script includes some of these definitions to define the BL3-2
 * memory map, but the GNU LD does not support the 'ull' suffix, causing the
 * build process to fail. To solve this problem, the auxiliary macro MAKE_ULL(x)
 * will add the 'ull' suffix only when the macro __LINKER__  is not defined
 * (__LINKER__ is defined in the command line to preprocess the linker script).
 * Constants in the linker script will not have the 'ull' suffix, but this is
 * not a problem since the linker evaluates all constant expressions to 64 bit
 * (assuming the target architecture is 64 bit).
 */
#ifndef __LINKER__
  #define MAKE_ULL(x)			x##ull
#else
  #define MAKE_ULL(x)			x
#endif

#ifdef B2120
/* b2120 board main settings */
#define PLAT_DDR_BASE			0x40000000
#define PLAT_DDR_SIZE			0x80000000
#define PLAT_TZDDR_BASE			0xBE000000
#define PLAT_TZDDR_SIZE			0x01E00000
#define PLAT_TZRAM_BASE			0xBFE00000
#define PLAT_TZRAM_SIZE			0x00100000
#else
/* b2260 board main settings */
#define PLAT_DDR_BASE			0x40000000
#define PLAT_DDR_SIZE			0x40000000
#define PLAT_TZDDR_BASE			0x7E000000
#define PLAT_TZDDR_SIZE			0x01E00000
#define PLAT_TZRAM_BASE			0x7FE80000
#define PLAT_TZRAM_SIZE			0x00080000
#define PLAT_TZRAM2_BASE		0x7FE00000
#define PLAT_TZRAM2_SIZE		0x00080000
#endif

/*******************************************************************************
 * Memory map related constants
 *
 *  +-------------------------+ ?0000000     -----
 *  | rsv for OP-TEE (256kB)  |                   |
 *  +-------------------------+ ?FEC0000          |
 *  | BL32 read-only and rw   |                   | simulated
 *  |    ---------------------|                   | internal
 *  | BL1  read-only and rw   |                   | secure
 *  |    ---------------------|                   | RAM
 *  | BL2  read-only and rw   |                   | (512kB)
 *  |    ---------------------|                   |
 *  | free                    |                   |
 *  +-------------------------+ ?FE80000     -----
 *  | unused                  |
 *  +-------------------------+ ?E000000     -----
 *  | NonSecure DDR           | (2GB-32MB)        | non secure DDR
 *  +-------------------------+ 40000000     -----
 *
 ******************************************************************************/

/* Internal RAM simulated at 512kB upper bounds on 1MByte bondary (see above) */
#define STIH410_SRAM_BASE		PLAT_TZRAM_BASE
#define STIH410_SRAM_SIZE		(256 * 1024)

/* DDR configuration (all NonSecure DDR but the last 32MB) */
#define STIH410_DDR_NS_BASE		PLAT_DDR_BASE
#define STIH410_DDR_NS_LIMIT		PLAT_TZDDR_BASE
#define STIH410_DDR_NS_SIZE		(STIH410_DDR_NS_LIMIT - \
						STIH410_DDR_NS_BASE)
#define STIH410_DDR_S_BASE		PLAT_TZDDR_BASE
#define STIH410_DDR_S_SIZE		PLAT_TZDDR_SIZE

/* ATF BLs reserved sizes */
#ifndef AARCH32_SP_OPTEE
#define STIH410_BL32_SRAM_RO_SIZE	(24 * 1024)
#define STIH410_BL32_SRAM_RW_SIZE	(12 * 1024)
#define STIH410_BL32_SRAM_SIZE		(STIH410_BL32_SRAM_RO_SIZE + \
						STIH410_BL32_SRAM_RW_SIZE)
#endif

#define STIH410_BL2_SRAM_RO_SIZE	(16 * 1024)
#define STIH410_BL2_SRAM_RW_SIZE	(4 * 1024)
#define STIH410_BL2_SRAM_SIZE		(STIH410_BL2_SRAM_RO_SIZE + \
						STIH410_BL2_SRAM_RW_SIZE)

#define STIH410_BL1_SRAM_RO_SIZE	(16 * 1024)
#define STIH410_BL1_SRAM_RW_SIZE	(4 * 1024)
#define STIH410_BL1_SRAM_SIZE		(STIH410_BL1_SRAM_RO_SIZE + \
						STIH410_BL1_SRAM_RW_SIZE)

#define STIH410_BLX_XLAT_L1_SIZE	(16 * 1024)
/* round L2 tables to 4kB page alignment */
#define STIH410_BLX_XLAT_L2_SIZE	(((1024 * MAX_XLAT_TABLES) + \
					  SMALL_PAGE_SIZE - 1) & \
						~SMALL_PAGE_MASK)
#define STIH410_BLX_XLAT_SIZE		(STIH410_BLX_XLAT_L1_SIZE + \
						STIH410_BLX_XLAT_L2_SIZE)

#define STIH410_BLX_XLAT_L1_BASE	(STIH410_SRAM_BASE + \
						STIH410_SRAM_SIZE - \
						STIH410_BLX_XLAT_L1_SIZE)

#define STIH410_BLX_XLAT_L2_BASE	(STIH410_SRAM_BASE + \
						STIH410_SRAM_SIZE - \
						STIH410_BLX_XLAT_SIZE)

#define STIH410_XLAT_BASE		STIH410_BLX_XLAT_L2_BASE
#define STIH410_BL_TOP			STIH410_XLAT_BASE

#ifdef AARCH32_SP_OPTEE
#define STIH410_BL32_ERAM_BASE		PLAT_TZDDR_BASE
#define STIH410_BL32_ERAM_SIZE		(4 * 1024 * 1024)
#define STIH410_BL32_SRAM_BASE		PLAT_TZRAM2_BASE
#define STIH410_BL32_SRAM_SIZE		PLAT_TZRAM2_SIZE
#define STIH410_BL1_SRAM_BASE		(STIH410_BL_TOP - \
						STIH410_BL1_SRAM_SIZE)
#define STIH410_BL2_SRAM_BASE		(STIH410_BL1_SRAM_BASE - \
						STIH410_BL2_SRAM_SIZE)
#define STIH410_BINARY_BASE		STIH410_BL2_SRAM_BASE
#define STIH410_BINARY_SIZE		(STIH410_BL1_SRAM_SIZE + \
						STIH410_BL2_SRAM_SIZE)
#else /* assume BL32 is an A-TF image */
#define STIH410_BL32_SRAM_BASE		(STIH410_BL_TOP - \
						STIH410_BL32_SRAM_SIZE)
#define STIH410_BL1_SRAM_BASE		(STIH410_BL32_SRAM_BASE - \
						STIH410_BL1_SRAM_SIZE)
#define STIH410_BL2_SRAM_BASE		(STIH410_BL1_SRAM_BASE - \
						STIH410_BL2_SRAM_SIZE)

#define STIH410_BINARY_BASE		STIH410_BL2_SRAM_BASE
#define STIH410_BINARY_SIZE		(STIH410_BL1_SRAM_SIZE + \
						STIH410_BL2_SRAM_SIZE + \
						STIH410_BL32_SRAM_SIZE)
#endif /* AARCH32_SP_xxx */

/* NonSecure loaded: hard coded, stih410 does not use Fip */
#define STIH410_BL33_ERAM_SIZE		(10 * 1024 * 1024)
#define STIH410_BL33_ERAM_BASE		(PLAT_TZDDR_BASE - \
						STIH410_BL33_ERAM_SIZE)

/* Map all SoC IPs iomem in a single area */
#define DEVICE0_BASE			0x00000000
#define DEVICE0_SIZE			0x40000000

#ifdef B2120
#define STIH410_DEBUG_USART_BASE	0x09530000
#else
#define STIH410_DEBUG_USART_BASE	0x09831000
#endif

#define PLATFORM_GICD_BASE		0x08761000
#define PLATFORM_GICC_BASE		0x08760100

#define STIH410_PL310_BASE		0x08762000
#define STIH410_SCU_BASE		0x08760000

/* Holding pen */
#define STIH410_HPEN_BASE		0x06058000
#define STIH410_HPEN_RSTVAL		0xFFFFFFFF
#define STIH410_HPEN_NS_KICKER		0x094100A4

#endif /* __STIH410_DEF_H__ */
