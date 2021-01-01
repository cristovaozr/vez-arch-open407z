##
# @author Cristóvão Zuppardo Rufino <cristovaozr@gmail.com>
# @version 0.1
#
# @copyright Copyright Cristóvão Zuppardo Rufino (c) 2020
# Please see LICENCE file to information regarding licensing

# Configuration flags for STM32F4xx

R_PATH = arch/$(ARCH)

CPU = -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard

ARCH_MCU = $(CPU) -mthumb

# ARCH specific ASM sources
ARCH_ASM_SOURCES += \
	$(R_PATH)/src/startup_stm32f407xx.s

ARCH_AS_INCLUDES +=

ARCH_C_DEFS += \
	-DUSE_FULL_LL_DRIVER \
	-DSTM32F407xx \
	-DHSE_VALUE=8000000

ARCH_C_INCLUDES += \
	-I$(R_PATH) \
	-I$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Inc \
	-I$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy \
	-I$(R_PATH)/Drivers/CMSIS/Device/ST/STM32F4xx/Include \
	-I$(R_PATH)/Drivers/CMSIS/Include \
	-I$(R_PATH)/freertos/portable/GCC/ARM_CM4F

ARCH_LDSCRIPT = $(R_PATH)/STM32F407ZETx_FLASH.ld

ARCH_C_SOURCES += \
	$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_gpio.c \
	$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_adc.c \
	$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dma.c \
	$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rcc.c \
	$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_utils.c \
	$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_exti.c \
	$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_i2c.c \
	$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rtc.c \
	$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_spi.c \
	$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_pwr.c \
	$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usart.c \
	$(R_PATH)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_tim.c

ARCH_C_SOURCES += \
	$(R_PATH)/freertos/portable/GCC/ARM_CM4F/port.c

ARCH_C_SOURCES += \
	$(R_PATH)/src/stm32f4xx_it.c \
	$(R_PATH)/src/system_stm32f4xx.c \
	$(R_PATH)/src/hw_init.c \
	$(R_PATH)/src/device/gpio_impl.c \
	$(R_PATH)/src/device/usart_impl.c