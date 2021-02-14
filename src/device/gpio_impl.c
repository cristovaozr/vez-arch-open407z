/**
 * @author Cristóvão Zuppardo Rufino <cristovaozr@gmail.com>
 * @version 0.1
 *
 * @copyright Copyright Cristóvão Zuppardo Rufino (c) 2020-2021
 * Please see LICENCE file to information regarding licensing
 */

#include "include/device/gpio.h"

#include "include/errors.h"

#include <stdint.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"

struct gpio_priv {
    LL_GPIO_InitTypeDef config;
    uint32_t            ahb1_grp1_periph;
    GPIO_TypeDef        *gpio;
    uint32_t            default_value;
};

static const struct gpio_priv led_priv = {
    .config = {
        .Pin = LL_GPIO_PIN_6,
        .Mode = LL_GPIO_MODE_OUTPUT,
        .Speed = LL_GPIO_SPEED_FREQ_LOW,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
    },
    .ahb1_grp1_periph = LL_AHB1_GRP1_PERIPH_GPIOF,
    .gpio = GPIOF,
};

static int32_t stm32f4xx_gpio_init(const struct gpio_device * const gpio)
{
    const struct gpio_priv *priv = (const struct gpio_priv *)gpio->priv;
    LL_AHB1_GRP1_EnableClock(priv->ahb1_grp1_periph);
    LL_GPIO_Init(priv->gpio, (LL_GPIO_InitTypeDef *)&priv->config);

    if (priv->default_value == GPIO_HIGH) LL_GPIO_SetOutputPin(priv->gpio, priv->config.Pin);
    else                                  LL_GPIO_ResetOutputPin(priv->gpio, priv->config.Pin);

    return E_SUCCESS;
}

static void stm32f4xx_gpio_write(const struct gpio_device * const gpio, int32_t value)
{
    const struct gpio_priv *priv = (const struct gpio_priv *)gpio->priv;
    if (value)  LL_GPIO_SetOutputPin(priv->gpio, priv->config.Pin);
    else        LL_GPIO_ResetOutputPin(priv->gpio, priv->config.Pin);
}

static int32_t stm32f4xx_gpio_read(const struct gpio_device * const gpio)
{
    const struct gpio_priv *priv = (const struct gpio_priv *)gpio->priv;
    return LL_GPIO_IsInputPinSet(priv->gpio, priv->config.Pin);
}

static void stm32f4xx_gpio_toggle(const struct gpio_device * const gpio)
{
    const struct gpio_priv *priv = (const struct gpio_priv *)gpio->priv;
    LL_GPIO_TogglePin(priv->gpio, priv->config.Pin);
}

static const struct gpio_operations gpio_ops = {
    .gpio_init = stm32f4xx_gpio_init,
    .gpio_write_op = stm32f4xx_gpio_write,
    .gpio_read_op = stm32f4xx_gpio_read,
    .gpio_toggle_op = stm32f4xx_gpio_toggle
};

const struct gpio_device led_gpio = {
    .ops = &gpio_ops,
    .priv = &led_priv,
};