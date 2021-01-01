/**
 * @author Cristóvão Zuppardo Rufino <cristovaozr@gmail.com>
 * @version 0.1
 *
 * @copyright Copyright Cristóvão Zuppardo Rufino (c) 2020
 * Please see LICENCE file to information regarding licensing
 */

#ifndef ARCH_OPEN407Z_EXPORTED_H_
#define ARCH_OPEN407Z_EXPORTED_H_

#include "include/device/gpio.h"
#include "include/device/usart.h"
#include "include/device/i2c.h"

// Available at gpio_impl.c
extern const struct gpio_device led_gpio;

// Available at usart_impl.c
extern const struct usart_device usart2;

// Available at i2c_impl.c
extern const struct i2c_device i2c1;

#endif // ARCH_OPEN407Z_EXPORTED_H_