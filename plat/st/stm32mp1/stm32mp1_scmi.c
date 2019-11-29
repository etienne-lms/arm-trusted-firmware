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
#include <drivers/scmi-server.h>
#include <drivers/scmi.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <dt-bindings/reset/stm32mp1-resets.h>

#include <stm32mp_shared_resources.h>
#include <stm32mp_common.h>

#define TIMEOUT_US_1MS		1000

#define SCMI_AGENT_COUNT	1

/* Locate all non-secure SMT message buffers in last page of SYSRAM */
#define SMT_BUFFER_BASE		STM32MP_NS_SYSRAM_BASE

static struct scmi_server_channel scmi_channel[SCMI_AGENT_COUNT] = {
	[0] = {
		.shm_addr = SMT_BUFFER_BASE,
		.shm_size = SMT_BUF_SLOT_SIZE,
	},
};

struct scmi_server_channel *plat_scmi_get_channel(unsigned int agent_id)
{
	assert(agent_id < ARRAY_SIZE(scmi_channel));

	return &scmi_channel[agent_id];
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
	CLOCK_CELL(CK_SCMI_HSE, CK_HSE, "clk-hse", true),
	CLOCK_CELL(CK_SCMI_HSI, CK_HSI, "clk-hsi", true),
	CLOCK_CELL(CK_SCMI_CSI, CK_CSI, "clk-csi", true),
	CLOCK_CELL(CK_SCMI_LSE, CK_LSE, "clk-lse", true),
	CLOCK_CELL(CK_SCMI_LSI, CK_LSI, "clk-lsi", true),
	CLOCK_CELL(CK_SCMI_PLL1_P, PLL1_P, "pll1_p", true),
	CLOCK_CELL(CK_SCMI_PLL1_Q, PLL1_Q, "pll1_q", true),
	CLOCK_CELL(CK_SCMI_PLL1_R, PLL1_R, "pll1_r", true),
	CLOCK_CELL(CK_SCMI_PLL2_P, PLL2_P, "pll2_p", true),
	CLOCK_CELL(CK_SCMI_PLL2_Q, PLL2_Q, "pll2_q", true),
	CLOCK_CELL(CK_SCMI_PLL2_R, PLL2_R, "pll2_r", true),
	CLOCK_CELL(CK_SCMI_PLL3_P, PLL3_P, "pll3_p", true),
	CLOCK_CELL(CK_SCMI_PLL3_Q, PLL3_Q, "pll3_q", true),
	CLOCK_CELL(CK_SCMI_PLL3_R, PLL3_R, "pll3_r", true),
	CLOCK_CELL(CK_SCMI_SPI6, SPI6_K, "spi6_k", false),
	CLOCK_CELL(CK_SCMI_I2C4, I2C4_K, "i2c4_k", false),
	CLOCK_CELL(CK_SCMI_I2C6, I2C6_K, "i2c6_k", false),
	CLOCK_CELL(CK_SCMI_USART1, USART1_K, "usart1_k", false),
	CLOCK_CELL(CK_SCMI_RTCAPB, RTCAPB, "rtcapb", false),
	CLOCK_CELL(CK_SCMI_IWDG1, IWDG1, "iwdg1", false),
	CLOCK_CELL(CK_SCMI_GPIOZ, GPIOZ, "gpioz", false),
	CLOCK_CELL(CK_SCMI_CRYP1, CRYP1, "cryp1", false),
	CLOCK_CELL(CK_SCMI_HASH1, HASH1, "hash1", false),
	CLOCK_CELL(CK_SCMI_RNG1, RNG1_K, "rng1", false),
	CLOCK_CELL(CK_SCMI_RTC, RTC, "ck_rtc", false),
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
	RESET_CELL(RST_SCMI_SPI6, SPI6_R, "spi6"),
	RESET_CELL(RST_SCMI_I2C4, I2C4_R, "i2c4"),
	RESET_CELL(RST_SCMI_I2C6, I2C6_R, "i2c6"),
	RESET_CELL(RST_SCMI_USART1, USART1_R, "usart1"),
	RESET_CELL(RST_SCMI_STGEN, STGEN_R, "stgen"),
	RESET_CELL(RST_SCMI_GPIOZ, GPIOZ_R, "gpioz"),
	RESET_CELL(RST_SCMI_CRYP1, CRYP1_R, "cryp1"),
	RESET_CELL(RST_SCMI_HASH1, HASH1_R, "hash1"),
	RESET_CELL(RST_SCMI_RNG1, RNG1_R, "rng1"),
	RESET_CELL(RST_SCMI_MDMA, MDMA_R, "mdma"),
	RESET_CELL(RST_SCMI_MCU, MCU_R, "mcu"),
};

struct scmi_agent_resources {
	struct stm32_scmi_clk *clock;
	size_t clock_count;
	struct stm32_scmi_rd *rd;
	size_t rd_count;
	struct stm32_scmi_pd *pd;
	size_t pd_count;
	struct stm32_scmi_perfs *perfs;
	size_t perfs_count;
};

/* Currently only 1 agent: ID is 0 */
const struct scmi_agent_resources agent_resources[] = {
	[0] = {
		.clock = stm32_scmi_clock,
		.clock_count = ARRAY_SIZE(stm32_scmi_clock),
		.rd = stm32_scmi_reset_domain,
		.rd_count = ARRAY_SIZE(stm32_scmi_reset_domain),
	},
};

static const struct scmi_agent_resources *find_resource(unsigned int agent_id)
{
	assert(agent_id < ARRAY_SIZE(agent_resources));

	return &agent_resources[agent_id];
}

#if ENABLE_ASSERTIONS
static size_t plat_scmi_protocol_count_paranoid(void)
{
	unsigned int n = 0;
	unsigned int count = 0;

	for (n = 0; n < ARRAY_SIZE(agent_resources); n++) {
		if (agent_resources[n].clock_count) {
			count++;
			break;
		}
	}

	for (n = 0; n < ARRAY_SIZE(agent_resources); n++) {
		if (agent_resources[n].rd_count) {
			count++;
			break;
		}
	}

	for (n = 0; n < ARRAY_SIZE(agent_resources); n++) {
		if (agent_resources[n].pd_count) {
			count++;
			break;
		}
	}

	for (n = 0; n < ARRAY_SIZE(agent_resources); n++) {
		if (agent_resources[n].perfs_count) {
			count++;
			break;
		}
	}

	return count;
}
#endif

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
static struct stm32_scmi_clk *find_clock(unsigned int agent_id,
					 unsigned int scmi_id)
{
	const struct scmi_agent_resources *res = find_resource(agent_id);
	struct stm32_scmi_clk *clock = NULL;
	unsigned int scmi_id_masked = scmi_id & GENMASK_32(7, 0);

	/* Assume less than 256 clock IDs exposed, used to mask ID value */
	if (scmi_id > scmi_id_masked) {
		ERROR("Outbound scmi_id %u above 256", scmi_id);
		return NULL;
	}

	if (res) {
		clock = res->clock + scmi_id_masked;

		if (!clock->name ||
		    !stm32mp_nsec_can_access_clock(clock->clock_id))
			clock = NULL;
	}

	return clock;
}

size_t plat_scmi_clock_count(unsigned int agent_id)
{
	const struct scmi_agent_resources *res = find_resource(agent_id);

	if (!res)
		return 0;

	return res->clock_count;
}

const char *plat_scmi_clock_get_name(unsigned int agent_id,
				     unsigned int scmi_id)
{
	/* find_rd() returns NULL is clock exists for denied the agent */
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if (!clock)
		return NULL;

	return clock->name;
}

unsigned long plat_scmi_clock_get_rate(unsigned int agent_id,
				       unsigned int scmi_id)
{
	/* find_rd() returns NULL is clock exists for denied the agent */
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if (!clock)
		return 0;

	return stm32mp_clk_get_rate(clock->clock_id);
}

int32_t plat_scmi_clock_set_rate(unsigned int agent_id __unused,
				 unsigned int scmi_id __unused,
				 unsigned long rate __unused)
{
	return SCMI_NOT_SUPPORTED;
}

int plat_scmi_clock_get_state(unsigned int agent_id, unsigned int scmi_id)
{
	/* find_rd() returns NULL is clock exists for denied the agent */
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if (!clock)
		return 0;

	return (int)clock->enabled;
}

int32_t plat_scmi_clock_set_state(unsigned int agent_id, unsigned int scmi_id,
				  bool enable_not_disable)
{
	/* find_rd() returns NULL is clock exists for denied the agent */
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if (!clock)
		return SCMI_NOT_FOUND;

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
static struct stm32_scmi_rd *find_rd(unsigned int agent_id,
				     unsigned int scmi_id)
{
	const struct scmi_agent_resources *res = find_resource(agent_id);
	struct stm32_scmi_rd *reset = NULL;
	unsigned int scmi_id_masked = scmi_id & GENMASK_32(7, 0);

	/* Assume less than 256 reset domain IDs exposed */
	if (scmi_id > scmi_id_masked) {
		ERROR("Outbound scmi_id %u", scmi_id);
		return NULL;
	}

	if (res) {
		reset = res->rd + scmi_id_masked;

		if (!reset->name ||
		    !stm32mp_nsec_can_access_reset(reset->reset_id))
			reset = NULL;
	}

	return reset;
}

const char *plat_scmi_rd_get_name(unsigned int agent_id, unsigned int scmi_id)
{
	/* find_rd() returns NULL is reset exists for denied the agent */
	const struct stm32_scmi_rd *rd = find_rd(agent_id, scmi_id);

	if (!rd)
		return NULL;

	return rd->name;
}

size_t plat_scmi_rd_count(unsigned int agent_id)
{
	const struct scmi_agent_resources *res = find_resource(agent_id);

	if (!res)
		return 0;

	return res->rd_count;
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

	if (stm32mp_reset_assert_to(rd->reset_id, TIMEOUT_US_1MS))
		return SCMI_HARDWARE_ERROR;

	if (stm32mp_reset_deassert_to(rd->reset_id, TIMEOUT_US_1MS))
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

const char *plat_scmi_vendor_name(void)
{
	static const char vendor[] = "ST";

	return vendor;

}

const char *plat_scmi_sub_vendor_name(void)
{
	static const char sub_vendor[] = "";

	return sub_vendor;
}

/*
 * Initialize platform SCMI resources
 */
void stm32mp1_init_scmi_server(void)
{
	size_t i = 0;
	size_t j = 0;

	scmi_smt_init_agent_channel(0);

	/* Synchronise SCMI clocks with their target init state */
	for (i = 0; i < ARRAY_SIZE(agent_resources); i++) {
		const struct scmi_agent_resources *res = &agent_resources[i];

		for (j = 0; j < res->clock_count; j++) {
			struct stm32_scmi_clk *clk = &res->clock[j];

			if (!clk->enabled)
				continue;

			assert(stm32mp_nsec_can_access_clock(clk->clock_id));
			stm32mp_clk_enable(clk->clock_id);
		}
	}
}
