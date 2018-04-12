# Bare metal firmware for Toradex Apalis TK1

The [Toradex Apalis TK1](https://www.toradex.com/computer-on-modules/apalis-arm-family/nvidia-tegra-k1) is a Computer-on-Module based on the NVIDIA Tegra K1 SoC.

The Tegra K1 SoC features a quad-core Cortex-A15 CPU and a NVIDIA Kepler GPU with 192 CUDA cores.

The Apalis TK1 Computer-on-Module features a [NXP Kinetis K20 (MK20DN512VMC10)](https://www.nxp.com/products/processors-and-microcontrollers/arm-based-processors-and-mcus/kinetis-cortex-m-mcus/k-seriesperformancem4/k2x-usb/kinetis-k20-100-mhz-full-speed-usb-mixed-signal-integration-microcontrollers-based-on-arm-cortex-m4-core:K20_100) companion MCU connected to the Tegra K1 SoC to provide additional peripherals (ADC, CAN, Touch screen controller).

The [default firmware on the K20 MCU](http://git.toradex.com/cgit/freertos-toradex.git/log/?h=apalis-tk1-k20-freertos-v9) is based on FreeRTOS and acts as an intermediate layer between the K20 hardware and the Linux drivers, gathering peripheral data on assigned tasks and sending the information via SPI to the Tegra K1.

A Linux Kernel driver abstracts the incoming SPI data from the K20 MCU and makes use of default Linux Kernel drivers (such as [Industrial I/O](https://wiki.analog.com/software/linux/docs/iio/iio) to provide seamsless access to the peripherals.

## Why, though?

A quick-and-dirty solution for these particular cases is to give up the Linux abstraction features provided by the RTOS firmware and implement an interrupt-driven bare metal application to capture data, store it e.g. in a buffer and send it via the SPI bus.

This project is based on the repository for the default Toradex K20 FreeRTOS firmware linked above. It is based on the Kinetis SDK V2.0.

## How the hell does this work?

Basically, for each ADC peripheral, we're using two DMA channels. Also, a single timer is used to trigger the ADC conversions.
One DMA channel is responsible for writing the ADC result to memory. Using the DMA Link feature, once the memory is written, another DMA channel changes the ADC channel for the next reading.

For a more in-depth explanation of how this works (for a single ADC peripheral), read [this](https://community.nxp.com/docs/DOC-335320).

We've added the possibility of reading two ADC peripherals (ADC0 and ADC1). Basically, the application flow is "doubled". Since the ADCs and the DMAs are separate peripherals, they all run "in parallel" and use the same timer.
