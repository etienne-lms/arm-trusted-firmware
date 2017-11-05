# OP-TEE is an secure payload built externally to A-TF.
#
# This makefile only aims at complying with A-TF build process that "optee"
# is a valid A-TF AArch32 Secure Playload identifier.

ifneq ($(ARCH),aarch32)
$(error This directory targets AArch32 support)
endif

$(eval $(call add_define,AARCH32_SP_OPTEE))

$(info A-TF built for OP-TEE payload support)
