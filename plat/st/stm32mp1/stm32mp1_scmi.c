/*
 * Copyright (c) 2019-2020, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <platform_def.h>

#include <drivers/st/scmi-msg.h>
#include <drivers/st/scmi.h>
#include <drivers/st/stm32mp_pmic.h>
#include <drivers/st/stm32mp_reset.h>
#include <drivers/st/stm32mp1_clk.h>
#include <drivers/st/stm32mp1_pwr.h>
#include <drivers/st/stpmic1.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <dt-bindings/regulator/st,stm32mp15-regulator.h>
#include <dt-bindings/reset/stm32mp1-resets.h>

#define TIMEOUT_US_1MS		1000U

#define SCMI_CLOCK_NAME_SIZE	16U
#define SCMI_RSTD_NAME_SIZE	16U
#define SCMI_VOLTD_NAME_SIZE	16U

/*
 * struct stm32_scmi_clk - Data for the exposed clock
 * @clock_id: Clock identifier in RCC clock driver
 * @name: Clock string ID exposed to agent
 * @enabled: State of the SCMI clock
 */
struct stm32_scmi_clk {
	unsigned long clock_id;
	const char *name;
	bool enabled;
};

/*
 * struct stm32_scmi_rstd - Data for the exposed reset controller
 * @reset_id: Reset identifier in RCC reset driver
 * @name: Reset string ID exposed to agent
 */
struct stm32_scmi_rstd {
	unsigned long reset_id;
	const char *name;
};

enum voltd_device {
	VOLTD_PWR,
	VOLTD_PMIC,
};

/*
 * struct stm32_scmi_voltd - Data for the exposed voltage domains
 * @name: Power regulator string ID exposed to agent
 * @priv_id: Internal string ID for the regulator
 * @priv_dev: Internal ID for the device implementing the regulator
 */
struct stm32_scmi_voltd {
	const char *name;
	const char *priv_id;
	enum voltd_device priv_dev;

};

/* Locate all non-secure SMT message buffers in last page of SYSRAM */
#define SMT_BUFFER_BASE		STM32MP_SCMI_NS_SHM_BASE
#define SMT_BUFFER0_BASE	SMT_BUFFER_BASE
#define SMT_BUFFER1_BASE	(SMT_BUFFER_BASE + 0x200)
#define SMT_BUFFER2_BASE	(SMT_BUFFER_BASE + 0x400)

CASSERT((STM32MP_SCMI_NS_SHM_BASE + STM32MP_SCMI_NS_SHM_SIZE) >=
	(SMT_BUFFER2_BASE + SMT_BUF_SLOT_SIZE),
	assert_scmi_non_secure_shm_fits_scmi_overall_buffer_size);

static struct scmi_msg_channel scmi_channel[] = {
	[0] = {
		.shm_addr = SMT_BUFFER0_BASE,
		.shm_size = SMT_BUF_SLOT_SIZE,
	},
	[1] = {
		.shm_addr = SMT_BUFFER1_BASE,
		.shm_size = SMT_BUF_SLOT_SIZE,
	},
	[2] = {
		.shm_addr = SMT_BUFFER2_BASE,
		.shm_size = SMT_BUF_SLOT_SIZE,
	},
};

struct scmi_msg_channel *plat_scmi_get_channel(unsigned int agent_id)
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

static struct stm32_scmi_clk stm32_scmi0_clock[] = {
	CLOCK_CELL(CK_SCMI0_HSE, CK_HSE, "ck_hse", true),
	CLOCK_CELL(CK_SCMI0_HSI, CK_HSI, "ck_hsi", true),
	CLOCK_CELL(CK_SCMI0_CSI, CK_CSI, "ck_csi", true),
	CLOCK_CELL(CK_SCMI0_LSE, CK_LSE, "ck_lse", true),
	CLOCK_CELL(CK_SCMI0_LSI, CK_LSI, "ck_lsi", true),
	CLOCK_CELL(CK_SCMI0_PLL2_Q, PLL2_Q, "pll2_q", true),
	CLOCK_CELL(CK_SCMI0_PLL2_R, PLL2_R, "pll2_r", true),
	CLOCK_CELL(CK_SCMI0_MPU, CK_MPU, "ck_mpu", true),
	CLOCK_CELL(CK_SCMI0_AXI, CK_AXI, "ck_axi", true),
	CLOCK_CELL(CK_SCMI0_BSEC, BSEC, "bsec", true),
	CLOCK_CELL(CK_SCMI0_CRYP1, CRYP1, "cryp1", false),
	CLOCK_CELL(CK_SCMI0_GPIOZ, GPIOZ, "gpioz", false),
	CLOCK_CELL(CK_SCMI0_HASH1, HASH1, "hash1", false),
	CLOCK_CELL(CK_SCMI0_I2C4, I2C4_K, "i2c4_k", false),
	CLOCK_CELL(CK_SCMI0_I2C6, I2C6_K, "i2c6_k", false),
	CLOCK_CELL(CK_SCMI0_IWDG1, IWDG1, "iwdg1", false),
	CLOCK_CELL(CK_SCMI0_RNG1, RNG1_K, "rng1_k", true),
	CLOCK_CELL(CK_SCMI0_RTC, RTC, "ck_rtc", true),
	CLOCK_CELL(CK_SCMI0_RTCAPB, RTCAPB, "rtcapb", true),
	CLOCK_CELL(CK_SCMI0_SPI6, SPI6_K, "spi6_k", false),
	CLOCK_CELL(CK_SCMI0_USART1, USART1_K, "usart1_k", false),
};

static struct stm32_scmi_clk stm32_scmi1_clock[] = {
	CLOCK_CELL(CK_SCMI1_PLL3_Q, PLL3_Q, "pll3_q", true),
	CLOCK_CELL(CK_SCMI1_PLL3_R, PLL3_R, "pll3_r", true),
	CLOCK_CELL(CK_SCMI1_MCU, CK_MCU, "ck_mcu", false),
};

#define RESET_CELL(_scmi_id, _id, _name) \
	[_scmi_id] = { \
		.reset_id = _id, \
		.name = _name, \
	}

static struct stm32_scmi_rstd stm32_scmi0_reset_domain[] = {
	RESET_CELL(RST_SCMI0_SPI6, SPI6_R, "spi6"),
	RESET_CELL(RST_SCMI0_I2C4, I2C4_R, "i2c4"),
	RESET_CELL(RST_SCMI0_I2C6, I2C6_R, "i2c6"),
	RESET_CELL(RST_SCMI0_USART1, USART1_R, "usart1"),
	RESET_CELL(RST_SCMI0_STGEN, STGEN_R, "stgen"),
	RESET_CELL(RST_SCMI0_GPIOZ, GPIOZ_R, "gpioz"),
	RESET_CELL(RST_SCMI0_CRYP1, CRYP1_R, "cryp1"),
	RESET_CELL(RST_SCMI0_HASH1, HASH1_R, "hash1"),
	RESET_CELL(RST_SCMI0_RNG1, RNG1_R, "rng1"),
	RESET_CELL(RST_SCMI0_MDMA, MDMA_R, "mdma"),
	RESET_CELL(RST_SCMI0_MCU, MCU_R, "mcu"),
	RESET_CELL(RST_SCMI0_MCU_HOLD_BOOT, MCU_HOLD_BOOT_R, "mcu_hold_boot"),
};

#define VOLTD_CELL(_scmi_id, _dev_id, _priv_id, _name) \
	[_scmi_id] = { \
		.priv_id = (_priv_id), \
		.priv_dev = (_dev_id), \
		.name = (_name), \
	}

struct stm32_scmi_voltd scmi0_voltage_domain[] = {
	VOLTD_CELL(VOLTD_SCMI0_REG11, VOLTD_PWR, "0", "reg11"),
	VOLTD_CELL(VOLTD_SCMI0_REG18, VOLTD_PWR, "1", "reg18"),
	VOLTD_CELL(VOLTD_SCMI0_USB33, VOLTD_PWR, "2", "usb33"),
};

struct stm32_scmi_voltd scmi2_voltage_domain[] = {
	VOLTD_CELL(VOLTD_SCMI2_BUCK1, VOLTD_PMIC, "buck1", "vddcore"),
	VOLTD_CELL(VOLTD_SCMI2_BUCK2, VOLTD_PMIC, "buck2", "vdd_ddr"),
	VOLTD_CELL(VOLTD_SCMI2_BUCK3, VOLTD_PMIC, "buck3", "vdd"),
	VOLTD_CELL(VOLTD_SCMI2_BUCK4, VOLTD_PMIC, "buck4", "v3v3"),
	VOLTD_CELL(VOLTD_SCMI2_LDO1, VOLTD_PMIC, "ldo1", "v1v8_audio"),
	VOLTD_CELL(VOLTD_SCMI2_LDO2, VOLTD_PMIC, "ldo2", "v3v3_hdmi"),
	VOLTD_CELL(VOLTD_SCMI2_LDO3, VOLTD_PMIC, "ldo3", "vtt_ddr"),
	VOLTD_CELL(VOLTD_SCMI2_LDO4, VOLTD_PMIC, "ldo4", "vdd_usb"),
	VOLTD_CELL(VOLTD_SCMI2_LDO5, VOLTD_PMIC, "ldo5", "vdda"),
	VOLTD_CELL(VOLTD_SCMI2_LDO6, VOLTD_PMIC, "ldo6", "v1v2_hdmi"),
	VOLTD_CELL(VOLTD_SCMI2_VREFDDR, VOLTD_PMIC, "vref_ddr", "vref_ddr"),
	VOLTD_CELL(VOLTD_SCMI2_BOOST, VOLTD_PMIC, "boost", "bst_out"),
	VOLTD_CELL(VOLTD_SCMI2_PWR_SW1, VOLTD_PMIC, "pwr_sw1", "vbus_otg"),
	VOLTD_CELL(VOLTD_SCMI2_PWR_SW2, VOLTD_PMIC, "pwr_sw2", "vbus_sw"),
};

struct scmi_agent_resources {
	struct stm32_scmi_clk *clock;
	size_t clock_count;
	struct stm32_scmi_rstd *rstd;
	size_t rstd_count;
	struct stm32_scmi_voltd *voltd;
	size_t voltd_count;
};

static const struct scmi_agent_resources agent_resources[] = {
	[0] = {
		.clock = stm32_scmi0_clock,
		.clock_count = ARRAY_SIZE(stm32_scmi0_clock),
		.rstd = stm32_scmi0_reset_domain,
		.rstd_count = ARRAY_SIZE(stm32_scmi0_reset_domain),
		.voltd = scmi0_voltage_domain,
		.voltd_count = ARRAY_SIZE(scmi0_voltage_domain),
	},
	[1] = {
		.clock = stm32_scmi1_clock,
		.clock_count = ARRAY_SIZE(stm32_scmi1_clock),
	},
	[2] = {
		.voltd = scmi2_voltage_domain,
		.voltd_count = ARRAY_SIZE(scmi2_voltage_domain),
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
	unsigned int n = 0U;
	unsigned int count = 0U;

	for (n = 0U; n < ARRAY_SIZE(agent_resources); n++) {
		if (agent_resources[n].clock_count) {
			count++;
			break;
		}
	}

	for (n = 0U; n < ARRAY_SIZE(agent_resources); n++) {
		if (agent_resources[n].rstd_count) {
			count++;
			break;
		}
	}

	for (n = 0U; n < ARRAY_SIZE(agent_resources); n++) {
		if (agent_resources[n].voltd_count) {
			count++;
			break;
		}
	}

	return count;
}
#endif

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

/* Currently supporting Clocks and Reset Domains */
static const uint8_t plat_protocol_list[] = {
	SCMI_PROTOCOL_ID_CLOCK,
	SCMI_PROTOCOL_ID_RESET_DOMAIN,
	SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN,
	0U /* Null termination */
};

size_t plat_scmi_protocol_count(void)
{
	const size_t count = ARRAY_SIZE(plat_protocol_list) - 1U;

	assert(count == plat_scmi_protocol_count_paranoid());

	return count;
}

const uint8_t *plat_scmi_protocol_list(unsigned int agent_id __unused)
{
	assert(plat_scmi_protocol_count_paranoid() ==
	       (ARRAY_SIZE(plat_protocol_list) - 1U));

	return plat_protocol_list;
}

/*
 * Platform SCMI clocks
 */
static struct stm32_scmi_clk *find_clock(unsigned int agent_id,
					 unsigned int scmi_id)
{
	const struct scmi_agent_resources *resource = find_resource(agent_id);
	size_t n = 0U;

	if (resource != NULL) {
		for (n = 0U; n < resource->clock_count; n++) {
			if (n == scmi_id) {
				return &resource->clock[n];
			}
		}
	}

	return NULL;
}

size_t plat_scmi_clock_count(unsigned int agent_id)
{
	const struct scmi_agent_resources *resource = find_resource(agent_id);

	if (resource == NULL) {
		return 0U;
	}

	return resource->clock_count;
}

const char *plat_scmi_clock_get_name(unsigned int agent_id,
				     unsigned int scmi_id)
{
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if ((clock == NULL) ||
	    !stm32mp_nsec_can_access_clock(clock->clock_id)) {
		return NULL;
	}

	return clock->name;
}

int32_t plat_scmi_clock_rates_array(unsigned int agent_id, unsigned int scmi_id,
				    size_t start_index, unsigned long *array,
				    size_t *nb_elts)
{
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if (clock == NULL) {
		return SCMI_NOT_FOUND;
	}

	if (!stm32mp_nsec_can_access_clock(clock->clock_id)) {
		return SCMI_DENIED;
	}

	/* Exposed clocks are currently fixed rate clocks */
	if (start_index) {
		return SCMI_INVALID_PARAMETERS;
	}

	if (array == NULL) {
		*nb_elts = 1U;
	} else if (*nb_elts == 1U) {
		*array = stm32mp_clk_get_rate(clock->clock_id);
	} else {
		return SCMI_GENERIC_ERROR;
	}

	return SCMI_SUCCESS;
}

unsigned long plat_scmi_clock_get_rate(unsigned int agent_id,
				       unsigned int scmi_id)
{
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if ((clock == NULL) ||
	    !stm32mp_nsec_can_access_clock(clock->clock_id)) {
		return 0U;
	}

	return stm32mp_clk_get_rate(clock->clock_id);
}

int32_t plat_scmi_clock_get_state(unsigned int agent_id, unsigned int scmi_id)
{
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if ((clock == NULL) ||
	    !stm32mp_nsec_can_access_clock(clock->clock_id)) {
		return 0U;
	}

	return (int32_t)clock->enabled;
}

int32_t plat_scmi_clock_set_state(unsigned int agent_id, unsigned int scmi_id,
				  bool enable_not_disable)
{
	struct stm32_scmi_clk *clock = find_clock(agent_id, scmi_id);

	if (clock == NULL) {
		return SCMI_NOT_FOUND;
	}

	if (!stm32mp_nsec_can_access_clock(clock->clock_id)) {
		return SCMI_DENIED;
	}

	if (enable_not_disable) {
		if (!clock->enabled) {
			VERBOSE("SCMI clock %u enable\n", scmi_id);
			stm32mp_clk_enable(clock->clock_id);
			clock->enabled = true;
		}
	} else {
		if (clock->enabled) {
			VERBOSE("SCMI clock %u disable\n", scmi_id);
			stm32mp_clk_disable(clock->clock_id);
			clock->enabled = false;
		}
	}

	return SCMI_SUCCESS;
}

/*
 * Platform SCMI reset domains
 */
static struct stm32_scmi_rstd *find_rstd(unsigned int agent_id,
					 unsigned int scmi_id)
{
	const struct scmi_agent_resources *resource = find_resource(agent_id);
	size_t n;

	if (resource != NULL) {
		for (n = 0U; n < resource->rstd_count; n++) {
			if (n == scmi_id) {
				return &resource->rstd[n];
			}
		}
	}

	return NULL;
}

const char *plat_scmi_rstd_get_name(unsigned int agent_id, unsigned int scmi_id)
{
	const struct stm32_scmi_rstd *rstd = find_rstd(agent_id, scmi_id);

	if (rstd == NULL) {
		return NULL;
	}

	return rstd->name;
}

size_t plat_scmi_rstd_count(unsigned int agent_id)
{
	const struct scmi_agent_resources *resource = find_resource(agent_id);

	if (resource == NULL) {
		return 0U;
	}

	return resource->rstd_count;
}

int32_t plat_scmi_rstd_autonomous(unsigned int agent_id, unsigned int scmi_id,
				uint32_t state)
{
	const struct stm32_scmi_rstd *rstd = find_rstd(agent_id, scmi_id);

	if (rstd == NULL) {
		return SCMI_NOT_FOUND;
	}

	if (!stm32mp_nsec_can_access_reset(rstd->reset_id)) {
		return SCMI_DENIED;
	}


	if (rstd->reset_id == MCU_HOLD_BOOT_R) {
		return SCMI_NOT_SUPPORTED;
	}

	/* Supports only reset with context loss */
	if (state != 0U) {
		return SCMI_NOT_SUPPORTED;
	}

	VERBOSE("SCMI reset %lu cycle\n", rstd->reset_id);

	if (stm32mp_reset_assert(rstd->reset_id, TIMEOUT_US_1MS)) {
		return SCMI_HARDWARE_ERROR;
	}

	if (stm32mp_reset_deassert(rstd->reset_id, TIMEOUT_US_1MS)) {
		return SCMI_HARDWARE_ERROR;
	}

	return SCMI_SUCCESS;
}

int32_t plat_scmi_rstd_set_state(unsigned int agent_id, unsigned int scmi_id,
				 bool assert_not_deassert)
{
	const struct stm32_scmi_rstd *rstd = find_rstd(agent_id, scmi_id);

	if (rstd == NULL) {
		return SCMI_NOT_FOUND;
	}

	if (!stm32mp_nsec_can_access_reset(rstd->reset_id)) {
		return SCMI_DENIED;
	}

	if (rstd->reset_id == MCU_HOLD_BOOT_R) {
		VERBOSE("SCMI MCU hold boot %s",
			assert_not_deassert ? "set" : "release");

		stm32mp_reset_assert_mcu_hold_boot(assert_not_deassert);

		return SCMI_SUCCESS;
	}

	if (assert_not_deassert) {
		VERBOSE("SCMI reset %lu set\n", rstd->reset_id);
		stm32mp_reset_set(rstd->reset_id);
	} else {
		VERBOSE("SCMI reset %lu release\n", rstd->reset_id);
		stm32mp_reset_release(rstd->reset_id);
	}

	return SCMI_SUCCESS;
}

/*
 * Platform SCMI voltage domains
 */
static struct stm32_scmi_voltd *find_voltd(unsigned int agent_id,
					   unsigned int scmi_id)
{
	const struct scmi_agent_resources *resource = find_resource(agent_id);
	size_t n = 0;

	if (resource != NULL) {
		for (n = 0U; n < resource->voltd_count; n++) {
			if (n == scmi_id) {
				return &resource->voltd[n];
			}
		}
	}

	return NULL;
}

size_t plat_scmi_voltd_count(unsigned int agent_id)
{
	const struct scmi_agent_resources *resource = find_resource(agent_id);

	if (resource == NULL) {
		return 0;
	}

	return resource->voltd_count;
}

const char *plat_scmi_voltd_get_name(unsigned int agent_id,
				     unsigned int scmi_id)
{
	struct stm32_scmi_voltd *voltd = find_voltd(agent_id, scmi_id);

	/* Currently non-secure is allowed to access all PWR regulators */
	if (voltd == NULL) {
		return NULL;
	}

	return voltd->name;
}

static enum pwr_regulator pwr_scmi_to_regu_id(struct stm32_scmi_voltd *voltd)
{
	size_t id = voltd->priv_id[0] - '0';

	assert(id < PWR_REGU_COUNT);
	return id;
}

static long pwr_get_level(struct stm32_scmi_voltd *voltd)
{
	enum pwr_regulator regu_id = pwr_scmi_to_regu_id(voltd);

	return (long)stm32mp1_pwr_regulator_mv(regu_id) * 1000;
}

static int32_t pwr_set_level(struct stm32_scmi_voltd *voltd, long level_uv)
{
	if (level_uv != pwr_get_level(voltd)) {
		return SCMI_INVALID_PARAMETERS;
	}

	return SCMI_SUCCESS;
}

static int32_t pwr_describe_levels(struct stm32_scmi_voltd *voltd,
				   size_t start_index, long *microvolt,
				   size_t *nb_elts)
{
	if (start_index) {
		return SCMI_INVALID_PARAMETERS;
	}

	if (!microvolt) {
		*nb_elts = 1;
		return SCMI_SUCCESS;
	}

	if (*nb_elts < 1) {
		return SCMI_GENERIC_ERROR;
	}

	*nb_elts = 1;
	*microvolt = pwr_get_level(voltd);

	return SCMI_SUCCESS;
}

static uint32_t pwr_get_state(struct stm32_scmi_voltd *voltd)
{
	enum pwr_regulator regu_id = pwr_scmi_to_regu_id(voltd);

	if (stm32mp1_pwr_regulator_is_enabled(regu_id)) {
		return SCMI_VOLTAGE_DOMAIN_CONFIG_ARCH_ON;
	}

	return SCMI_VOLTAGE_DOMAIN_CONFIG_ARCH_OFF;
}

static void pwr_set_state(struct stm32_scmi_voltd *voltd, bool enable)
{
	enum pwr_regulator regu_id = pwr_scmi_to_regu_id(voltd);

	VERBOSE("%sable PWR %s (was %s)", enable ? "En" : "Dis", voltd->name,
		stm32mp1_pwr_regulator_is_enabled(regu_id) ? "on" : "off");

	stm32mp1_pwr_regulator_set_state(regu_id, enable);
}

static int32_t pmic_describe_levels(struct stm32_scmi_voltd *voltd,
				    size_t start_index, long *microvolt,
				    size_t *nb_elts)
{
	const uint16_t *levels = NULL;
	size_t full_count = 0;
	size_t out_count = 0;
	size_t i = 0;

	if (!stm32mp_nsec_can_access_pmic_regu(voltd->priv_id)) {
		return SCMI_DENIED;
	}

	stpmic1_regulator_levels_mv(voltd->priv_id, &levels, &full_count);

	if (!microvolt) {
		*nb_elts = full_count - start_index;
		return SCMI_SUCCESS;
	}

	if (start_index >= full_count) {
		return SCMI_OUT_OF_RANGE;
	}

	out_count = MIN(full_count - start_index, *nb_elts);

	VERBOSE("%zu levels: start %zu requested %zu output %zu",
		full_count, start_index, *nb_elts, out_count);

	for (i = 0; i < out_count; i++) {
		microvolt[i] = levels[start_index + i] * 1000;
	}

	*nb_elts = out_count;

	return SCMI_SUCCESS;
}

static long pmic_get_level(struct stm32_scmi_voltd *voltd)
{
	unsigned long level_mv = 0;

	if (!stm32mp_nsec_can_access_pmic_regu(voltd->priv_id)) {
		return 0;
	}

	level_mv = stpmic1_regulator_voltage_get(voltd->priv_id);

	return (long)level_mv * 1000;
}

static int32_t pmic_set_level(struct stm32_scmi_voltd *voltd, long level_uv)
{
	unsigned int level_mv = 0;

	if (!stm32mp_nsec_can_access_pmic_regu(voltd->priv_id)) {
		return SCMI_DENIED;
	}

	if (level_uv < 0 || level_uv > (UINT16_MAX * 1000)) {
		return SCMI_INVALID_PARAMETERS;
	}

	level_mv = (unsigned int)level_uv / 1000;

	VERBOSE("Set STPMIC1 regulator %s level to %dmV", voltd->name,
		level_mv);

	if (stpmic1_regulator_voltage_set(voltd->priv_id, level_mv) != 0) {
	       return SCMI_GENERIC_ERROR;
	}

	return SCMI_SUCCESS;
}

static uint32_t pmic_get_state(struct stm32_scmi_voltd *voltd)
{
	if (stm32mp_nsec_can_access_pmic_regu(voltd->priv_id)) {
		return SCMI_VOLTAGE_DOMAIN_CONFIG_ARCH_OFF;
	}

	if (stpmic1_is_regulator_enabled(voltd->priv_id)) {
		return SCMI_VOLTAGE_DOMAIN_CONFIG_ARCH_ON;
	}

	return SCMI_VOLTAGE_DOMAIN_CONFIG_ARCH_OFF;
}

static int32_t pmic_set_state(struct stm32_scmi_voltd *voltd, bool enable)
{
	int rc = 0;

	if (!stm32mp_nsec_can_access_pmic_regu(voltd->priv_id)) {
		return SCMI_DENIED;
	}

	VERBOSE("%sable STPMIC1 %s (was %s)", enable ? "En" : "Dis",
		voltd->name,
		stpmic1_is_regulator_enabled(voltd->priv_id) ? "on" : "off");

	if (enable) {
		rc = stpmic1_regulator_enable(voltd->priv_id);
	}else {
		rc = stpmic1_regulator_disable(voltd->priv_id);
	}

	if (rc != 0) {
	       return SCMI_GENERIC_ERROR;
	}

	return SCMI_SUCCESS;
}

int32_t plat_scmi_voltd_levels_array(unsigned int agent_id,
				     unsigned int scmi_id, size_t start_index,
				     long *levels, size_t *nb_elts)

{
	struct stm32_scmi_voltd *voltd = find_voltd(agent_id, scmi_id);

	if (!voltd) {
		return SCMI_NOT_FOUND;
	}

	switch (voltd->priv_dev) {
	case VOLTD_PWR:
		return pwr_describe_levels(voltd, start_index, levels, nb_elts);
	case VOLTD_PMIC:
		return pmic_describe_levels(voltd, start_index, levels, nb_elts);
	default:
		return SCMI_GENERIC_ERROR;
	}
}

long plat_scmi_voltd_get_level(unsigned int agent_id, unsigned int scmi_id)
{
	struct stm32_scmi_voltd *voltd = find_voltd(agent_id, scmi_id);

	if (!voltd) {
		return 0;
	}

	switch (voltd->priv_dev) {
	case VOLTD_PWR:
		return pwr_get_level(voltd);
	case VOLTD_PMIC:
		return pmic_get_level(voltd);
	default:
		panic();
	}
}

int32_t plat_scmi_voltd_set_level(unsigned int agent_id, unsigned int scmi_id,
				  long level)
{
	struct stm32_scmi_voltd *voltd = find_voltd(agent_id, scmi_id);

	if (!voltd) {
		return SCMI_NOT_FOUND;
	}

	switch (voltd->priv_dev) {
	case VOLTD_PWR:
		return pwr_set_level(voltd, level);
	case VOLTD_PMIC:
		return pmic_set_level(voltd, level);
	default:
		return SCMI_GENERIC_ERROR;
	}
}

int32_t plat_scmi_voltd_get_config(unsigned int agent_id, unsigned int scmi_id,
				   uint32_t *config)
{
	struct stm32_scmi_voltd *voltd = find_voltd(agent_id, scmi_id);

	if (!voltd) {
		return SCMI_NOT_FOUND;
	}

	switch (voltd->priv_dev) {
	case VOLTD_PWR:
		*config = pwr_get_state(voltd);
		break;
	case VOLTD_PMIC:
		*config = pmic_get_state(voltd);
		break;
	default:
		return SCMI_GENERIC_ERROR;
	}

	return SCMI_SUCCESS;
}

int32_t plat_scmi_voltd_set_config(unsigned int agent_id, unsigned int scmi_id,
				   uint32_t config)
{
	struct stm32_scmi_voltd *voltd = find_voltd(agent_id, scmi_id);
	int32_t rc = SCMI_SUCCESS;

	if (!voltd) {
		return SCMI_NOT_FOUND;
	}

	switch (voltd->priv_dev) {
	case VOLTD_PWR:
		pwr_set_state(voltd, config);
		break;
	case VOLTD_PMIC:
		rc = pmic_set_state(voltd, config);
		break;
	default:
		return SCMI_GENERIC_ERROR;
	}

	return rc;
}

/*
 * Initialize platform SCMI resources
 */
void stm32mp1_init_scmi_server(void)
{
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(scmi_channel); i++) {
		scmi_smt_init_agent_channel(&scmi_channel[i]);
	}

	for (i = 0U; i < ARRAY_SIZE(agent_resources); i++) {
		const struct scmi_agent_resources *res = &agent_resources[i];
		size_t j;

		for (j = 0U; j < res->clock_count; j++) {
			struct stm32_scmi_clk *clk = &res->clock[j];

			if ((clk->name == NULL) ||
			    (strlen(clk->name) >= SCMI_CLOCK_NAME_SIZE)) {
				ERROR("Invalid SCMI clock name\n");
				panic();
			}

			/* Sync SCMI clocks with their targeted initial state */
			if (clk->enabled &&
			    stm32mp_nsec_can_access_clock(clk->clock_id)) {
				stm32mp_clk_enable(clk->clock_id);
			}
		}

		for (j = 0U; j < res->rstd_count; j++) {
			struct stm32_scmi_rstd *rstd = &res->rstd[j];

			if ((rstd->name == NULL) ||
			    (strlen(rstd->name) >= SCMI_RSTD_NAME_SIZE)) {
				ERROR("Invalid SCMI reset domain name\n");
				panic();
			}
		}

		for (j = 0U; j < res->voltd_count; j++) {
			struct stm32_scmi_voltd *voltd = &res->voltd[j];

			if ((voltd->name == NULL) ||
			    (strlen(voltd->name) >= SCMI_RSTD_NAME_SIZE)) {
				ERROR("Invalid SCMI voltage domain name\n");
				panic();
			}
		}
	}
}
