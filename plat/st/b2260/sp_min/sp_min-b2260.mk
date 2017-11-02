BL32_SOURCES		+=	${PLAT_PATH}/sp_min/sp_min_setup.c
BL32_SOURCES		+=	${PLAT_PATH}/sp_min/sp_min_optee_svc.c
BL32_SOURCES		+=	${PLAT_PATH}/stih410_topology.c
BL32_SOURCES		+=	${PLAT_PATH}/stih410_pm.c
BL32_SOURCES		+=	${PLAT_PATH}/stih410_gic.c
BL32_SOURCES		+=	${PLAT_PATH}/stih410_pl310.c

BL32_SOURCES		+=	plat/common/aarch32/platform_mp_stack.S
BL32_SOURCES		+=	plat/common/aarch32/plat_common.c
BL32_SOURCES		+=	plat/common/plat_psci_common.c
BL32_SOURCES		+=	plat/common/plat_gicv2.c
BL32_SOURCES		+=	drivers/arm/gic/common/gic_common.c
BL32_SOURCES		+=	drivers/arm/gic/v2/gicv2_main.c
BL32_SOURCES		+=	drivers/arm/gic/v2/gicv2_helpers.c
BL32_SOURCES		+=	drivers/arm/pl310/pl310.c
BL32_SOURCES		+=	drivers/arm/pl310/aarch32/pl310_asm.S

# Pl310 and Cortex support ACTLR[FLZW]: we need to provide the field position.
ARMV7_CORTEX_A_ACTLR_FLZW_BIT	:=	0x8
$(eval $(call add_define,ARMV7_CORTEX_A_ACTLR_FLZW_BIT))
