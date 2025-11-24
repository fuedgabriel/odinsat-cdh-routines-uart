################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/drivers/BMI088.c \
../Core/Src/drivers/adcs.c \
../Core/Src/drivers/can_driver.c \
../Core/Src/drivers/can_protocol.c \
../Core/Src/drivers/uart_protocol.c 

OBJS += \
./Core/Src/drivers/BMI088.o \
./Core/Src/drivers/adcs.o \
./Core/Src/drivers/can_driver.o \
./Core/Src/drivers/can_protocol.o \
./Core/Src/drivers/uart_protocol.o 

C_DEPS += \
./Core/Src/drivers/BMI088.d \
./Core/Src/drivers/adcs.d \
./Core/Src/drivers/can_driver.d \
./Core/Src/drivers/can_protocol.d \
./Core/Src/drivers/uart_protocol.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/drivers/%.o Core/Src/drivers/%.su Core/Src/drivers/%.cyclo: ../Core/Src/drivers/%.c Core/Src/drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-drivers

clean-Core-2f-Src-2f-drivers:
	-$(RM) ./Core/Src/drivers/BMI088.cyclo ./Core/Src/drivers/BMI088.d ./Core/Src/drivers/BMI088.o ./Core/Src/drivers/BMI088.su ./Core/Src/drivers/adcs.cyclo ./Core/Src/drivers/adcs.d ./Core/Src/drivers/adcs.o ./Core/Src/drivers/adcs.su ./Core/Src/drivers/can_driver.cyclo ./Core/Src/drivers/can_driver.d ./Core/Src/drivers/can_driver.o ./Core/Src/drivers/can_driver.su ./Core/Src/drivers/can_protocol.cyclo ./Core/Src/drivers/can_protocol.d ./Core/Src/drivers/can_protocol.o ./Core/Src/drivers/can_protocol.su ./Core/Src/drivers/uart_protocol.cyclo ./Core/Src/drivers/uart_protocol.d ./Core/Src/drivers/uart_protocol.o ./Core/Src/drivers/uart_protocol.su

.PHONY: clean-Core-2f-Src-2f-drivers

