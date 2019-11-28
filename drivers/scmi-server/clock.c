// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019, Linaro Limited
 */
#include <assert.h>
#include <string.h>

#include <lib/cassert.h>
#include <lib/utils.h>

#include <drivers/scmi-server.h>
#include <drivers/scmi.h>

#include "common.h"

static bool message_id_is_supported(unsigned int message_id);

static void report_version(struct scmi_msg *msg)
{
	struct scmi_protocol_version_p2a return_values = {
		.status = SCMI_SUCCESS,
		.version = SCMI_PROTOCOL_VERSION_CLOCK,
	};

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void report_attributes(struct scmi_msg *msg)
{
	struct scmi_protocol_attributes_p2a return_values = {
	        .status = SCMI_SUCCESS,
		.attributes = SCMI_CLOCK_PROTOCOL_ATTRIBUTES(
			1 /*  Max pending transactions */,
			plat_scmi_clock_count(msg->agent_id)),
	};

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void report_message_attributes(struct scmi_msg *msg)
{
	struct scmi_protocol_message_attributes_a2p *inargs = (void *)msg->in;
	struct scmi_protocol_message_attributes_p2a return_values = {
		.status = SCMI_SUCCESS,
		.attributes = 0,
	};

	if (!message_id_is_supported(inargs->message_id))
		scmi_status_response(msg, SCMI_NOT_FOUND);
	else
		scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void scmi_clock_attributes(struct scmi_msg *msg)
{
	const struct scmi_clock_attributes_a2p *inargs = (void *)msg->in;
	struct scmi_clock_attributes_p2a return_values;
	const char *name;

	name = plat_scmi_clock_get_name(msg->agent_id, inargs->clock_id);
	if (!name) {
		scmi_status_response(msg, SCMI_NOT_FOUND);
		return;
	}
	assert(strlen(name) < SCMI_CLOCK_NAME_LENGTH_MAX);

	/* zero status reflects return_values.status = SCMI_SUCCESS */
	zeromem(&return_values, sizeof(return_values));

	return_values.attributes = plat_scmi_clock_get_state(msg->agent_id,
							     inargs->clock_id);

	memcpy(return_values.clock_name, name,
	       strnlen(name, sizeof(return_values.clock_name)));

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void scmi_clock_rate_get(struct scmi_msg *msg)
{
	const struct scmi_clock_rate_get_a2p *in_args = (void *)msg->in;
	unsigned long rate = 0;
	struct scmi_clock_rate_get_p2a return_values;

	rate = plat_scmi_clock_get_rate(msg->agent_id, in_args->clock_id);

	zeromem(&return_values, sizeof(return_values));
	return_values.rate[0] = (uint32_t)rate;
	return_values.rate[1] = (uint32_t)((uint64_t)rate >> 32);

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void scmi_clock_rate_set(struct scmi_msg *msg)
{
	scmi_status_response(msg, SCMI_NOT_SUPPORTED);
}

static void scmi_clock_config_set(struct scmi_msg *msg)
{
	const struct scmi_clock_config_set_a2p *inargs = NULL;
	int32_t status = SCMI_GENERIC_ERROR;
	bool enable = false;

	inargs = (const struct scmi_clock_config_set_a2p*)msg->in;
	enable = inargs->attributes & SCMI_CLOCK_CONFIG_SET_ENABLE_MASK;

	status = plat_scmi_clock_set_state(msg->agent_id, inargs->clock_id,
					   enable);

	scmi_status_response(msg, status);
}

/* Currently support only single rate */
static void scmi_clock_describe_rates(struct scmi_msg *msg)
{
	const struct scmi_clock_describe_rates_a2p *inargs = (void *)msg->in;
	struct scmi_clock_describe_rates_p2a p2a;
	char outargs[sizeof(p2a) + 2 * sizeof(uint32_t)];

	zeromem(&outargs, sizeof(outargs));
	zeromem(&p2a, sizeof(p2a));

	p2a.status = SCMI_SUCCESS;
	p2a.num_rates_flags = 0;

	if (inargs->rate_index == 0) {
		unsigned long rate;
		uint32_t rate_low;
		uint32_t rate_high;

		p2a.num_rates_flags =
			SCMI_CLOCK_DESCRIBE_RATES_NUM_RATES_FLAGS(1,
					SCMI_CLOCK_RATE_FORMAT_LIST, 0);

		rate = plat_scmi_clock_get_rate(msg->agent_id,
						inargs->clock_id);
		rate_low = (uint32_t)rate;
		rate_high = (uint32_t)((uint64_t)rate >> 32);
		memcpy(outargs + sizeof(p2a),
		       &rate_low, sizeof(uint32_t));
		memcpy(outargs + sizeof(p2a) + sizeof(uint32_t),
		       &rate_high, sizeof(uint32_t));
	}

	scmi_write_response(msg, &outargs, sizeof(outargs));
}

static const scmi_msg_handler_t handler_table[] = {
	[SCMI_PROTOCOL_VERSION] = report_version,
	[SCMI_PROTOCOL_ATTRIBUTES] = report_attributes,
	[SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] = report_message_attributes,
	[SCMI_CLOCK_ATTRIBUTES] = scmi_clock_attributes,
	[SCMI_CLOCK_DESCRIBE_RATES] = scmi_clock_describe_rates,
	[SCMI_CLOCK_RATE_SET] = scmi_clock_rate_set,
	[SCMI_CLOCK_RATE_GET] = scmi_clock_rate_get,
	[SCMI_CLOCK_CONFIG_SET] = scmi_clock_config_set,
};

static const size_t payload_size_table[] = {
	[SCMI_PROTOCOL_VERSION] = 0,
	[SCMI_PROTOCOL_ATTRIBUTES] = 0,
	[SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
		sizeof(struct scmi_protocol_message_attributes_a2p),
	[SCMI_CLOCK_ATTRIBUTES] = sizeof(struct scmi_clock_attributes_a2p),
	[SCMI_CLOCK_DESCRIBE_RATES] =
		sizeof(struct scmi_clock_describe_rates_a2p),
	[SCMI_CLOCK_RATE_SET] = sizeof(struct scmi_clock_rate_set_a2p),
	[SCMI_CLOCK_RATE_GET] = sizeof(struct scmi_clock_rate_get_a2p),
	[SCMI_CLOCK_CONFIG_SET] = sizeof(struct scmi_clock_config_set_a2p),
};

CASSERT(ARRAY_SIZE(handler_table) == ARRAY_SIZE(payload_size_table),
	assert_clock_protocol_message_id_tables_are_consistent);

static bool message_id_is_supported(unsigned int message_id)
{
	return scmi_get_msg_handler(message_id, handler_table,
				    ARRAY_SIZE(handler_table));
}

void scmi_clock_message_handler(struct scmi_msg *msg)
{
	assert(msg->protocol_id == SCMI_PROTOCOL_ID_CLOCK);

	scmi_call_msg_handler(msg, handler_table, payload_size_table,
			      ARRAY_SIZE(handler_table));
}
