/**
 * @author Cristóvão Zuppardo Rufino <cristovaozr@gmail.com>
 * @version 0.1
 *
 * @copyright Copyright Cristóvão Zuppardo Rufino (c) 2021
 * Please see LICENCE file to information regarding licensing
 */

#include "include/device/i2c.h"

#include "include/errors.h"

#include <stdint.h>
#include <stddef.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_i2c.h"

#define READ_FROM(x)    ((x) << 1 | 0x01)
#define WRITE_TO(x)     ((x) << 1 | 0x00)

struct i2c_priv {
    I2C_TypeDef *i2c;
};

static int32_t stm32f4xx_i2c1_init(const struct i2c_device * const i2c)
{
    int32_t ret = E_SUCCESS;

    const LL_GPIO_InitTypeDef GPIO_InitStruct = {
        .Pin = LL_GPIO_PIN_6|LL_GPIO_PIN_7,
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .OutputType = LL_GPIO_OUTPUT_OPENDRAIN,
        .Pull = LL_GPIO_PULL_UP,
        .Alternate = LL_GPIO_AF_4
    };

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_GPIO_Init(GPIOB, (LL_GPIO_InitTypeDef *)&GPIO_InitStruct);

    const LL_I2C_InitTypeDef I2C_InitStruct = {
        .PeripheralMode = LL_I2C_MODE_I2C,
        .ClockSpeed = 100000,
        .DutyCycle = LL_I2C_DUTYCYCLE_2,
        .OwnAddress1 = 0,
        .TypeAcknowledge = LL_I2C_ACK,
        .OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT
    };
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
    LL_I2C_DisableOwnAddress2(I2C1);
    LL_I2C_DisableGeneralCall(I2C1);
    LL_I2C_EnableClockStretching(I2C1);
    LL_I2C_SetOwnAddress2(I2C1, 0);
    if (LL_I2C_Init(I2C1, (LL_I2C_InitTypeDef *)&I2C_InitStruct) != SUCCESS) ret = E_HARDWARE_CONFIG_FAILED;

    return ret;
}

static int32_t stm32f4xx_i2c_write(const struct i2c_device * const i2c, const struct i2c_transaction *transaction, uint32_t timeout)
{
    int32_t ret;
    uint32_t t;

    if (transaction == NULL || i2c == NULL) {
        ret = E_INVALID_PARAMETER;
        goto exit;
    }

    const struct i2c_priv *priv = (const struct i2c_priv *)i2c->priv;
    const uint8_t *uwrite = (const uint8_t *)transaction->write_data;

    // Generate start
    LL_I2C_GenerateStartCondition(priv->i2c);
    t = timeout;
    while (LL_I2C_IsActiveFlag_SB(priv->i2c) == 0 && --t);
    if (t == 0) { ret = E_TIMEOUT; goto exit; }

    // Send address
    LL_I2C_TransmitData8(priv->i2c, WRITE_TO(transaction->i2c_device_addr));
    t = timeout;
    while (LL_I2C_IsActiveFlag_ADDR(priv->i2c) == 0 && --t);
    if (t == 0) { ret = E_TIMEOUT; goto exit; }
    LL_I2C_ClearFlag_ADDR(priv->i2c);

    // Send register address
    LL_I2C_TransmitData8(priv->i2c, transaction->i2c_device_reg);
    t = timeout;
    while (LL_I2C_IsActiveFlag_TXE(priv->i2c) == 0 && --t);
    if (t == 0) { ret = E_TIMEOUT; goto exit; }

    for (ret = 0; ret < transaction->transaction_size; ret++) {
        LL_I2C_TransmitData8(priv->i2c, uwrite[ret]);
        t = timeout;
        while (LL_I2C_IsActiveFlag_TXE(priv->i2c) == 0 && --t);
        if (t == 0) { ret = E_TIMEOUT; goto exit; }
    }

    t = timeout;
    while (LL_I2C_IsActiveFlag_BTF(priv->i2c) == 0 && --t);
    if (t == 0) { ret = E_TIMEOUT; goto exit; }
    LL_I2C_GenerateStopCondition(priv->i2c);

    exit:
    return ret;
}

static int32_t stm32f4xx_i2c_read(const struct i2c_device * const i2c, const struct i2c_transaction *transaction, uint32_t timeout)
{
    int32_t ret;
    uint32_t t;

    if (transaction == NULL || i2c == NULL) {
        ret = E_INVALID_PARAMETER;
        goto exit;
    }

    const struct i2c_priv *priv = (const struct i2c_priv *)i2c->priv;
    uint8_t *uread = (uint8_t *)transaction->read_data;

    // Generate start bit
    LL_I2C_GenerateStartCondition(priv->i2c);
    t = timeout;
    while (LL_I2C_IsActiveFlag_SB(priv->i2c) == 0 && --t);
    if (t == 0) { ret = E_TIMEOUT; goto exit; }

    // Sends device address
    LL_I2C_TransmitData8(priv->i2c, WRITE_TO(transaction->i2c_device_addr));
    t = timeout;
    while (LL_I2C_IsActiveFlag_ADDR(priv->i2c) == 0 && --t);
    if (t == 0) { ret = E_TIMEOUT; goto exit; }
    LL_I2C_ClearFlag_ADDR(priv->i2c);

    // Sends device register to read from
    LL_I2C_TransmitData8(priv->i2c, transaction->i2c_device_reg);

    // Repeated start condition
    LL_I2C_GenerateStartCondition(priv->i2c);
    t = timeout;
    while (LL_I2C_IsActiveFlag_SB(priv->i2c) == 0 && --t);
    if (t == 0) { ret = E_TIMEOUT; goto exit; }

    // Sends device address
    LL_I2C_TransmitData8(priv->i2c, READ_FROM(transaction->i2c_device_addr));
    t = timeout;
    while (LL_I2C_IsActiveFlag_ADDR(priv->i2c) == 0 && --t);
    if (t == 0) { ret = E_TIMEOUT; goto exit; }

    if (transaction->transaction_size == 1) {
        LL_I2C_ClearFlag_ADDR(priv->i2c);
        LL_I2C_AcknowledgeNextData(priv->i2c, LL_I2C_NACK);
        LL_I2C_GenerateStopCondition(priv->i2c);

        uread[0] = LL_I2C_ReceiveData8(priv->i2c);
        ret = 1;

    } else if (transaction->transaction_size == 2) {
        LL_I2C_AcknowledgeNextData(priv->i2c, LL_I2C_NACK);
        LL_I2C_EnableBitPOS(priv->i2c);
        LL_I2C_ClearFlag_ADDR(priv->i2c);

        t = timeout;
        while (LL_I2C_IsActiveFlag_BTF(priv->i2c) == 0 && --t);
        if (t == 0) { ret = E_TIMEOUT; goto exit; }
        LL_I2C_GenerateStopCondition(priv->i2c);

        uread[0] = LL_I2C_ReceiveData8(priv->i2c);
        uread[1] = LL_I2C_ReceiveData8(priv->i2c);

    } else if (transaction->transaction_size > 2) {
        ret = E_UNIMPEMENTED;
        goto exit;

    } else {
        ret = E_INVALID_PARAMETER;
        goto exit;
    }

    exit:
    return ret;
}

static const struct i2c_operations i2c1_ops = {
    .i2c_init = stm32f4xx_i2c1_init,
    .i2c_write_op = stm32f4xx_i2c_write,
    .i2c_read_op = stm32f4xx_i2c_read,
};

static const struct i2c_priv i2c1_priv = {
    .i2c = I2C1
};

const struct i2c_device i2c1 = {
    .i2c_ops = &i2c1_ops,
    .priv = &i2c1_priv
};
