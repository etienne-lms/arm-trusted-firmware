/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019, Linaro Limited
 */
#ifndef SCMI_SERVER_RESET_DOMAIN_H
#define SCMI_SERVER_RESET_DOMAIN_H

#include <stdbool.h>
#include <stdint.h>

#include <lib/utils_def.h>
#include <lib/libc/cdefs.h>

#include "common.h"

#define SCMI_PROTOCOL_VERSION_RESET_DOMAIN	0x10000

#define SCMI_RESET_STATE_ARCH			(1 << 31)
#define SCMI_RESET_STATE_IMPL			(0 << 31)

/*
 * Identifiers of the SCMI Reset Domain Management Protocol commands
 */
enum scmi_reset_domain_command_id {
    SCMI_RESET_DOMAIN_ATTRIBUTES = 0x03,
    SCMI_RESET_DOMAIN_REQUEST = 0x04,
    SCMI_RESET_DOMAIN_NOTIFY = 0x05,
};

/*
 * Identifiers of the SCMI Reset Domain Management Protocol responses
 */
enum scmi_reset_domain_response_id {
    SCMI_RESET_ISSUED   = 0x00,
    SCMI_RESET_COMPLETE = 0x04,
};

/*
 * PROTOCOL_ATTRIBUTES
 */

#define SCMI_RESET_DOMAIN_COUNT_MASK		GENMASK_32(15, 0)

struct __packed scmi_reset_domain_protocol_attributes_p2a {
    int32_t status;
    uint32_t attributes;
};

/* Value for scmi_reset_domain_attributes_p2a:flags */
#define SCMI_RESET_DOMAIN_ATTR_ASYNC		BIT(31)
#define SCMI_RESET_DOMAIN_ATTR_NOTIF		BIT(30)

/* Value for scmi_reset_domain_attributes_p2a:latency */
#define SCMI_RESET_DOMAIN_ATTR_UNK_LAT		0x7fffffff
#define SCMI_RESET_DOMAIN_ATTR_MAX_LAT		0x7ffffffe

/* Macro for scmi_reset_domain_attributes_p2a:name */
#define SCMI_RESET_DOMAIN_ATTR_NAME_SZ		16

struct __packed scmi_reset_domain_attributes_a2p {
    uint32_t domain_id;
};

struct __packed scmi_reset_domain_attributes_p2a {
    int32_t status;
    uint32_t flags;
    uint32_t latency;
    uint8_t name[SCMI_RESET_DOMAIN_ATTR_NAME_SZ];
};

/*
 * RESET
 */

/* Values for scmi_reset_domain_request_a2p:flags */
#define SCMI_RESET_DOMAIN_ASYNC		(1 << 2)
#define SCMI_RESET_DOMAIN_EXPLICIT	(1 << 1)
#define SCMI_RESET_DOMAIN_AUTO		(1 << 0)

struct __packed scmi_reset_domain_request_a2p {
    uint32_t domain_id;
    uint32_t flags;
    uint32_t reset_state;
};

struct __packed scmi_reset_domain_request_p2a {
    int32_t status;
};

/*
 * RESET_NOTIFY
 */

/* Values for scmi_reset_notify_p2a:flags */
#define SCMI_RESET_DOMAIN_DO_NOTIFY	(1 << 0)

struct __packed scmi_reset_domain_notify_a2p {
    uint32_t domain_id;
    uint32_t notify_enable;
};

struct __packed scmi_reset_domain_notify_p2a {
    int32_t status;
};

/*
 * RESET_COMPLETE
 */

struct __packed scmi_reset_domain_complete_p2a {
    int32_t status;
    uint32_t domain_id;
};

/*
 * RESET_ISSUED
 */

struct __packed scmi_reset_domain_issued_p2a {
    uint32_t domain_id;
    uint32_t reset_state;
};

struct scmi_msg;

void scmi_reset_domain_message_handler(struct scmi_msg *msg);

#endif /* SCMI_SERVER_RESET_DOMAIN_H */
