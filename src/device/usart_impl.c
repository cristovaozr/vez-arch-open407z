/**
 * @author Cristóvão Zuppardo Rufino <cristovaozr@gmail.com>
 * @version 0.1
 *
 * @copyright Copyright Cristóvão Zuppardo Rufino (c) 2020-2021
 * Please see LICENCE file to information regarding licensing
 */

#include "include/device/usart.h"

#include <stdint.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"

#include "include/errors.h"
#include "include/device/pool_op.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Number of USARTs available
#define AVAILABLE_USARTS    1

struct usart_priv_rtos {
    QueueHandle_t tx_queue;
    QueueHandle_t rx_queue;
    SemaphoreHandle_t mutex;
};

struct usart_priv {
    uint32_t irqn;
    USART_TypeDef *usart;
    int index;
};

static struct usart_priv_rtos priv_rtos[AVAILABLE_USARTS];

static int32_t stm32f4xx_usart2_init(const struct usart_device * const usart)
{
    const struct usart_priv *priv = (const struct usart_priv *)usart->priv;

    const LL_USART_InitTypeDef usart2_config = {
        .BaudRate = 115200,
        .DataWidth = LL_USART_DATAWIDTH_8B,
        .StopBits = LL_USART_STOPBITS_1,
        .Parity = LL_USART_PARITY_NONE,
        .TransferDirection = LL_USART_DIRECTION_TX_RX,
        .HardwareFlowControl = LL_USART_HWCONTROL_NONE,
        .OverSampling = LL_USART_OVERSAMPLING_16
    };

    const LL_GPIO_InitTypeDef usart2_gpio_config = {
        .Pin = LL_GPIO_PIN_2|LL_GPIO_PIN_3,
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO,
        .Alternate = LL_GPIO_AF_7,
    };

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_GPIO_Init(GPIOA, (LL_GPIO_InitTypeDef *)&usart2_gpio_config);

    /* USART2 interrupt Init */
    NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_EnableIRQ(USART2_IRQn);

    LL_USART_Init(USART2, (LL_USART_InitTypeDef *)&usart2_config);
    LL_USART_ConfigAsyncMode(USART2);
    LL_USART_Enable(USART2);

    priv_rtos[priv->index].tx_queue = xQueueCreate(64, sizeof(uint8_t));
    priv_rtos[priv->index].rx_queue = xQueueCreate(64, sizeof(uint8_t));
    priv_rtos[priv->index].mutex = xSemaphoreCreateMutex();

    LL_USART_EnableIT_RXNE(USART2);

    return E_SUCCESS;
}

static int32_t stm32f4xx_usart_write(const struct usart_device * const usart, const void *data, uint32_t size, uint32_t timeout)
{
    const struct usart_priv *priv = (const struct usart_priv *)usart->priv;
    const uint8_t *udata = (const uint8_t *)data;
    uint32_t i;
    int32_t ret;

    if (priv_rtos[priv->index].tx_queue == NULL || priv_rtos[priv->index].mutex == NULL) {
        ret = E_NOT_INITIALIZED;
        goto exit;
    }

    xSemaphoreTake(priv_rtos[priv->index].mutex, portMAX_DELAY);
    for(i = 0; i < size; i++) {
        if (xQueueSend(priv_rtos[priv->index].tx_queue, &udata[i], timeout) == pdFAIL) {
            break; // Timed-out. Must stop and return now. A timeout is not an error!
        } else {
            LL_USART_EnableIT_TXE(priv->usart);
        }
    }
    ret = i;

    exit:
    xSemaphoreGive(priv_rtos[priv->index].mutex);
    return (int32_t)ret;
}

static int32_t stm32f4xx_usart_read(const struct usart_device * const usart, void *data, uint32_t size, uint32_t timeout)
{
const struct usart_priv *priv = (const struct usart_priv *)usart->priv;
    uint8_t *udata = (uint8_t *)data;
    uint32_t i;
    int32_t ret;

    if (priv_rtos[priv->index].rx_queue == NULL || priv_rtos[priv->index].mutex == NULL) {
        ret = E_NOT_INITIALIZED;
        goto exit;
    }

    xSemaphoreTake(priv_rtos[priv->index].mutex, portMAX_DELAY);
    for(i = 0; i < size; i++) {
        if (xQueueReceive(priv_rtos[priv->index].rx_queue, &udata[i], timeout) == pdFAIL) {
            ret = E_TIMEOUT;
            goto exit;
        }
    }
    ret = i;

    exit:
    xSemaphoreGive(priv_rtos[priv->index].mutex);
    return ret;
}

int32_t stm32f4xx_usart_poll(const struct usart_device * const usart, enum poll_op op, void *answer)
{
    const struct usart_priv *priv = (const struct usart_priv *)usart->priv;
    int32_t ret;

    switch (op) {
    case POLL_RX_QUEUE_SIZE: {
        if (priv_rtos[priv->index].rx_queue == NULL) {
            ret = E_NOT_INITIALIZED;
            goto exit;
        }
        *((uint32_t *)answer) = uxQueueMessagesWaiting(priv_rtos[priv->index].rx_queue);
        ret = E_SUCCESS;
        break;
    }
    
    default:
        ret = E_POLLOP_INVALID;
        goto exit;
    }

exit:
    return ret;
}

static const struct usart_priv usart2_priv = {
    .irqn = USART2_IRQn,
    .usart = USART2,
    .index = 0
};

static const struct usart_operations usart2_ops = {
    .usart_init = stm32f4xx_usart2_init,
    .usart_write_op = stm32f4xx_usart_write,
    .usart_read_op = stm32f4xx_usart_read,
    .usart_poll_op = stm32f4xx_usart_poll
};

const struct usart_device usart2 = {
    .ops = &usart2_ops,
    .priv = &usart2_priv
};

static void usart_irq_handle(const struct usart_device * const usart)
{
    BaseType_t context_switch;
    const struct usart_priv *priv = (const struct usart_priv *)usart->priv;

    if (LL_USART_IsActiveFlag_TXE(priv->usart)) {
        while (LL_USART_IsActiveFlag_TXE(priv->usart)) {
            uint8_t byte;
            if (xQueueReceiveFromISR(priv_rtos[priv->index].tx_queue, &byte, &context_switch) == pdFAIL) {
                LL_USART_DisableIT_TXE(priv->usart);
                break; // Queue empty and can safelly disable TXE
            }
            LL_USART_TransmitData8(priv->usart, byte);
        }
    }

    if (LL_USART_IsActiveFlag_RXNE(priv->usart)) {
        while(LL_USART_IsActiveFlag_RXNE(priv->usart)) {
            uint8_t byte = LL_USART_ReceiveData8(priv->usart);
            xQueueSendFromISR(priv_rtos[priv->index].rx_queue, &byte, &context_switch);
            // Could not enqueue anymore because queue is full
        }
    }

    portYIELD_FROM_ISR(context_switch);
}

void USART2_IRQHandler(void)
{
    usart_irq_handle(&usart2);
}