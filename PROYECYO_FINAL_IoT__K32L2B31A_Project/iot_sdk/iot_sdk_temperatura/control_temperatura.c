/*! @file : control_temperatura.c
 * @author  Luis Carlos Nigrinis Alvarez
 * @version 1.0.0
 * @date    28/11/2021
 * @brief   Driver para 
 * @details
 *
*/
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdio.h>
#include "irq_lptmr0.h"
#include "pin_mux.h"
#include "board.h"
#include "control_temperatura.h"
#include "leds.h"
#include "fsl_spi.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SPI_MASTER              SPI0
#define EXAMPLE_SPI_MASTER_SOURCE_CLOCK kCLOCK_BusClk
#define EXAMPLE_SPI_MASTER_CLK_FREQ     CLOCK_GetFreq(kCLOCK_BusClk)

#define BUFFER_SIZE (2)


/*******************************************************************************
 * Private Prototypes
 ******************************************************************************/


/*******************************************************************************
 * External vars
 ******************************************************************************/


/*******************************************************************************
 * Local vars
 ******************************************************************************/
static uint8_t srcBuff[BUFFER_SIZE];
static uint16_t destBuff[BUFFER_SIZE];

volatile float sensor_temperatura;
uint16_t dato_original;
uint16_t dato_entero;
uint16_t dato_final_entero;
uint16_t dato_final_decimal;
uint16_t dato_decimal ;
float temperatura_grados ;
float temperatura_correccion;

spi_master_config_t userConfig = {0};
uint32_t srcFreq               = 0;
//uint32_t i                     = 0;
//uint32_t err                   = 0;
spi_transfer_t xfer            = {0};

/*******************************************************************************
 * Private Source Code
 ******************************************************************************/
void Sensor_temperatura_Init(void){


    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    PRINTF("\n\rMaster Start...\n\r");
    /*
     * userConfig.enableStopInWaitMode = false;
     * userConfig.polarity = kSPI_ClockPolarityActiveHigh;
     * userConfig.phase = kSPI_ClockPhaseFirstEdge;
     * userConfig.direction = kSPI_MsbFirst;
     * userConfig.dataMode = kSPI_8BitMode;
     * userConfig.txWatermark = kSPI_TxFifoOneHalfEmpty;
     * userConfig.rxWatermark = kSPI_RxFifoOneHalfFull;
     * userConfig.pinMode = kSPI_PinModeNormal;
     * userConfig.outputMode = kSPI_SlaveSelectAutomaticOutput;
     * userConfig.baudRate_Bps = 500000U;
     */
    SPI_MasterGetDefaultConfig(&userConfig);
    userConfig.dataMode = kSPI_16BitMode;
    userConfig.outputMode = kSPI_SlaveSelectAutomaticOutput;
    userConfig.direction = kSPI_MsbFirst;
    userConfig.pinMode = kSPI_PinModeNormal;

    srcFreq = EXAMPLE_SPI_MASTER_CLK_FREQ;
    SPI_MasterInit(EXAMPLE_SPI_MASTER, &userConfig, srcFreq);
}


float Sensor_temperatura_Captura_de_datos(void){
	if(tiempo_capturadato_temperatura==100){
		 /*Start Transfer*/
		xfer.txData   = srcBuff;
		xfer.rxData   = destBuff;
		xfer.dataSize = BUFFER_SIZE;
		SPI_MasterTransferBlocking(EXAMPLE_SPI_MASTER, &xfer);
		encender_led_rojo();
	}
	if(tiempo_capturadato_temperatura > 300){
		apagar_led_rojo();
		tiempo_capturadato_temperatura = 0;
		dato_original = destBuff[0] ;
		dato_decimal = dato_original & 0x32;
		dato_final_decimal  = dato_decimal>>3;
		dato_entero = dato_original & 0b0000111111100000;
		dato_final_entero = dato_entero>>5;

		temperatura_grados = dato_final_entero + dato_final_decimal*0.1;

		if(temperatura_grados > 10){
			temperatura_correccion = temperatura_grados;
		}
		//printf("16_bits---%d\r\n",dato_original);
		//printf("%0.2f\r\n",temperatura_grados);
	}

	return(temperatura_correccion);
}


/*******************************************************************************
 * Public Source Code
 ******************************************************************************/


void Sensor_temperatura_Task_Run(void){
	  sensor_temperatura = Sensor_temperatura_Captura_de_datos();
	  //printf("%0.2f\r\n",sensor_temperatura);
}
