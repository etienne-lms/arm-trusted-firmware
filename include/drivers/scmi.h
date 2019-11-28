/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 */
#ifndef SCMI_SERVER_SCMI_H
#define SCMI_SERVER_SCMI_H

#define SCMI_PROTOCOL_ID_BASE			0x10
#define SCMI_PROTOCOL_ID_CLOCK			0x14
#define SCMI_PROTOCOL_ID_RESET_DOMAIN		0x16

/* SCMI error codes reported to agent through server-to-agent messages */
enum scmi_error {
    SCMI_SUCCESS            =  0,
    SCMI_NOT_SUPPORTED      = -1,
    SCMI_INVALID_PARAMETERS = -2,
    SCMI_DENIED             = -3,
    SCMI_NOT_FOUND          = -4,
    SCMI_OUT_OF_RANGE       = -5,
    SCMI_BUSY               = -6,
    SCMI_COMMS_ERROR        = -7,
    SCMI_GENERIC_ERROR      = -8,
    SCMI_HARDWARE_ERROR     = -9,
    SCMI_PROTOCOL_ERROR     = -10,
};

#endif /* SCMI_SERVER_SCMI_H */
