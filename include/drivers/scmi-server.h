/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019, Linaro Limited
 */

#ifndef SCMI_SERVER_H
#define SCMI_SERVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Minimum size expected for SMT based shared memory message buffers */
#define SMT_BUF_SLOT_SIZE	128

/* A channel abstract a communication path between agent and server */
struct scmi_server_channel;

/*
 * struct scmi_server_channel - Shared message buffer
 *
 * @shm_addr: Address of the shared memory for the SCMI channel
 * @shm_size: Byte size of the shared memory for the SCMI channel
 * @busy: True when channel is busy, flase when channel is free
 */
struct scmi_server_channel {
	uintptr_t shm_addr;
	size_t shm_size;
	bool busy;
};

/*
 * Initialize SMT memory buffer, called by platform at init for each
 * agent channel using the SMT header format.
 *
 * @agent_id: SCMI agent ID the SMT belongs to
 */
void scmi_smt_init_agent_channel(unsigned int agent_id);

/*
 * Process SMT formatted message for a agent in a fastcall SMC context.
 * Called by platform on SMC entry. When returning, output message is
 * available in shared memory for agent to read the response.
 *
 * @agent_id: SCMI agent ID the SMT belongs to
 */
void scmi_smt_fastcall_smc_entry(unsigned int agent_id);

/*
 * Process SMT formatted message for a agent in a standard SMC context
 * Called by platform on SMC entry. When returning, output message is
 * avaiable in shared memory for agent to read the response.
 *
 * @agent_id: SCMI agent ID the SMT belongs to
 */
void scmi_smt_stdcall_smc_entry(unsigned int agent_id);

/* Platform callback functions */

/*
 * Return the SCMI channel related to an agent
 * @agent_id: SCMI agent ID
 */
struct scmi_server_channel *plat_scmi_get_channel(unsigned int agent_id);

/*
 * Return how many SCMI protocols supported by the platform
 * According to the SCMI specification, this function does not target
 * a specific agent ID and shall return all platform known capabilities.
 */
size_t plat_scmi_protocol_count(void);

/*
 * Get the count and list of SCMI protocols (but base) supported for an agent
 *
 * @agent_id: SCMI agent ID
 * Return a pointer to a null terminated array supported protocol IDs.
 */
const uint8_t *plat_scmi_protocol_list(unsigned int agent_id);

/* Get the name of the SCMI vendor for the platform */
const char *plat_scmi_vendor_name(void);

/* Get the name of the SCMI sub-vendor for the platform */
const char *plat_scmi_sub_vendor_name(void);

/* Handlers for SCMI Clock protocol services */

/*
 * Return number of clock controllers for an agent
 * @agent_id: SCMI agent ID
 * Return number of clock controllers
 */
size_t plat_scmi_clock_count(unsigned int agent_id);

/*
 * Get clock controller string ID (aka name)
 * @agent_id: SCMI agent ID
 * @scmi_id: SCMI clock ID
 * Return pointer to name or NULL
 */
const char *plat_scmi_clock_get_name(unsigned int agent_id,
				     unsigned int scmi_id);

/*
 * Get clock rate in Hertz
 * @agent_id: SCMI agent ID
 * @scmi_id: SCMI clock ID
 * Return clock rate or 0
 */
unsigned long plat_scmi_clock_get_rate(unsigned int agent_id,
				       unsigned int scmi_id);

/*
 * Set clock rate in Hertz
 * @agent_id: SCMI agent ID
 * @scmi_id: SCMI clock ID
 * Return a compliant SCMI error code
 */
int32_t plat_scmi_clock_set_rate(unsigned int agent_id, unsigned int scmi_id,
			         unsigned long rate);

/*
 * Get clock state (enabled or disabled)
 * @agent_id: SCMI agent ID
 * @scmi_id: SCMI clock ID
 * Return 1 if clock is enabled, 0 otherwise
 */
int plat_scmi_clock_get_state(unsigned int agent_id, unsigned int scmi_id);

/*
 * Get clock state (enabled or disabled)
 * @agent_id: SCMI agent ID
 * @scmi_id: SCMI clock ID
 * @enable_not_disable: Enable clock if true, disable clock otherwise
 * Return a compliant SCMI error code
 */
int32_t plat_scmi_clock_set_state(unsigned int agent_id, unsigned int scmi_id,
				  bool enable_not_disable);

/* Handlers for SCMI Reset Domain protocol services */

/*
 * Return number of reset domains for the agent
 * @agent_id: SCMI agent ID
 * Return number of reset domains
 */
size_t plat_scmi_rd_count(unsigned int agent_id);

/*
 * Get reset domain string ID (aka name)
 * @agent_id: SCMI agent ID
 * @scmi_id: SCMI reset domain ID
 * Return pointer to name or NULL
 */
const char *plat_scmi_rd_get_name(unsigned int agent_id, unsigned int scmi_id);

/*
 * Perform a reset cycle on a target reset domain
 * @agent_id: SCMI agent ID
 * @scmi_id: SCMI reset domain ID
 * @state: Target reset state (see SCMI specification, 0 means context loss)
 * Return a compliant SCMI error code
 */
int32_t plat_scmi_rd_autonomous(unsigned int agent_id, unsigned int scmi_id,
				unsigned int state);

/*
 * Assert or deassert target reset domain
 * @agent_id: SCMI agent ID
 * @scmi_id: SCMI reset domain ID
 * @assert_not_deassert: Assert domain if true, otherwise deassert domain
 * Return a compliant SCMI error code
 */
int32_t plat_scmi_rd_set_state(unsigned int agent_id, unsigned int scmi_id,
			       bool assert_not_deassert);

#endif /* SCMI_SERVER_H */
