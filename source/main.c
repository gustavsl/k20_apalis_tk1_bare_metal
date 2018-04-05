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
#include "fsl_dspi.h"
#include "fsl_edma.h"

void BOARD_ADCInit(void);
void BOARD_SPIInitAsMaster(void);
void BOARD_SPIInitAsSlave(void);
void SPITransferAsMaster(void);
int SPITransferAsSlave(void);
void DSPI_SlaveUserCallback(SPI_Type *base, dspi_slave_handle_t *handle, status_t status, void *isTransferCompleted);

// Defines
#define N_SAMPLES 1
#define N_BYTES 1 // 5 is used for sample, 1 is used for \t

// Globals
dspi_master_handle_t g_m_handle; //global variable
dspi_slave_handle_t g_s_handle;//global variable

dspi_transfer_t masterXfer;
dspi_transfer_t slaveXfer;

uint8_t masterSendBuffer[N_SAMPLES*N_BYTES];
uint8_t masterReceiveBuffer[N_SAMPLES*N_BYTES];
uint16_t transfer_dataSize = N_SAMPLES*N_BYTES;

uint8_t slaveSendBuffer[N_SAMPLES*N_BYTES];
uint8_t slaveReceiveBuffer[N_SAMPLES*N_BYTES];

volatile bool isTransferCompleted = false;

/*!
 * @brief Application entry point.
 */
int main(void) {

	/* Init board hardware. */
	BOARD_InitPins();
	BOARD_BootClockRUN();
	BOARD_InitDebugConsole();
	PRINTF("\033[2J"); // Limpa tela, em ANSI
	PRINTF("\033[H");  // Cursor para home
	PRINTF("\r -- Aloha from MK20DN512MC10 ADC-SPI V00 R00! \n");

	//BOARD_ADCInit();
	//PRINTF("\r -- ADC is initialized! \n");

	// Init SPI as slave
	BOARD_SPIInitAsSlave();
	PRINTF("\r -- SPI is initialized! \n");

	// Fill transfer buffer
	uint32_t local_counter = 0;
	/*for (local_counter = 0; local_counter < N_SAMPLES*N_BYTES - 2; local_counter++){
		slaveSendBuffer[local_counter] = local_counter;
	}
	slaveSendBuffer[N_SAMPLES*N_BYTES - 1] = 9; // 9 for tab, 10 for line feed*/
	slaveSendBuffer[0] = 0xA;

	int ret_code = 0;
	local_counter = 0;
	for(;;) { /* Infinite loop to avoid leaving the main function */
		local_counter++;
		if (local_counter == 10000){
			//GPIO_WritePinOutput(GPIOA, 5, 1);
			//GPIO_WritePinOutput(GPIOA, 3, 1);
			GPIO_WritePinOutput(GPIOA, 17, 1);
			GPIO_WritePinOutput(GPIOA, 5, 1);

			ret_code = SPITransferAsSlave();
			PRINTF("\r -- Return code is: %d \n", ret_code);
			//GPIO_WritePinOutput(GPIOA, 5, 0);
		}else if (local_counter == 20000){
			local_counter = 0;
			//GPIO_WritePinOutput(GPIOA, 3, 0);
			GPIO_WritePinOutput(GPIOA, 17, 0);
			GPIO_WritePinOutput(GPIOA, 5, 0);
			//PRINTF("\r -- Alive! \n");
		}
	}
}

void BOARD_ADCInit(void){

}

void BOARD_SPIInitAsMaster(void){
	dspi_master_config_t  masterConfig;
	DSPI_MasterGetDefaultConfig(&masterConfig);
	DSPI_MasterInit(SPI2, &masterConfig, CLOCK_GetFreq(kCLOCK_CoreSysClk));
	//DSPI_MasterTransferCreateHandle(SPI2, &g_m_handle, NULL, NULL);
}

void BOARD_SPIInitAsSlave(void){
	dspi_slave_config_t  slaveConfig;
	gpio_pin_config_t gpio_out_config = {
				kGPIO_DigitalOutput, 0,
		};
	GPIO_PinInit(GPIOD, 11u, &gpio_out_config);

	slaveConfig.whichCtar = kDSPI_Ctar0;
	slaveConfig.ctarConfig.bitsPerFrame = 8;
	slaveConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh;
	slaveConfig.ctarConfig.cpha = kDSPI_ClockPhaseSecondEdge;
	slaveConfig.enableContinuousSCK = false;
	slaveConfig.enableRxFifoOverWrite = true;
	slaveConfig.enableModifiedTimingFormat = false;
	slaveConfig.samplePoint = kDSPI_SckToSin0Clock;

	DSPI_SlaveInit(SPI2, &slaveConfig);
	NVIC_SetPriority(SPI2_IRQn, 0U);
	DSPI_SlaveTransferCreateHandle(SPI2, &g_s_handle, DSPI_SlaveUserCallback, NULL);

}

void SPITransferAsMaster(void){
	uint16_t local_counter = 0;
	for (local_counter = 0; local_counter < N_SAMPLES*N_BYTES - 2; local_counter++){
		masterSendBuffer[local_counter] = local_counter;
	}
	masterSendBuffer[N_SAMPLES*N_BYTES - 1] = 9; // 9 for tab, 10 for line feed

	masterXfer.txData      = masterSendBuffer;
	masterXfer.rxData      = masterReceiveBuffer;
	masterXfer.dataSize    = transfer_dataSize;
	masterXfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 ;
	DSPI_MasterTransferBlocking(SPI2, &masterXfer);
}

int SPITransferAsSlave(void){
	int32_t spi_return_code = 0;

	slaveXfer.txData      = slaveSendBuffer;
	slaveXfer.rxData      = slaveReceiveBuffer;
	slaveXfer.dataSize    = transfer_dataSize;
	slaveXfer.configFlags = kDSPI_SlaveCtar0;
	isTransferCompleted = false;
	spi_return_code = DSPI_SlaveTransferNonBlocking(SPI2, &g_s_handle, &slaveXfer);
	return spi_return_code;
	//PRINTF("\r -- Received message: %i \n", received_msg);
}

void DSPI_SlaveUserCallback(SPI_Type *base, dspi_slave_handle_t *handle, status_t status, void *isTransferCompleted){
	PRINTF("\r -- This is DSPI slave call back. \n");
	if (status == kStatus_Success){
		__NOP();
	}
	else if (status == kStatus_DSPI_Error){
		__NOP();
	}
	isTransferCompleted = true;
	//*((bool *)isTransferCompleted) = true;

	PRINTF("\r -- This is DSPI slave call back. \n");
}

