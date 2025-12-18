################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/Systemslayer/Error.c \
../Source/Systemslayer/HRT.c \
../Source/Systemslayer/PAL.c \
../Source/Systemslayer/Power.c \
../Source/Systemslayer/USBDev.c \
../Source/Systemslayer/USBi.c \
../Source/Systemslayer/dI2C.c 

OBJS += \
./Source/Systemslayer/Error.o \
./Source/Systemslayer/HRT.o \
./Source/Systemslayer/PAL.o \
./Source/Systemslayer/Power.o \
./Source/Systemslayer/USBDev.o \
./Source/Systemslayer/USBi.o \
./Source/Systemslayer/dI2C.o 

C_DEPS += \
./Source/Systemslayer/Error.d \
./Source/Systemslayer/HRT.d \
./Source/Systemslayer/PAL.d \
./Source/Systemslayer/Power.d \
./Source/Systemslayer/USBDev.d \
./Source/Systemslayer/USBi.d \
./Source/Systemslayer/dI2C.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Systemslayer/Error.o: ../Source/Systemslayer/Error.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32L4R5xx -DDEBUG -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Source/Application -I../Source/Middleware -I../Core/Src -I../Source/Systemslayer -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Source/Systemslayer/Error.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Source/Systemslayer/HRT.o: ../Source/Systemslayer/HRT.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32L4R5xx -DDEBUG -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Source/Application -I../Source/Middleware -I../Core/Src -I../Source/Systemslayer -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Source/Systemslayer/HRT.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Source/Systemslayer/PAL.o: ../Source/Systemslayer/PAL.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32L4R5xx -DDEBUG -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Source/Application -I../Source/Middleware -I../Core/Src -I../Source/Systemslayer -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Source/Systemslayer/PAL.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Source/Systemslayer/Power.o: ../Source/Systemslayer/Power.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32L4R5xx -DDEBUG -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Source/Application -I../Source/Middleware -I../Core/Src -I../Source/Systemslayer -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Source/Systemslayer/Power.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Source/Systemslayer/USBDev.o: ../Source/Systemslayer/USBDev.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32L4R5xx -DDEBUG -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Source/Application -I../Source/Middleware -I../Core/Src -I../Source/Systemslayer -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Source/Systemslayer/USBDev.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Source/Systemslayer/USBi.o: ../Source/Systemslayer/USBi.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32L4R5xx -DDEBUG -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Source/Application -I../Source/Middleware -I../Core/Src -I../Source/Systemslayer -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Source/Systemslayer/USBi.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Source/Systemslayer/dI2C.o: ../Source/Systemslayer/dI2C.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32L4R5xx -DDEBUG -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Source/Application -I../Source/Middleware -I../Core/Src -I../Source/Systemslayer -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Source/Systemslayer/dI2C.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

