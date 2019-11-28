/*
 * Copyright (c) 2017-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_SHARED_RESOURCES_H
#define STM32MP_SHARED_RESOURCES_H

#include <stdbool.h>

#include <common/debug.h>
#include <drivers/st/etzpc.h>

enum stm32mp_shres;

#if STM32MP_SHARED_RESOURCES
/* Return true only if non-secure is allowed to manipulate clock controller */
bool stm32mp_nsec_can_access_clock(unsigned long clock_id);

/* Return true only if non-secure is allowed to manipulate reset controller */
bool stm32mp_nsec_can_access_reset(unsigned int reset_id);
#else
static inline bool stm32mp_nsec_can_access_clock(unsigned long clock_id)
{
	return true;
}
static inline bool stm32mp_nsec_can_access_reset(unsigned int reset_id);
{
	return true;
}
#endif

#endif /* STM32MP_SHARED_RESOURCES_H */
