// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, Linaro Limited
 */
#include <cdefs.h>
#include <string.h>

#include <drivers/st/scmi-msg.h>
#include <drivers/st/scmi.h>
#include <lib/utils.h>
#include <lib/utils_def.h>

#include "common.h"

static bool message_id_is_supported(unsigned int message_id);

#pragma weak plat_scmi_voltd_count
#pragma weak plat_scmi_voltd_get_name
#pragma weak plat_scmi_voltd_levels_array
#pragma weak plat_scmi_voltd_levels_by_step
#pragma weak plat_scmi_voltd_get_level
#pragma weak plat_scmi_voltd_set_level
#pragma weak plat_scmi_voltd_get_config
#pragma weak plat_scmi_voltd_set_config

size_t plat_scmi_voltd_count(unsigned int agent_id __unused)
{
	return 0;
}

const char *plat_scmi_voltd_get_name(unsigned int agent_id __unused,
				     unsigned int scmi_id __unused)
{
	return NULL;
}

int32_t plat_scmi_voltd_levels_array(unsigned int agent_id __unused,
				     unsigned int scmi_id __unused,
				     size_t start_index __unused,
				     long *levels __unused,
				     size_t *nb_elts __unused)
{
	return SCMI_NOT_SUPPORTED;
}

int32_t plat_scmi_voltd_levels_by_step(unsigned int agent_id __unused,
				       unsigned int scmi_id __unused,
				       long *steps __unused)
{
	return SCMI_NOT_SUPPORTED;
}

long plat_scmi_voltd_get_level(unsigned int agent_id __unused,
			       unsigned int scmi_id __unused)
{
	return 0;
}

int32_t plat_scmi_voltd_set_level(unsigned int agent_id __unused,
				  unsigned int scmi_id __unused,
				  long microvolt __unused)
{
	return SCMI_NOT_SUPPORTED;
}

int32_t plat_scmi_voltd_get_config(unsigned int agent_id __unused,
				   unsigned int scmi_id __unused,
				   uint32_t *config __unused)
{
	return SCMI_NOT_SUPPORTED;
}

int32_t plat_scmi_voltd_set_config(unsigned int agent_id __unused,
				   unsigned int scmi_id __unused,
				   uint32_t config __unused)
{
	return SCMI_NOT_SUPPORTED;
}

static void report_version(struct scmi_msg *msg)
{
	struct scmi_protocol_version_p2a out_args = {
		.status = SCMI_SUCCESS,
		.version = SCMI_PROTOCOL_VERSION_VOLTAGE_DOMAIN,
	};

	if (msg->in_size != 0) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	scmi_write_response(msg, &out_args, sizeof(out_args));
}

static void report_attributes(struct scmi_msg *msg)
{
	size_t domain_count = plat_scmi_voltd_count(msg->agent_id);
	struct scmi_protocol_attributes_p2a out_args = {
		.status = SCMI_SUCCESS,
		.attributes = domain_count,
	};

	assert((domain_count & ~SCMI_VOLTAGE_DOMAIN_COUNT_MASK) == 0U);

	if (msg->in_size != 0U) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	scmi_write_response(msg, &out_args, sizeof(out_args));
}

static void report_message_attributes(struct scmi_msg *msg)
{
	struct scmi_protocol_message_attributes_a2p *in_args = (void *)msg->in;
	struct scmi_protocol_message_attributes_p2a out_args = {
		.status = SCMI_SUCCESS,
		/* For this protocol, attributes shall be zero */
		.attributes = 0U,
	};

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	if (!message_id_is_supported(in_args->message_id)) {
		scmi_status_response(msg, SCMI_NOT_FOUND);
		return;
	}

	scmi_write_response(msg, &out_args, sizeof(out_args));
}

static void scmi_voltd_domain_attributes(struct scmi_msg *msg)
{
	const struct scmi_voltd_attributes_a2p *in_args = (void *)msg->in;
	struct scmi_voltd_attributes_p2a out_args = {
		.status = SCMI_SUCCESS,
	};
	const char *name = NULL;
	unsigned int domain_id = 0U;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	domain_id = SPECULATION_SAFE_VALUE(in_args->domain_id);

	if (domain_id >= plat_scmi_voltd_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	name = plat_scmi_voltd_get_name(msg->agent_id, domain_id);
	if (name == NULL) {
		scmi_status_response(msg, SCMI_NOT_FOUND);
		return;
	}

	COPY_NAME_IDENTIFIER(out_args.name, name);

	scmi_write_response(msg, &out_args, sizeof(out_args));
}

#define LEVELS_BY_ARRAY(_nb_rates, _rem_rates) \
	SCMI_VOLTAGE_DOMAIN_LEVELS_FLAGS((_nb_rates), \
					 SCMI_VOLTD_LEVELS_FORMAT_LIST, \
					 (_rem_rates))

/* List levels array by small chunks fitting in SCMI message max payload size */
#define LEVELS_ARRAY_SIZE_MAX_2	\
	((SCMI_PLAYLOAD_MAX - sizeof(struct scmi_voltd_describe_levels_p2a)) / \
	 sizeof(int32_t))

#define LEVELS_ARRAY_SIZE_MAX	10U

#define LEVELS_BY_STEP \
	SCMI_VOLTAGE_DOMAIN_LEVELS_FLAGS(3, SCMI_VOLTD_LEVELS_FORMAT_RANGE, 0)

static void scmi_voltd_describe_levels(struct scmi_msg *msg)
{
	const struct scmi_voltd_describe_levels_a2p *in_args = (void *)msg->in;
	size_t nb_levels = 0U;
	unsigned int domain_id = 0U;
	int32_t status = SCMI_GENERIC_ERROR;

	if (msg->in_size != sizeof(*in_args)) {
		status = SCMI_PROTOCOL_ERROR;
		goto out;
	}

	domain_id = SPECULATION_SAFE_VALUE(in_args->domain_id);

	if (domain_id >= plat_scmi_voltd_count(msg->agent_id)) {
		status = SCMI_INVALID_PARAMETERS;
		goto out;
	}

	/* Platform may support array rate description */
	status = plat_scmi_voltd_levels_array(msg->agent_id, domain_id, 0, NULL,
					      &nb_levels);
	if (status == SCMI_SUCCESS) {
		/* Use the stack to get the returned portion of levels array */
		long plat_levels[LEVELS_ARRAY_SIZE_MAX];
		size_t ret_nb = 0U;
		size_t rem_nb = 0U;

		if (in_args->level_index >= nb_levels) {
			status = SCMI_INVALID_PARAMETERS;
			goto out;
		}

		ret_nb = MIN(ARRAY_SIZE(plat_levels),
			     nb_levels - in_args->level_index);
		rem_nb = nb_levels - in_args->level_index - ret_nb;

		status =  plat_scmi_voltd_levels_array(msg->agent_id, domain_id,
						       in_args->level_index,
						       plat_levels,
						       &ret_nb);
		if (status == SCMI_SUCCESS) {
			struct scmi_voltd_describe_levels_p2a out_args = {
				.flags = LEVELS_BY_ARRAY(ret_nb, rem_nb),
				.status = SCMI_SUCCESS,
			};
			int32_t *voltage = NULL;
			size_t i = 0U;

			/* By construction these values are 32bit aligned */
			voltage = (int32_t *)(uintptr_t)(msg->out +
							 sizeof(out_args));

			for (i = 0; i < ret_nb; i++) {
				voltage[i] = plat_levels[i];
			}

			memcpy(msg->out, &out_args, sizeof(out_args));

			msg->out_size_out = sizeof(out_args) +
					    ret_nb * sizeof(uint32_t);
		}
	} else if (status == SCMI_NOT_SUPPORTED) {
		long triplet[3] = { 0, 0, 0 };

		/* Platform may support min/max/step triplet description */
		status =  plat_scmi_voltd_levels_by_step(msg->agent_id,
							 domain_id, triplet);
		if (status == SCMI_SUCCESS) {
			struct scmi_voltd_describe_levels_p2a out_args = {
				.flags = LEVELS_BY_STEP,
				.status = SCMI_SUCCESS,
			};
			int32_t *voltage = NULL;

			/* By construction these values are 32bit aligned */
			voltage = (int32_t *)(uintptr_t)(msg->out +
							 sizeof(out_args));
			voltage[0] = triplet[0];
			voltage[1] = triplet[1];
			voltage[2] = triplet[2];

			memcpy(msg->out, &out_args, sizeof(out_args));

			msg->out_size_out = sizeof(out_args) +
					    3U * sizeof(int32_t);
		}
	}

out:
	if (status != SCMI_SUCCESS) {
		scmi_status_response(msg, status);
	}
}

static void scmi_voltd_config_set(struct scmi_msg *msg)
{
	const struct scmi_voltd_config_set_a2p *in_args = (void *)msg->in;
	unsigned int domain_id = 0U;
	unsigned long config = 0U;
	int32_t status = SCMI_GENERIC_ERROR;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	domain_id = SPECULATION_SAFE_VALUE(in_args->domain_id);

	if (domain_id >= plat_scmi_voltd_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	config = in_args->config & SCMI_VOLTAGE_DOMAIN_CONFIG_MASK;

	status = plat_scmi_voltd_set_config(msg->agent_id, domain_id, config);

	scmi_status_response(msg, status);
}

static void scmi_voltd_config_get(struct scmi_msg *msg)
{
	const struct scmi_voltd_config_get_a2p *in_args = (void *)msg->in;
	struct scmi_voltd_config_get_p2a out_args = { };
	unsigned int domain_id = 0U;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	domain_id = SPECULATION_SAFE_VALUE(in_args->domain_id);

	if (domain_id >= plat_scmi_voltd_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	if (plat_scmi_voltd_get_config(msg->agent_id, domain_id,
				       &out_args.config) != 0) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	scmi_write_response(msg, &out_args, sizeof(out_args));
}

static void scmi_voltd_level_set(struct scmi_msg *msg)
{
	const struct scmi_voltd_level_set_a2p *in_args = (void *)msg->in;
	int32_t status = SCMI_GENERIC_ERROR;
	unsigned int domain_id = 0U;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	domain_id = SPECULATION_SAFE_VALUE(in_args->domain_id);

	if (domain_id >= plat_scmi_voltd_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	status = plat_scmi_voltd_set_level(msg->agent_id, domain_id,
					   in_args->voltage_level);

	scmi_status_response(msg, status);
}

static void scmi_voltd_level_get(struct scmi_msg *msg)
{
	const struct scmi_voltd_level_get_a2p *in_args = (void *)msg->in;
	struct scmi_voltd_level_get_p2a out_args = {
		.status = SCMI_SUCCESS,
	};
	unsigned int domain_id = 0U;

	if (msg->in_size != sizeof(*in_args)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	domain_id = SPECULATION_SAFE_VALUE(in_args->domain_id);

	if (domain_id >= plat_scmi_voltd_count(msg->agent_id)) {
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}

	out_args.voltage_level = plat_scmi_voltd_get_level(msg->agent_id,
							   domain_id);

	scmi_write_response(msg, &out_args, sizeof(out_args));
}

static const scmi_msg_handler_t handler_array[] = {
	[SCMI_PROTOCOL_VERSION] = report_version,
	[SCMI_PROTOCOL_ATTRIBUTES] = report_attributes,
	[SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] = report_message_attributes,
	[SCMI_VOLTAGE_DOMAIN_ATTRIBUTES] = scmi_voltd_domain_attributes,
	[SCMI_VOLTAGE_DESCRIBE_LEVELS] = scmi_voltd_describe_levels,
	[SCMI_VOLTAGE_CONFIG_SET] = scmi_voltd_config_set,
	[SCMI_VOLTAGE_CONFIG_GET] = scmi_voltd_config_get,
	[SCMI_VOLTAGE_LEVEL_SET] = scmi_voltd_level_set,
	[SCMI_VOLTAGE_LEVEL_GET] = scmi_voltd_level_get,
};

static bool message_id_is_supported(size_t id)
{
	return id < ARRAY_SIZE(handler_array) && handler_array[id];
}

scmi_msg_handler_t scmi_msg_get_voltd_handler(struct scmi_msg *msg)
{
	unsigned int message_id = SPECULATION_SAFE_VALUE(msg->message_id);

	if (message_id >= ARRAY_SIZE(handler_array)) {
		VERBOSE("Voltage domain handle not found %u\n", msg->message_id);
		return NULL;
	}

	return handler_array[message_id];
}
