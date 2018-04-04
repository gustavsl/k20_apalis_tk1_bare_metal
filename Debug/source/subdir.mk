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
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -O3 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -flto -Wall -Wextra  -g3 -D"CPU_MK20DN512VLK10" -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/CMSIS/Driver/Include -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/CMSIS/Include -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/startup -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/board -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/utilities -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/source -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/CMSIS -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/drivers -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


