// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2018-2019, STMicroelectronics
 */

#include <assert.h>
#include <stdbool.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32mp1_pwr.h>
#include <lib/mmio.h>

#define PWR_CR3_USB33_EN	BIT(24)
#define PWR_CR3_USB33_RDY	BIT(26)
#define PWR_CR3_REG18_EN	BIT(28)
#define PWR_CR3_REG18_RDY	BIT(29)
#define PWR_CR3_REG11_EN	BIT(30)
#define PWR_CR3_REG11_RDY	BIT(31)

struct pwr_regu_desc {
	unsigned long level_mv;
	uint32_t cr3_enable_mask;
	uint32_t cr3_ready_mask;
};

static const struct pwr_regu_desc pwr_regulators[PWR_REGU_COUNT] = {
	 [PWR_REG11] = {
		 .level_mv = 1100,
		 .cr3_enable_mask = PWR_CR3_REG11_EN,
		 .cr3_ready_mask = PWR_CR3_REG11_RDY,
	 },
	 [PWR_REG18] = {
		 .level_mv = 1800,
		 .cr3_enable_mask = PWR_CR3_REG18_EN,
		 .cr3_ready_mask = PWR_CR3_REG18_RDY,
	 },
	 [PWR_USB33] = {
		 .level_mv = 3300,
		 .cr3_enable_mask = PWR_CR3_USB33_EN,
		 .cr3_ready_mask = PWR_CR3_USB33_RDY,
	 },
};

unsigned long stm32mp1_pwr_regulator_mv(enum pwr_regulator id)
{
	assert(id < PWR_REGU_COUNT);

	return pwr_regulators[id].level_mv;
}

void stm32mp1_pwr_regulator_set_state(enum pwr_regulator id, bool enable)
{
	uintptr_t cr3 = stm32mp_pwr_base() + PWR_CR3;
	uint32_t enable_mask = pwr_regulators[id].cr3_enable_mask;

	assert(id < PWR_REGU_COUNT);

	if (enable != 0) {
		uint64_t to = timeout_init_us(10 * 1000);
		uint32_t ready_mask = pwr_regulators[id].cr3_ready_mask;

		mmio_setbits_32(cr3, enable_mask);

		while (!timeout_elapsed(to)) {
			if ((mmio_read_32(cr3) & ready_mask) != 0) {
				break;
			}
		}

		if ((mmio_read_32(cr3) & ready_mask) == 0U) {
			panic();
		}
	} else {
		mmio_clrbits_32(cr3, enable_mask);
	}
}

bool stm32mp1_pwr_regulator_is_enabled(enum pwr_regulator id)
{
	assert(id < PWR_REGU_COUNT);

	return mmio_read_32(stm32mp_pwr_base() + PWR_CR3) &
	       pwr_regulators[id].cr3_enable_mask;
}
