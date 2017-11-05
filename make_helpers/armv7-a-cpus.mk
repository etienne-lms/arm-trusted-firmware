#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# For ARMv7, set march32 from platform directive ARMV7_CORTEX_Ax=yes
# and ARM_WITH_NEON=yes/no.
#
# GCC and Clang require -march=armv7-a for C-A9 and -march=armv7ve for C-A15.
# armClang requires -march=armv7-a for all ARMv7 Cortex-A. To comply with
# all, just drop -march and supply only -mcpu.

# Platform can override march32-directive through MARCH32_DIRECTIVE
ifdef MARCH32_DIRECTIVE
march32-directive		:= $(MARCH32_DIRECTIVE)
else
march32-set-${ARM_CORTEX_A5}	:= -mcpu=cortex-a5
march32-set-${ARM_CORTEX_A7}	:= -mcpu=cortex-a7
march32-set-${ARM_CORTEX_A9}	:= -mcpu=cortex-a9
march32-set-${ARM_CORTEX_A12}	:= -mcpu=cortex-a12
march32-set-${ARM_CORTEX_A15}	:= -mcpu=cortex-a15
march32-set-${ARM_CORTEX_A17}	:= -mcpu=cortex-a17
march32-neon-$(ARM_WITH_NEON)	:= -mfpu=neon

# default to -march=armv7-a as target directive
march32-set-yes			?= -march=armv7-a
march32-directive		:= ${march32-set-yes} ${march32-neon-yes}
endif

# Platform may override these extension support directives:
#
# ARCH_SUPPORTS_LARGE_PAGE_ADDRESSING = 0/1
# Defines if core supports the Large Page Addressing extension.
#
# ARCH_SUPPORTS_VIRTUALIZATION = 0/1
# Defines if ARMv7 core supports the Virtualization extension.

define pre-a15-extensions
ARCH_SUPPORTS_LARGE_PAGE_ADDRESSING	?= 	0
ARCH_SUPPORTS_VIRTUALIZATION		?= 	0
endef
define post-a15-extensions
ARCH_SUPPORTS_LARGE_PAGE_ADDRESSING	?= 	1
ARCH_SUPPORTS_VIRTUALIZATION		?= 	1
endef

ifeq ($(ARM_CORTEX_A5),yes)
$(call $(call pre-a15-extensions))
endif

ifeq ($(ARM_CORTEX_A7),yes)
$(eval $(call post-a15-extensions))
endif

ifeq ($(ARM_CORTEX_A9),yes)
$(eval $(call pre-a15-extensions))
endif

ifeq ($(ARM_CORTEX_A12),yes)
$(eval $(call post-a15-extensions))
endif

ifeq ($(ARM_CORTEX_A15),yes)
$(eval $(call post-a15-extensions))
endif

ifeq ($(ARM_CORTEX_A17),yes)
$(eval $(call post-a15-extensions))
endif

$(eval $(call assert_boolean,ARCH_SUPPORTS_LARGE_PAGE_ADDRESSING))
$(eval $(call add_define,ARCH_SUPPORTS_LARGE_PAGE_ADDRESSING))

$(eval $(call assert_boolean,ARCH_SUPPORTS_VIRTUALIZATION))
$(eval $(call add_define,ARCH_SUPPORTS_VIRTUALIZATION))
