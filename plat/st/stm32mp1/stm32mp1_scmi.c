// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2019, STMicroelectronics
 */

#include <assert.h>
#include <stdint.h>

#include <platform_def.h>
#include <common/speculation_barrier.h>

#include <drivers/st/stm32mp1_clk.h>
#include <drivers/st/stm32mp_reset.h>
#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <dt-bindings/reset/stm32mp1-resets.h>

#include <stm32mp_shared_resources.h>
#include <stm32mp_common.h>

#define TIMEOUT_US_1MS		1000

/* Locate all non-secure SMT message buffers in last page of SYSRAM */
#define SMT_BUFFER_BASE		STM32MP_NS_SYSRAM_BASE

static struct scmi_msg_channel scmi_channel = {
	.shm_addr = SMT_BUFFER_BASE,
	.shm_size = SMT_BUF_SLOT_SIZE,
};

struct scmi_msg_channel *plat_scmi_get_channel(unsigned int agent_id)
{
	if (agent_id == 0)
		return &scmi_channel;
	else
		return NULL;
}

#define CLOCK_CELL(_scmi_id, _id, _name, _init_enabled) \
	[_scmi_id] = { \
		.clock_id = _id, \
		.name = _name, \
		.enabled = _init_enabled, \
	}

struct stm32_scmi_clk {
	unsigned long clock_id;
	const char *name;
	bool enabled;
};

struct stm32_scmi_clk stm32_scmi_clock[] = {
	CLOCK_CELL(0, CK_HSE, "clk-hse", true),
	CLOCK_CELL(1, CK_HSI, "clk-hsi", true),
	CLOCK_CELL(2, CK_CSI, "clk-csi", true),
	CLOCK_CELL(3, CK_LSE, "clk-lse", true),
	CLOCK_CELL(4, CK_LSI, "clk-lsi", true),
};

#define RESET_CELL(_scmi_id, _id, _name) \
	[_scmi_id] = { \
		.reset_id = _id, \
		.name = _name, \
	}

struct stm32_scmi_rd {
	unsigned long reset_id;
	const char *name;
};

struct stm32_scmi_rd stm32_scmi_reset_domain[] = {
	RESET_CELL(0, SPI6_R, "spi6"),
	RESET_CELL(1, I2C4_R, "i2c4"),
	RESET_CELL(2, I2C6_R, "i2c6"),
	RESET_CELL(3, USART1_R, "usart1"),
};

static const char vendor[] = "ST";
static const char sub_vendor[] = "";

const char *plat_scmi_vendor_name(void)
{
	return vendor;
}

const char *plat_scmi_sub_vendor_name(void)
{
	return sub_vendor;
}

/* Currently supporting Clock and Reset Domain */
static const uint8_t plat_protocol_list[] = {
	SCMI_PROTOCOL_ID_CLOCK,
	SCMI_PROTOCOL_ID_RESET_DOMAIN,
	0 /* Null termination */
};

size_t plat_scmi_protocol_count(void)
{
	const size_t count = ARRAY_SIZE(plat_protocol_list) - 1;

	assert(count == plat_scmi_protocol_count_paranoid());

	return count;
}

const uint8_t *plat_scmi_protocol_list(unsigned int agent_id __unused)
{

	assert(plat_scmi_protocol_count_paranoid() ==
	       (ARRAY_SIZE(plat_protocol_list) - 1));

	return plat_protocol_list;
}

/*
 * Platform SCMI clocks
 */
struct stm32_scmi_clk stm32_scmi_clock[] = {
	[0] = { .clock_id = CK_HSE, .name = "clk-hse", .enabled = true },
	[1] = { .clock_id = CK_HSI, .name = "clk-hsi", .enabled = true },
	[2] = { .clock_id = CK_CSI, .name = "clk-csi", .enabled = true },
	[3] = { .clock_id = CK_LSE, .name = "clk-lse", .enabled = true },
	[4] = { .clock_id = CK_LSI, .name = "clk-lsi", .enabled = true },
};

static struct stm32_scmi_clk *find_clock(unsigned int agent_id,
					 unsigned int scmi_id)
{
	size_t n = 0;
	struct stm32_scmi_clk *clk = NULL;

	if (agent_id == 0) {
		for (n = 0; n < ARRAY_SIZE(stm32_scmi_clock); n++)
			if (n == scmi_id)
				break;

		if (n < ARRAY_SIZE(stm32_scmi_clock))
			clk = &stm32_scmi_clock[n];
	}

	return clk;
}

size_t plat_scmi_clock_count(unsigned int agent_id)
{
	if (agent_id == 0)
		return ARRAY_SIZE(stm32_scmi_clock);
	else
		return 0;
}

const char *plat_scmi_clock_get_name(unsigned int agent_id,
				     unsigned int scmi_id)
{
	/* find_rd() returns NULL if clock exists for denied the agent */
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if (clock == NULL) {
		return NULL;
	}

	return clock->name;
}

int32_t plat_scmi_clock_rates_array(unsigned int agent_id, unsigned int scmi_id,
				    unsigned long *array, size_t *nb_elts)
{
	/* find_rd() returns NULL if clock exists for denied the agent */
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if (clock == NULL) {
		return SCMI_NOT_FOUND;
	}

	if (array == NULL) {
		*nb_elts = 1;

		return SCMI_SUCCESS;
	} else if (*nb_elts == 1) {
		*array = stm32mp_clk_get_rate(clock->clock_id);

		return SCMI_SUCCESS;
	} else {
		return SCMI_GENERIC_ERROR;
	}
}

unsigned long plat_scmi_clock_get_current_rate(unsigned int agent_id,
					       unsigned int scmi_id)
{
	/* find_rd() returns NULL if clock exists for denied the agent */
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if (!clock) {
		return 0;
	}

	return stm32mp_clk_get_rate(clock->clock_id);
}

int32_t plat_scmi_clock_get_state(unsigned int agent_id, unsigned int scmi_id)
{
	/* find_rd() returns NULL if clock exists for denied the agent */
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if (!clock) {
		return 0;
	}

	return (int32_t)clock->enabled;
}

int32_t plat_scmi_clock_set_state(unsigned int agent_id, unsigned int scmi_id,
				  bool enable_not_disable)
{
	/* find_rd() returns NULL if clock exists for denied the agent */
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if (!clock) {
		return SCMI_NOT_FOUND;
	}

	if (enable_not_disable) {
		if (!clock->enabled) {
			stm32mp_clk_enable(clock->clock_id);
			clock->enabled = true;
		}
	} else {
		if (clock->enabled) {
			stm32mp_clk_disable(clock->clock_id);
			clock->enabled = false;
		}
	}

	return SCMI_SUCCESS;
}

/*
 * Platform SCMI reset domains
 */
struct stm32_scmi_rd {
	unsigned long reset_id;
	const char *name;
};

struct stm32_scmi_rd stm32_scmi_reset_domain[] = {
	[0] = { .reset_id = SPI6_R, .name = "spi6" },
	[1] = { .reset_id = I2C4_R, .name = "i2c4" },
	[2] = { .reset_id = I2C6_R, .name = "i2c6" },
	[3] = { .reset_id = USART1_R, .name = "usart1" },
};

static struct stm32_scmi_rd *find_rd(unsigned int agent_id,
				     unsigned int scmi_id)
{
	size_t n = 0;
	struct stm32_scmi_rd *rd = NULL;

	if (agent_id == 0) {
		for (n = 0; n < ARRAY_SIZE(stm32_scmi_reset_domain); n++)
			if (n == scmi_id)
				break;

		if (n < ARRAY_SIZE(stm32_scmi_reset_domain))
			rd = &stm32_scmi_reset_domain[n];
	}

	return rd;
}

const char *plat_scmi_rd_get_name(unsigned int agent_id, unsigned int scmi_id)
{
	/* find_rd() returns NULL is reset exists for denied the agent */
	const struct stm32_scmi_rd *rd = find_rd(agent_id, scmi_id);

	if (rd == NULL) {
		return NULL;
	}

	return rd->name;
}

size_t plat_scmi_rd_count(unsigned int agent_id)
{
	if (agent_id == 0)
		return ARRAY_SIZE(stm32_scmi_reset_domain);
	else
		return 0;
}

int32_t plat_scmi_rd_autonomous(unsigned int agent_id, unsigned int scmi_id,
				uint32_t state)
{
	/* find_rd() returns NULL is reset exists for denied the agent */
	const struct stm32_scmi_rd *rd = find_rd(agent_id, scmi_id);

	if (!rd)
		return SCMI_NOT_FOUND;

	/* Supports only full reset with context loss */
	if (state)
		return SCMI_NOT_SUPPORTED;

	if (state)
		return SCMI_DENIED;

	if (stm32mp_reset_assert(rd->reset_id))
		return SCMI_HARDWARE_ERROR;

	if (stm32mp_reset_deassert(rd->reset_id))
		return SCMI_HARDWARE_ERROR;

	return SCMI_SUCCESS;
}

int32_t plat_scmi_rd_set_state(unsigned int agent_id, unsigned int scmi_id,
			       bool assert_not_deassert)
{
	/* find_rd() returns NULL is reset exists for denied the agent */
	const struct stm32_scmi_rd *rd = find_rd(agent_id, scmi_id);

	if (!rd)
		return SCMI_NOT_FOUND;

	if (assert_not_deassert) {
		stm32mp_reset_assert(rd->reset_id);
	} else {
		stm32mp_reset_deassert(rd->reset_id);
	}

	return SCMI_SUCCESS;
}

/*
 * Initialize platform SCMI resources
 */
void stm32mp1_init_scmi_server(void)
{
	size_t j = 0;

	scmi_smt_init_agent_channel(0);

	/* Synchronise SCMI clocks with their target init state */
	for (j = 0; j < ARRAY_SIZE(stm32_scmi_clock); j++) {
		struct stm32_scmi_clk *clk = &stm32_scmi_clock[j];

		if (clk->enabled) {
			stm32mp_clk_enable(clk->clock_id);
		}
	}
}
