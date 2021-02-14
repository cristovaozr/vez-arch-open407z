/**
 * @author Cristóvão Zuppardo Rufino <cristovaozr@gmail.com>
 * @version 0.1
 *
 * @copyright Copyright Cristóvão Zuppardo Rufino (c) 2020-2021
 * Please see LICENCE file to information regarding licensing
 */

#include "include/hw_init.h"
#include "include/device/device.h"
#include "include/errors.h"

#include "stm32f4xx.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_utils.h"

int32_t hw_init_early_config(void)
{
    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    return E_SUCCESS;
}

int32_t hw_init(void)
{
    int32_t ret = E_SUCCESS;

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
    if (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_5) {
        ret = E_HARDWARE_CONFIG_FAILED;
        goto exit;
    }

    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    LL_RCC_HSE_Enable();

    /* Wait till HSE is ready */
    while(LL_RCC_HSE_IsReady() != 1);
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_4, 168, LL_RCC_PLLP_DIV_2);
    LL_RCC_PLLI2S_ConfigDomain_I2S(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLI2SM_DIV_4, 192, LL_RCC_PLLI2SR_DIV_2);
    LL_RCC_PLL_Enable();

    /* Wait till PLL is ready */
    while(LL_RCC_PLL_IsReady() != 1);
    LL_RCC_PLLI2S_Enable();

    // /* Wait till PLL is ready */
    while(LL_RCC_PLLI2S_IsReady() != 1);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Wait till System clock is ready */
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);
    LL_SetSystemCoreClock(168000000);

    // LL_RCC_SetI2SClockSource(LL_RCC_I2S1_CLKSOURCE_PLLI2S);

    exit:
    return ret;
}

extern const struct usart_device usart2;
extern const struct gpio_device led_gpio;
extern const struct i2c_device i2c1;
extern const struct i2s_device i2s2;
extern const struct i2s_device i2s3;

int32_t hw_init_late_config(void)
{
    int32_t ret;
    if ((ret = device_init(&led_gpio)) != E_SUCCESS) goto exit;
    if ((ret = device_init(&usart2)) != E_SUCCESS) goto exit;
    if ((ret = device_init(&i2c1)) != E_SUCCESS) goto exit;
    if ((ret = device_init(&i2s3)) != E_SUCCESS) goto exit;

    exit:
    return ret;
}
