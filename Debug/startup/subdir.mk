################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../startup/startup_stm32.s 

OBJS += \
./startup/startup_stm32.o 

S_DEPS += \
./startup/startup_stm32.d 


# Each subdirectory must supply rules for building sources it contributes
startup/startup_stm32.o: ../startup/startup_stm32.s
	arm-none-eabi-gcc -mcpu=cortex-m3 -g3 -c -I"/home/nuno/STM32CubeIDE/workspace_1.4.0/sistr_freertos/inc" -I"/home/nuno/STM32CubeIDE/workspace_1.4.0/sistr_freertos/StdPeriph_Driver/inc" -I"/home/nuno/STM32CubeIDE/workspace_1.4.0/sistr_freertos/CMSIS/core" -I"/home/nuno/STM32CubeIDE/workspace_1.4.0/sistr_freertos/CMSIS/device" -I"/home/nuno/STM32CubeIDE/workspace_1.4.0/sistr_freertos/FreeRTOS/Source/include" -I"/home/nuno/STM32CubeIDE/workspace_1.4.0/sistr_freertos/FreeRTOS/Source/portable/GCC/ARM_CM3" -x assembler-with-cpp -MMD -MP -MF"startup/startup_stm32.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"

