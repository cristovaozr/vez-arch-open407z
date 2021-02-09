/**
 * @author Cristóvão Zuppardo Rufino <cristovaozr@gmail.com>
 * @version 0.1
 *
 * @copyright Copyright Cristóvão Zuppardo Rufino (c) 2020
 * Please see LICENCE file to information regarding licensing
 */

#include "include/device/cpu.h"

#include "include/errors.h"
#include "ulibc/include/utils.h"

#include "stm32f4xx.h"

#include <stdint.h>
#include <stddef.h>

#define UID_SIZE_IN_BYTES 12 // UID is 96 bits long

static int32_t stm32f4xx_get_uuid(const struct cpu * const cpu, void * const uuid, uint32_t size)
{
    int32_t ret = E_SUCCESS;
    const uint32_t *stm_uuid = (const uint32_t *)UID_BASE;
    uint32_t * const uuuid = (uint32_t * const)uuid;
    if (size < UID_SIZE_IN_BYTES) {
        ret = E_INVALID_PARAMETER;
        goto exit;
    }

    uuuid[0] = stm_uuid[0];
    uuuid[1] = stm_uuid[1];
    uuuid[2] = stm_uuid[2];

    exit:
    return ret;
}

static int32_t stm32f4xx_get_rtc_timestamp(const struct cpu * const cpu, uint32_t * const timestamp)
{
    return E_UNIMPEMENTED;
}

static int32_t stm32f4xx_get_clock_in_hz(const struct cpu * const cpu, uint32_t * const clock)
{
    *clock = SystemCoreClock;
    return E_SUCCESS;
}

static int32_t stm32f4xx_reset(const struct cpu * cpu)
{
    NVIC_SystemReset();
    return E_SUCCESS; // Theoretically it'll never reach here
}

const struct cpu stm32f4xx_cpu = {
    .get_uuid = stm32f4xx_get_uuid,
    .get_rtc_timestamp = stm32f4xx_get_rtc_timestamp,
    .get_clock_in_hz = stm32f4xx_get_clock_in_hz,
    .reset = stm32f4xx_reset
};