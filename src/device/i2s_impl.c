/**
 * @author Cristóvão Zuppardo Rufino <cristovaozr@gmail.com>
 * @version 0.1
 *
 * @copyright Copyright Cristóvão Zuppardo Rufino (c) 2021
 * Please see LICENCE file to information regarding licensing
 */

#include "include/device/i2s.h"

#include "include/errors.h"

#include <stdint.h>
#include <stddef.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_spi.h"

struct i2s_priv {
    SPI_TypeDef *i2s;
};

static int32_t stm32f4xx_i2s2_init(const struct i2s_device * const i2s)
{
    int32_t ret = E_SUCCESS;

    const LL_GPIO_InitTypeDef gpio_set_1 = {
        .Pin = LL_GPIO_PIN_3 | LL_GPIO_PIN_6,
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO,
        .Speed = LL_GPIO_SPEED_FREQ_LOW,
        .Alternate = LL_GPIO_AF_5
    };

    const LL_GPIO_InitTypeDef gpio_set_2 = {
        .Pin = LL_GPIO_PIN_9 | LL_GPIO_PIN_10,
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO,
        .Speed = LL_GPIO_SPEED_FREQ_LOW,
        .Alternate = LL_GPIO_AF_5
    };

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_GPIO_Init(GPIOC, (LL_GPIO_InitTypeDef *)&gpio_set_1);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_GPIO_Init(GPIOB, (LL_GPIO_InitTypeDef *)&gpio_set_2);

    const LL_I2S_InitTypeDef config = {
        .Mode = LL_I2S_MODE_MASTER_TX,
        .Standard = LL_I2S_STANDARD_PHILIPS,
        .DataFormat = LL_I2S_DATAFORMAT_16B,
        .MCLKOutput = LL_I2S_MCLK_OUTPUT_ENABLE,
        .AudioFreq = LL_I2S_AUDIOFREQ_8K,
        .ClockPolarity = LL_I2S_POLARITY_LOW
    };

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
    if (LL_I2S_Init(SPI2, (LL_I2S_InitTypeDef *)&config) == ERROR) {
        ret = E_HARDWARE_CONFIG_FAILED;
        goto exit;
    }
    LL_I2S_Enable(SPI2);

    exit:
    return ret;
}

static int32_t stm32f4xx_i2s3_init(const struct i2s_device * const i2s)
{
    int32_t ret = E_SUCCESS;

    const LL_GPIO_InitTypeDef gpio_set_1 = {
        .Pin = LL_GPIO_PIN_7 | LL_GPIO_PIN_10,
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO,
        .Speed = LL_GPIO_SPEED_FREQ_LOW,
        .Alternate = LL_GPIO_AF_6
    };

    const LL_GPIO_InitTypeDef gpio_set_2 = {
        .Pin = LL_GPIO_PIN_5,
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO,
        .Speed = LL_GPIO_SPEED_FREQ_LOW,
        .Alternate = LL_GPIO_AF_6
    };

    const LL_GPIO_InitTypeDef gpio_set_3 = {
        .Pin = LL_GPIO_PIN_4,
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO,
        .Speed = LL_GPIO_SPEED_FREQ_LOW,
        .Alternate = LL_GPIO_AF_6
    };

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_GPIO_Init(GPIOC, (LL_GPIO_InitTypeDef *)&gpio_set_1);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_GPIO_Init(GPIOB, (LL_GPIO_InitTypeDef *)&gpio_set_2);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_GPIO_Init(GPIOA, (LL_GPIO_InitTypeDef *)&gpio_set_3);

    const LL_I2S_InitTypeDef config = {
        .Mode = LL_I2S_MODE_MASTER_TX,
        .Standard = LL_I2S_STANDARD_PHILIPS,
        .DataFormat = LL_I2S_DATAFORMAT_16B,
        .MCLKOutput = LL_I2S_MCLK_OUTPUT_ENABLE,
        .AudioFreq = LL_I2S_AUDIOFREQ_8K,
        .ClockPolarity = LL_I2S_POLARITY_LOW
    };

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
    if (LL_I2S_Init(SPI3, (LL_I2S_InitTypeDef *)&config) == ERROR) {
        ret = E_HARDWARE_CONFIG_FAILED;
        goto exit;
    }
    LL_I2S_Enable(SPI3);

    exit:
    return ret;
}

static int32_t stm32f4xx_i2s_write(const struct i2s_device * const i2s, uint16_t l_ch, uint16_t r_ch)
{
    const struct i2s_priv *priv = (const struct i2s_priv *)i2s->priv;

    while (LL_I2S_IsActiveFlag_TXE(priv->i2s) == 0);
    LL_I2S_TransmitData16(priv->i2s, l_ch);
    while (LL_I2S_IsActiveFlag_TXE(priv->i2s) == 0);
    LL_I2S_TransmitData16(priv->i2s, r_ch);

    return (sizeof(uint16_t) + sizeof(uint16_t));
}

static const struct i2s_operations i2s2_ops = {
    .i2s_init = stm32f4xx_i2s2_init,
    .i2s_write_op = stm32f4xx_i2s_write
};

static const struct i2s_priv i2s2_priv = {
    .i2s = SPI2
};

const struct i2s_device i2s2 = {
    .i2s_ops = &i2s2_ops,
    .priv = &i2s2_priv
};

static const struct i2s_operations i2s3_ops = {
    .i2s_init = stm32f4xx_i2s3_init,
    .i2s_write_op = stm32f4xx_i2s_write
};

static const struct i2s_priv i2s3_priv = {
    .i2s = SPI3
};

const struct i2s_device i2s3 = {
    .i2s_ops = &i2s3_ops,
    .priv = &i2s3_priv
};
