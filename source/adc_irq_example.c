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
#define ADC16_BASE0 ADC0
#define ADC16_CHANNEL_GROUP 0U
#define ADC16_USER_CHANNEL 10U /* PTA7, ADC0_SE10 */

#define ADC16_BASE1 ADC1

#define ADC16_IRQn0 ADC0_IRQn
#define ADC16_IRQ0_HANDLER_FUNC ADC0_IRQHandler

#define ADC16_IRQn1 ADC1_IRQn
#define ADC16_IRQ1_HANDLER_FUNC ADC1_IRQHandler

// Globals
volatile bool g_Adc16ConversionDoneFlag = false;
volatile uint32_t g_Adc16ConversionValue;
volatile uint32_t g_Adc16InterruptCounter;

void ADC16_IRQ_HANDLER_FUNC(void)
{
    g_Adc16ConversionDoneFlag = true;
    /* Read conversion result to clear the conversion completed flag. */
    g_Adc16ConversionValue = ADC16_GetChannelConversionValue(ADC16_BASE0, ADC16_CHANNEL_GROUP);
    g_Adc16InterruptCounter++;
}
/*!
 * @brief Application entry point.
 */
int main(void) {

    adc16_config_t adc16ConfigStruct;
    adc16_channel_config_t adc16ChannelConfigStruct;
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    EnableIRQ(ADC16_IRQn0);

    PRINTF("\r\nADC16 interrupt Example.\r\n");

    ADC16_GetDefaultConfig(&adc16ConfigStruct);
    ADC16_Init(ADC16_BASE0, &adc16ConfigStruct);
    ADC16_EnableHardwareTrigger(ADC16_BASE0, false); /* Make sure the software trigger is used. */

    adc16ChannelConfigStruct.channelNumber = ADC16_USER_CHANNEL;
    adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = true; /* Enable the interrupt. */

    g_Adc16InterruptCounter = 0U;

    while (1)
    {
        GETCHAR();
        g_Adc16ConversionDoneFlag = false;
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
        ADC16_SetChannelConfig(ADC16_BASE0, ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
        while (!g_Adc16ConversionDoneFlag)
        {
        }
        PRINTF("ADC Value: %d\r\n", g_Adc16ConversionValue);
        PRINTF("ADC Interrupt Count: %d\r\n", g_Adc16InterruptCounter);
        PRINTF("ADC Channel: %d\r\n", adc16ChannelConfigStruct.channelNumber);

//      Change ADC channel
        adc16ChannelConfigStruct.channelNumber = 11U;
    }
}

