################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/board.c \
../board/clock_config.c \
../board/pin_mux.c 

OBJS += \
./board/board.o \
./board/clock_config.o \
./board/pin_mux.o 

C_DEPS += \
./board/board.d \
./board/clock_config.d \
./board/pin_mux.d 


# Each subdirectory must supply rules for building sources it contributes
board/%.o: ../board/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -O3 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -flto -Wall -Wextra  -g3 -D"CPU_MK20DN512VLK10" -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/CMSIS/Driver/Include -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/CMSIS/Include -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/startup -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/board -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/utilities -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/source -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/CMSIS -I/home/favero/workspace.kds/k20_apalis_tk1_bare_metal/drivers -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


