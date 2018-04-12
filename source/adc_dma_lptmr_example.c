/*
 * Copyright (c) 2016, NXP Semiconductor, Inc.
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
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
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
 * @file    K64F_ADC_DMA_Demo.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"

#include "fsl_adc16.h"
#include "fsl_pit.h"
#include "fsl_dmamux.h"
#include "fsl_edma.h"
#include "fsl_uart.h"

#define CHANNELS         4u				/* Number of ADC channels*/
#define SAMPLES_PER_CH   10u
#define B_SIZE           CHANNELS*SAMPLES_PER_CH	/* Internal Buffer size */

#define VREFH_CH         29u				/*ADC Channels*/
#define VREFL_CH	 30u

#define DMAChannel_0	 0u				/*DMA Channels*/
#define DMAChannel_1	 1u
#define DMAChannel_2	 2u
#define DMAChannel_3	 3u

/* UART instance and clock */
#define DATA_UART UART3
#define DATA_UART_CLKSRC kCLOCK_BusClk
#define DATA_UART_CLK_FREQ CLOCK_GetFreq(kCLOCK_BusClk)

/* TX UART message*/
uint8_t txbuff[] = "abcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcdeabcde\r\n";

/* ADC Channels array */
uint8_t g_ADC0_mux[CHANNELS] = {VREFH_CH, 8, 12, 13};
uint8_t g_ADC1_mux[CHANNELS] = {VREFH_CH, 7, 9, 17};

/* Internal ADC buffers */
uint16_t g_ADC0_resultBuffer[B_SIZE] = {0};
uint16_t g_ADC1_resultBuffer[B_SIZE] = {0};

/* Global Done flags*/
volatile bool g_Adc0ConversionDoneFlag = false;
volatile bool g_Adc1ConversionDoneFlag = false;

volatile bool g_Transfer_Done_ch0 = false;
volatile bool g_Transfer_Done_ch1 = false;

volatile bool g_Transfer_Done_ch2 = false;
volatile bool g_Transfer_Done_ch3 = false;

/* edma handler for channels 0 and 1 */
edma_handle_t g_EDMA_Handle_0;
edma_handle_t g_EDMA_Handle_1;

/* edma handler for channels 2 and 3 */
edma_handle_t g_EDMA_Handle_2;
edma_handle_t g_EDMA_Handle_3;

void ADC0_IRQHandler(void)
{
	g_Adc0ConversionDoneFlag = true;
}

void ADC1_IRQHandler(void)
{
	g_Adc1ConversionDoneFlag = true;
}

void EDMA_Callback_0(edma_handle_t *handle, void *param, bool transferDone,
		uint32_t tcds)
{
	if(transferDone) {
		g_Transfer_Done_ch0 = true;
	}
}

void EDMA_Callback_1(edma_handle_t *handle, void *param, bool transferDone,
		uint32_t tcds)
{
	if(transferDone) {
		/* If next line is uncommented, it will start again after 12 transfers finished */
		EDMA_StartTransfer(&g_EDMA_Handle_1);
		g_Transfer_Done_ch1 = true;
	}
}

void EDMA_Callback_2(edma_handle_t *handle, void *param, bool transferDone,
		uint32_t tcds)
{
	if(transferDone) {
		g_Transfer_Done_ch2 = true;
	}
}

void EDMA_Callback_3(edma_handle_t *handle, void *param, bool transferDone,
		uint32_t tcds)
{
	if(transferDone) {
		/* If next line is uncommented, it will start again after 12 transfers finished */
		EDMA_StartTransfer(&g_EDMA_Handle_3);
		g_Transfer_Done_ch3 = true;
	}
}

/*
 * @brief   Application entry point.
 */
int main(void)
{
	uart_config_t config;

	/* Init board hardware. */
	BOARD_InitPins();
	BOARD_BootClockRUN();
	BOARD_InitDebugConsole();
  
	/****************************************
	 * UART Config and init
	 ****************************************/
	UART_GetDefaultConfig(&config);
	config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
	//config.baudRate_Bps = 9600U;
	config.enableTx = true;
	config.enableRx = true;

	UART_Init(DATA_UART, &config, DATA_UART_CLK_FREQ);
	//UART_WriteBlocking(DATA_UART, txbuff, sizeof(txbuff) - 1);
	
	/****************************************
	 * ADC0 Config
	 ****************************************/
	EnableIRQ(ADC0_IRQn);

	adc16_config_t adc16ConfigStruct;
	adc16_channel_config_t adc16ChannelConfigStruct;
	/*
	 * adc16ConfigStruct.referenceVoltageSource = kADC16_ReferenceVoltageSourceVref;
	 * adc16ConfigStruct.clockSource = kADC16_ClockSourceAsynchronousClock;
	 * adc16ConfigStruct.enableAsynchronousClock = true;
	 * adc16ConfigStruct.clockDivider = kADC16_ClockDivider8;
	 * adc16ConfigStruct.resolution = kADC16_ResolutionSE12Bit;
	 * adc16ConfigStruct.longSampleMode = kADC16_LongSampleDisabled;
	 * adc16ConfigStruct.enableHighSpeed = false;
	 * adc16ConfigStruct.enableLowPower = false;
	 * adc16ConfigStruct.enableContinuousConversion = false;
	 */
	ADC16_GetDefaultConfig(&adc16ConfigStruct);
	ADC16_Init( ADC0, &adc16ConfigStruct);

	ADC16_DoAutoCalibration( ADC0);

	ADC16_EnableHardwareTrigger( ADC0, true);
	ADC16_EnableDMA( ADC0, true);

	adc16ChannelConfigStruct.channelNumber = g_ADC0_mux[2];
	adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = true; /* Enable the interrupt. */
	adc16ChannelConfigStruct.enableDifferentialConversion = false;
	ADC16_SetChannelConfig( ADC0, 0, &adc16ChannelConfigStruct);

	/****************************************
	 * ADC1 Config
	 ****************************************/
	EnableIRQ(ADC1_IRQn);
	/*
	 * adc16ConfigStruct.referenceVoltageSource = kADC16_ReferenceVoltageSourceVref;
	 * adc16ConfigStruct.clockSource = kADC16_ClockSourceAsynchronousClock;
	 * adc16ConfigStruct.enableAsynchronousClock = true;
	 * adc16ConfigStruct.clockDivider = kADC16_ClockDivider8;
	 * adc16ConfigStruct.resolution = kADC16_ResolutionSE12Bit;
	 * adc16ConfigStruct.longSampleMode = kADC16_LongSampleDisabled;
	 * adc16ConfigStruct.enableHighSpeed = false;
	 * adc16ConfigStruct.enableLowPower = false;
	 * adc16ConfigStruct.enableContinuousConversion = false;
	 */
	ADC16_GetDefaultConfig(&adc16ConfigStruct);
	ADC16_Init( ADC1, &adc16ConfigStruct);

	ADC16_DoAutoCalibration( ADC1);

	ADC16_EnableHardwareTrigger( ADC1, true);
	ADC16_EnableDMA( ADC1, true);

	adc16ChannelConfigStruct.channelNumber = g_ADC1_mux[2];
	adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = true; /* Enable the interrupt. */
	adc16ChannelConfigStruct.enableDifferentialConversion = false;
	ADC16_SetChannelConfig( ADC1, 0, &adc16ChannelConfigStruct);

	/****************************************
	 * PIT for ADC HW trigger Config
	 ****************************************/
	pit_config_t pitConfig;

	PIT_GetDefaultConfig(&pitConfig);

	/* Initialize PIT */
	PIT_Init(PIT, &pitConfig);

	/* Set PIT period */
	PIT_SetTimerPeriod( PIT, kPIT_Chnl_0,
			USEC_TO_COUNT(2, CLOCK_GetFreq(kCLOCK_CoreSysClk)));

	/* Configure SIM for ADC hw trigger source selection */
	SIM->SOPT7 |= 0x8484U;

	/****************************************
	 * DMA Config
	 ****************************************/
	edma_transfer_config_t transferConfig_ch0;
	edma_transfer_config_t transferConfig_ch1;
	edma_transfer_config_t transferConfig_ch2;
	edma_transfer_config_t transferConfig_ch3;
	edma_config_t userConfig;

	/* Configure DMAMUX */
	DMAMUX_Init(DMAMUX0);

	DMAMUX_SetSource(DMAMUX0, DMAChannel_0, kDmaRequestMux0AlwaysOn60); /* Channel 0 Source 60: DMA always enabled */
	DMAMUX_EnableChannel(DMAMUX0, DMAChannel_0);

	DMAMUX_SetSource(DMAMUX0, DMAChannel_1, kDmaRequestMux0ADC0); /* Channel 1 Source 40: ADC COCO trigger */
	DMAMUX_EnableChannel(DMAMUX0, DMAChannel_1);

	DMAMUX_SetSource(DMAMUX0, DMAChannel_2, kDmaRequestMux0AlwaysOn60); /* Channel 0 Source 60: DMA always enabled */
	DMAMUX_EnableChannel(DMAMUX0, DMAChannel_2);

	DMAMUX_SetSource(DMAMUX0, DMAChannel_3, kDmaRequestMux0ADC1); /* Channel 1 Source 40: ADC COCO trigger */
	DMAMUX_EnableChannel(DMAMUX0, DMAChannel_3);

	/*
	 * Configure EDMA one shot transfer
	 * userConfig.enableRoundRobinArbitration = false;
	 * userConfig.enableHaltOnError = true;
	 * userConfig.enableContinuousLinkMode = false;
	 * userConfig.enableDebugMode = false;
	 */
	EDMA_GetDefaultConfig(&userConfig);
	userConfig.enableHaltOnError = false;

	EDMA_Init(DMA0, &userConfig);

	EDMA_CreateHandle(&g_EDMA_Handle_1, DMA0, DMAChannel_1);
	EDMA_SetCallback(&g_EDMA_Handle_1, EDMA_Callback_1, NULL);

	EDMA_PrepareTransfer(&transferConfig_ch1, /* Prepare TCD for CH1 */
			(uint32_t*)(ADC0->R), /* Source Address (ADC0_RA) */
			sizeof(uint16_t), /* Source width (2 bytes) */
			g_ADC0_resultBuffer, /* Destination Address (Internal buffer)*/
			sizeof(g_ADC0_resultBuffer[0]), /* Destination width (2 bytes) */
			sizeof(uint16_t), /* Bytes to transfer each minor loop (2 bytes) */
			B_SIZE * 2, /* Total of bytes to transfer (12*2 bytes) */
			kEDMA_PeripheralToMemory); /* From ADC to Memory */
	/* Push TCD for CH1 into hardware TCD Register */
	EDMA_SubmitTransfer(&g_EDMA_Handle_1, &transferConfig_ch1);

	/* Link channel 1 to channel 0
	 * when channel 1 transfer ADC data,
	 * then channel 0 will change ADC channel*/
	EDMA_SetChannelLink(DMA0, DMAChannel_1, kEDMA_MinorLink, DMAChannel_0);
	EDMA_SetChannelLink(DMA0, DMAChannel_1, kEDMA_MajorLink, DMAChannel_0);

	EDMA_CreateHandle(&g_EDMA_Handle_0, DMA0, DMAChannel_0);
	EDMA_SetCallback(&g_EDMA_Handle_0, EDMA_Callback_0, NULL);

	EDMA_PrepareTransfer(&transferConfig_ch0, /* Prepare TCD for CH0 */
			&g_ADC0_mux[0], /* Source Address (ADC channels array) */
			sizeof(g_ADC0_mux[0]), /* Source width (1 bytes) */
			(uint32_t*)(ADC0->SC1),/* Destination Address (ADC_SC1A_ADCH)*/
			sizeof(uint8_t), /* Destination width (1 bytes) */
			sizeof(uint8_t), /* Bytes to transfer each minor loop (1 bytes) */
			CHANNELS, /* Total of bytes to transfer (3*1 bytes) */
			kEDMA_MemoryToPeripheral);/* From ADC channels array to ADCH register */
	/* Push TCD for CH0 into hardware TCD Register */
	EDMA_SubmitTransfer(&g_EDMA_Handle_0, &transferConfig_ch0);

	/* If transfer will continue it will need to adjust last source and destination */
	DMA0->TCD[0].SLAST = -1 * CHANNELS;
	DMA0->TCD[1].DLAST_SGA = -2 * B_SIZE;

	EDMA_StartTransfer(&g_EDMA_Handle_1);



	EDMA_CreateHandle(&g_EDMA_Handle_3, DMA0, DMAChannel_3);
	EDMA_SetCallback(&g_EDMA_Handle_3, EDMA_Callback_3, NULL);

	EDMA_PrepareTransfer(&transferConfig_ch3, /* Prepare TCD for CH1 */
			(uint32_t*)(ADC1->R), /* Source Address (ADC0_RA) */
			sizeof(uint16_t), /* Source width (2 bytes) */
			g_ADC1_resultBuffer, /* Destination Address (Internal buffer)*/
			sizeof(g_ADC1_resultBuffer[0]), /* Destination width (2 bytes) */
			sizeof(uint16_t), /* Bytes to transfer each minor loop (2 bytes) */
			B_SIZE * 2, /* Total of bytes to transfer (12*2 bytes) */
			kEDMA_PeripheralToMemory); /* From ADC to Memory */
	/* Push TCD for CH1 into hardware TCD Register */
	EDMA_SubmitTransfer(&g_EDMA_Handle_3, &transferConfig_ch3);

	/* Link channel 1 to channel 0
	 * when channel 1 transfer ADC data,
	 * then channel 0 will change ADC channel*/
	EDMA_SetChannelLink(DMA0, DMAChannel_3, kEDMA_MinorLink, DMAChannel_2);
	EDMA_SetChannelLink(DMA0, DMAChannel_3, kEDMA_MajorLink, DMAChannel_2);

	EDMA_CreateHandle(&g_EDMA_Handle_2, DMA0, DMAChannel_2);
	EDMA_SetCallback(&g_EDMA_Handle_2, EDMA_Callback_2, NULL);

	EDMA_PrepareTransfer(&transferConfig_ch2, /* Prepare TCD for CH0 */
			&g_ADC1_mux[0], /* Source Address (ADC channels array) */
			sizeof(g_ADC1_mux[0]), /* Source width (1 bytes) */
			(uint32_t*)(ADC1->SC1),/* Destination Address (ADC_SC1A_ADCH)*/
			sizeof(uint8_t), /* Destination width (1 bytes) */
			sizeof(uint8_t), /* Bytes to transfer each minor loop (1 bytes) */
			CHANNELS, /* Total of bytes to transfer (3*1 bytes) */
			kEDMA_MemoryToPeripheral);/* From ADC channels array to ADCH register */
	/* Push TCD for CH0 into hardware TCD Register */
	EDMA_SubmitTransfer(&g_EDMA_Handle_2, &transferConfig_ch2);

	/* If transfer will continue it will need to adjust last source and destination */
	DMA0->TCD[2].SLAST = -1 * CHANNELS;
	DMA0->TCD[3].DLAST_SGA = -2 * B_SIZE;

	EDMA_StartTransfer(&g_EDMA_Handle_3);

	/* Start PIT */
	PIT_StartTimer(PIT, kPIT_Chnl_0);

	uint16_t i = 0;

	while(1) {
		if(g_Transfer_Done_ch0) {
//			PRINTF("%d: %d \t\t", i, g_ADC0_resultBuffer[i++]);
//			PRINTF("%d: %d \t\t", i, g_ADC0_resultBuffer[i++]);
//			PRINTF("%d: %d \t\t", i, g_ADC0_resultBuffer[i++]);
//			PRINTF("\r\n");
//
//			g_Transfer_Done_ch0 = false;
			if(g_Transfer_Done_ch1) {
				i = 0;
				PRINTF("Finished ADC0\r\n");

				//UART_WriteBlocking(DATA_UART, g_ADC0_resultBuffer, sizeof(g_ADC0_resultBuffer) - 1);

				for (i = 0; i < B_SIZE; i+2) {
					PRINTF("\r");
					PRINTF("%d \t\t", g_ADC0_resultBuffer[i++]);
					PRINTF("%d \t\t", g_ADC0_resultBuffer[i++]);
					PRINTF("%d \t\t", g_ADC0_resultBuffer[i++]);
					PRINTF("%d \t\t", g_ADC0_resultBuffer[i++]);										PRINTF("%d \t\t", g_ADC0_resultBuffer[i++]);
					PRINTF("\r\n");
				}
				g_Transfer_Done_ch1 = false;

			}
		}

		if(g_Transfer_Done_ch2) {

			if(g_Transfer_Done_ch3) {
				i = 0;
				PRINTF("\r\nFinished ADC1\r\n");

				//UART_WriteBlocking(DATA_UART, g_ADC1_resultBuffer, sizeof(g_ADC1_resultBuffer) - 1);

				for (i = 0; i < B_SIZE; i+2) {
					PRINTF("\r");
					PRINTF("%d \t\t", g_ADC1_resultBuffer[i++]);
					PRINTF("%d \t\t", g_ADC1_resultBuffer[i++]);
					PRINTF("%d \t\t", g_ADC1_resultBuffer[i++]);
					PRINTF("%d \t\t", g_ADC1_resultBuffer[i++]);
					PRINTF("\r\n");
				}
				g_Transfer_Done_ch3 = false;
			}
		}
	}
	return 0;
}
