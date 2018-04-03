################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/gpio_ext.c \
../source/main.c 

OBJS += \
./source/gpio_ext.o \
./source/main.o 

C_DEPS += \
./source/gpio_ext.d \
./source/main.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -O3 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -flto -Wall -Wextra  -g3 -D"CPU_MK20DN512VLK10" -I../startup -I../board -I../utilities -I../freertos/Source/portable/GCC/ARM_CM3 -I../source -I../CMSIS -I../drivers -I"/home/toradex/prjs/apalis_tk1/freertos-toradex-v9/CMSIS/Include" -I"/home/toradex/prjs/apalis_tk1/freertos-toradex-v9/CMSIS/Driver/Include" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


