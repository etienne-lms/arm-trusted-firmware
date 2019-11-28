/*
 * Copyright (c) 2019, STMicroelectronics
 * Copyright (c) 2019, Linaro Limited
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdbool.h>
#include <stdint.h>

/* Currently allow full access by non-secure to platform clock services */
bool stm32mp_nsec_can_access_clock(unsigned long clock_id)
{
	return true;
}

/* Currently allow full access by non-secure to platform reset services */
bool stm32mp_nsec_can_access_reset(unsigned int reset_id)
{
	return true;
}
