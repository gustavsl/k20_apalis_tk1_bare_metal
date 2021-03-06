/*
 * Copyright (c) 2013 - 2016, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * This is template for main module created by New Kinetis SDK 2.x Project Wizard. Enjoy!
 **/

#include <string.h>

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_adc16.h"
#include "fsl_uart.h"

// Defines

#define USE_BOTH_ADCS // Uncomment if you want to use ADC0 and ADC1 simultaneously

#define ADC_BASE0 ADC0
#define ADC16_CHANNEL_GROUP 0U
#define ADC16_USER_CHANNEL 8U /* PTB0, ADC0_SE8 */

#define ADC_BASE1 ADC1

#define ADC0_IRQn ADC0_IRQn
#define ADC0_IRQ_HANDLER_FUNC ADC0_IRQHandler

#define ADC1_IRQn ADC1_IRQn
#define ADC1_IRQ_HANDLER_FUNC ADC1_IRQHandler

// Globals
volatile bool g_Adc16ConversionDoneFlag0 = false;
volatile bool g_Adc16ConversionDoneFlag1 = false;

volatile uint32_t g_Adc16ConversionValue0;
volatile uint32_t g_Adc16ConversionValue1;

volatile uint32_t g_Adc16InterruptCounter0;
volatile uint32_t g_Adc16InterruptCounter1;

void ADC0_IRQ_HANDLER_FUNC(void)
{
	g_Adc16ConversionDoneFlag0 = true;
	/* Read conversion result to clear the conversion completed flag. */
	g_Adc16ConversionValue0 = ADC16_GetChannelConversionValue(ADC_BASE0,
			ADC16_CHANNEL_GROUP);
	g_Adc16InterruptCounter0++;
}

void ADC1_IRQ_HANDLER_FUNC(void)
{
	g_Adc16ConversionDoneFlag1 = true;
	/* Read conversion result to clear the conversion completed flag. */
	g_Adc16ConversionValue1 = ADC16_GetChannelConversionValue(ADC_BASE1,
			ADC16_CHANNEL_GROUP);
	g_Adc16InterruptCounter1++;
}
/*!
 * @brief Application entry point.
 */
int main(void)
{

	adc16_config_t adc16ConfigStruct;
	adc16_channel_config_t adc16ChannelConfigStruct;
	/* Init board hardware. */
	BOARD_InitPins();
	BOARD_BootClockRUN();
	BOARD_InitDebugConsole();
	EnableIRQ(ADC0_IRQn);
#ifdef USE_BOTH_ADCS
	EnableIRQ(ADC1_IRQn);
#endif

	ADC16_GetDefaultConfig(&adc16ConfigStruct);
	ADC16_Init(ADC_BASE0, &adc16ConfigStruct);
#ifdef USE_BOTH_ADCS
	ADC16_Init(ADC_BASE1, &adc16ConfigStruct);
#endif

	ADC16_EnableHardwareTrigger(ADC_BASE0, false); /* Make sure the software trigger is used. */
#ifdef USE_BOTH_ADCS
	ADC16_EnableHardwareTrigger(ADC_BASE1, false); /* Make sure the software trigger is used. */
#endif

	adc16ChannelConfigStruct.channelNumber = ADC16_USER_CHANNEL;
	adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = true; /* Enable the interrupt. */

	g_Adc16InterruptCounter0 = 0U;
	g_Adc16InterruptCounter1 = 0U;

	while(1) {
		g_Adc16ConversionDoneFlag0 = false;
#ifdef USE_BOTH_ADCS

		g_Adc16ConversionDoneFlag1 = false;
#endif
		/*
		 When in software trigger mode, each conversion would be launched once calling the "ADC16_ChannelConfigure()"
		 function, which works like writing a conversion command and executing it. For another channel's conversion,
		 just to change the "channelNumber" field in channel configuration structure, and call the function
		 "ADC16_ChannelConfigure()"" again.
		 Also, the "enableInterruptOnConversionCompleted" inside the channel configuration structure is a parameter for
		 the conversion command. It takes affect just for the current conversion. If the interrupt is still required
		 for the following conversion, it is necessary to assert the "enableInterruptOnConversionCompleted" every time
		 for each command.
		 */
		ADC16_SetChannelConfig(ADC_BASE0, ADC16_CHANNEL_GROUP,
				&adc16ChannelConfigStruct);
#ifdef USE_BOTH_ADCS
		ADC16_SetChannelConfig(ADC_BASE1, ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
#endif
		while(!g_Adc16ConversionDoneFlag0) {
		}
		PRINTF("%d\r\n", g_Adc16ConversionValue0);
#ifdef USE_BOTH_ADCS
		while (!g_Adc16ConversionDoneFlag1)
		{
		}
		PRINTF("%d\r\n", g_Adc16ConversionValue1);

#endif
//      Change ADC channel
//        adc16ChannelConfigStruct.channelNumber = 11U;
	}
}

