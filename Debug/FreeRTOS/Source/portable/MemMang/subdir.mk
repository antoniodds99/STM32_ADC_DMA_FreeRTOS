################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS/Source/portable/MemMang/heap_2.c 

OBJS += \
./FreeRTOS/Source/portable/MemMang/heap_2.o 

C_DEPS += \
./FreeRTOS/Source/portable/MemMang/heap_2.d 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/Source/portable/MemMang/heap_2.o: ../FreeRTOS/Source/portable/MemMang/heap_2.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DSTM32F1 -DSTM32F103RBTx -DSTM32 -DDEBUG -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD -c -I"/home/nuno/STM32CubeIDE/workspace_1.4.0/sistr_freertos/inc" -I"/home/nuno/STM32CubeIDE/workspace_1.4.0/sistr_freertos/StdPeriph_Driver/inc" -I"/home/nuno/STM32CubeIDE/workspace_1.4.0/sistr_freertos/CMSIS/core" -I"/home/nuno/STM32CubeIDE/workspace_1.4.0/sistr_freertos/CMSIS/device" -I"/home/nuno/STM32CubeIDE/workspace_1.4.0/sistr_freertos/FreeRTOS/Source/include" -I"/home/nuno/STM32CubeIDE/workspace_1.4.0/sistr_freertos/FreeRTOS/Source/portable/GCC/ARM_CM3" -O0 -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"FreeRTOS/Source/portable/MemMang/heap_2.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

