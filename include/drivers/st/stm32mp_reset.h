/*
 * Copyright (c) 2018-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_RESET_H
#define STM32MP_RESET_H

#include <stdint.h>

/*
 * Assert target reset, if @to_us non null, wait until reset is asserted
 *
 * @reset_id: Reset controller ID
 * @to_us: Timeout in microsecond, or 0 if not waiting
 * Return 0 on success and -ETIMEDOUT if waiting and timeout expired
 */
int stm32mp_reset_assert_to(uint32_t reset_id, unsigned int to_us);

/*
 * Set reset assert control to reset assertion state and return
 *
 * @reset_id: Reset controller ID
 */
static inline void stm32mp_reset_assert(uint32_t reset_id)
{
	(void)stm32mp_reset_assert_to(reset_id, 0);
}

/*
 * Deassert target reset, if @to_us non null, wait until reset is deasserted
 *
 * @reset_id: Reset controller ID
 * @to_us: Timeout in microsecond, or 0 if not waiting
 * Return 0 on success and -ETIMEDOUT if waiting and timeout expired
 */
int stm32mp_reset_deassert_to(uint32_t reset_id, unsigned int to_us);

/*
 * Set reset assert control to reset deassertion state and return
 *
 * @reset_id: Reset controller ID
 */
static inline void stm32mp_reset_deassert(uint32_t reset_id)
{
	(void)stm32mp_reset_deassert_to(reset_id, 0);
}

#endif /* STM32MP_RESET_H */
