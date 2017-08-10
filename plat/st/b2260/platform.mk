PLAT_PATH 		:=	plat/st/b2260

PLAT_INCLUDES		:=	-I${PLAT_PATH}/include/
PLAT_INCLUDES		+=	-Iinclude/common/tbbr

PLAT_BL_COMMON_SOURCES	:=	${PLAT_PATH}/stih410_common.c
PLAT_BL_COMMON_SOURCES	+=	${PLAT_PATH}/asc.S
PLAT_BL_COMMON_SOURCES	+=	${PLAT_PATH}/stih410_helper.S
PLAT_BL_COMMON_SOURCES	+=	lib/aarch32/arm32_aeabi_divmod.c
PLAT_BL_COMMON_SOURCES	+=	lib/aarch32/arm32_aeabi_divmod_a32.S
PLAT_BL_COMMON_SOURCES	+=	lib/cpus/aarch32/cortex_a9.S
PLAT_BL_COMMON_SOURCES	+=	lib/xlat_tables/${ARCH}/nonlpae_tables.c
PLAT_BL_COMMON_SOURCES	+=	common/desc_image_load.c

BL1_SOURCES 		:=	${PLAT_PATH}/bl1_plat_setup.c
BL1_SOURCES		+=	${PLAT_PATH}/stih410_io_storage.c
BL1_SOURCES		+=	drivers/io/io_storage.c
BL1_SOURCES		+=	drivers/io/io_dummy.c

BL2_SOURCES 		:=	${PLAT_PATH}/bl2_plat_setup.c
BL2_SOURCES		+=	${PLAT_PATH}/stih410_io_storage.c
BL2_SOURCES		+=	drivers/io/io_storage.c
BL2_SOURCES		+=	drivers/io/io_dummy.c
BL2_SOURCES		+=	${PLAT_PATH}/bl2_mem_params_desc.c
BL2_SOURCES		+=	${PLAT_PATH}/stih410_hpen.c

ifeq ($(AARCH32_SP),optee)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif

CTX_INCLUDE_FPREGS	:=	1
DEBUG			:=	0
ENABLE_PLAT_COMPAT	:=	0
SP_MIN_WITH_SECURE_FIQ	:=	1
PLAT_OUTER_CACHE	:=	1
LOAD_IMAGE_V2 		:= 	1

# Generic make expects the target core to be define when with ARMv7
ARM_CORTEX_A9		:=	yes
ARM_WITH_NEON		:=	yes
