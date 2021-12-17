/*! @file : sensor_MQ3.c
 * @author  Luis Carlos Nigrinis Alvarez
 * @version 1.0.0
 * @date    2/12/2021
 * @brief   Driver para 
 * @details
 *
*/
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "sensor_MQ3.h"
#include <stdio.h>

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_adc16.h"
#include "math.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_ADC16_BASE          ADC0
#define DEMO_ADC16_CHANNEL_GROUP 0U
#define DEMO_ADC16_USER_CHANNEL  23U /* PTE30, ADC0_SE23 */



/*******************************************************************************
 * Private Prototypes
 ******************************************************************************/


/*******************************************************************************
 * External vars
 ******************************************************************************/


/*******************************************************************************
 * Local vars
 ******************************************************************************/
adc16_channel_config_t adc16ChannelConfigStruct;
adc16_config_t adc16ConfigStruct;
uint16_t adc_sensor_MQ3;
float voltaje;
float Rs;
float alcohol;
/*******************************************************************************
 * Private Source Code
 ******************************************************************************/
void sensor_MQ3_Init(void){
	ADC16_GetDefaultConfig(&adc16ConfigStruct);
	adc16ConfigStruct.referenceVoltageSource = kADC16_ReferenceVoltageSourceValt;
	adc16ConfigStruct.resolution = kADC16_ResolutionSE10Bit;
	ADC16_Init(DEMO_ADC16_BASE, &adc16ConfigStruct);
	ADC16_EnableHardwareTrigger(DEMO_ADC16_BASE, false); /* Make sure the software trigger is used. */

}
void sensor_MQ3_obtenerdato(void){

	adc16ChannelConfigStruct.channelNumber = DEMO_ADC16_USER_CHANNEL;
	adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;
	adc16ChannelConfigStruct.enableDifferentialConversion = false;
}

void sensor_MQ3_Task_Run(void){
	sensor_MQ3_Init();
	sensor_MQ3_obtenerdato();
	ADC16_SetChannelConfig(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
	adc_sensor_MQ3 = ADC16_GetChannelConversionValue(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP);
    voltaje = adc_sensor_MQ3 * (5 / 1023.0);

    if(voltaje < 5){
    	Rs = 1000*((5-voltaje)/voltaje);
    	alcohol = (0.4091*pow(Rs/5463, -1.497))-1;
    }
    //alcohol = voltaje;
    //alcohol = (0.4091*pow(Rs/5463, -1.497))-2;
    //printf("%0.4f\r\n",alcohol);
}

/*******************************************************************************
 * Public Source Code
 ******************************************************************************/

