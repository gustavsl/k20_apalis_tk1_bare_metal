# Bare metal firmware for Toradex Apalis TK1

The [Toradex Apalis TK1](https://www.toradex.com/computer-on-modules/apalis-arm-family/nvidia-tegra-k1) is a Computer-on-Module based on the NVIDIA Tegra K1 SoC.

The Tegra K1 SoC features a quad-core Cortex-A15 CPU and a NVIDIA Kepler GPU with 192 CUDA cores.

The Apalis TK1 Computer-on-Module features a [NXP Kinetis K20 (MK20DN512VMC10)](https://www.nxp.com/products/processors-and-microcontrollers/arm-based-processors-and-mcus/kinetis-cortex-m-mcus/k-seriesperformancem4/k2x-usb/kinetis-k20-100-mhz-full-speed-usb-mixed-signal-integration-microcontrollers-based-on-arm-cortex-m4-core:K20_100) companion MCU connected to the Tegra K1 SoC to provide additional peripherals (ADC, CAN, Touch screen controller).

The [default firmware on the K20 MCU](http://git.toradex.com/cgit/freertos-toradex.git/log/?h=apalis-tk1-k20-freertos-v9) is based on FreeRTOS and acts as a intermediate layer between the K20 hardware and the Linux drivers, gathering peripheral data on assigned tasks and sending the information via SPI to the Tegra K1.

A Linux Kernel driver abstracts the incoming SPI data from the K20 MCU and makes use of default Linux Kernel drivers (such as [Industrial I/O](https://wiki.analog.com/software/linux/docs/iio/iio) to provide seamsless access to the peripherals.

## Why a bare metal firmware?

The FreeRTOS approach is not feasible when trying to acquire high-speed data from peripherals such as the ADC. 

An interesting solution for these particular cases is to give up the Linux abstraction features provided by the RTOS firmware and implement an interrupt-driven bare metal application to capture data, store it e.g. in a buffer and send it via the SPI bus.

This project is based on the repository for the default Toradex K20 FreeRTOS firmware linked above. It is based on the Kinetis SDK V2.0.
