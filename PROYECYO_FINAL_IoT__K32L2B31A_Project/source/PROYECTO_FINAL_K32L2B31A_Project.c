

/*! @file : K32L2B31A_Project.c
 * @author  Luis Carlos Niginis Alvarez
 * @version 0.0.000
 * @date    23/08/2021
 * @brief   Funcion principal main
 * @details
 *			v0.0.000	Proyecto base creado usando MCUXpresso
 *
 *
 */
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "K32L2B31A.h"
#include "fsl_debug_console.h"
#include "stdbool.h"

#include "leds.h"

#include "botones.h"
#include "modem.h"
#include "irq_lptmr0.h"
#include "irq_lpuart0.h"
#include "sensor_ultrasonico_dp1.h"

#include "cronometro.h"
#include "control_temperatura.h"


#include "fsl_adc16.h"
#include "sensor_MQ3.h"




#define DEMO_ADC16_BASE          ADC0
#define DEMO_ADC16_CHANNEL_GROUP 0U
#define DEMO_ADC16_USER_CHANNEL  23U /* PTE30, ADC0_SE23 */

// Para modulo Alarma
volatile uint32_t segAct=0;
volatile uint32_t tiempoAct=0;

// Inicializa Alarma
void Alarma_Init(void){
	segAct = lptmr0_irq_counter;
}

// retorna el tiempo futuro de la alarma
uint32_t Alarma_Set(uint32_t time2Wait){
	segAct = lptmr0_irq_counter;
	return (segAct + time2Wait);
}

// retorna si la alarma se cumplio
char Alarma_Elapsed(uint32_t time2Test){
	segAct = lptmr0_irq_counter;
	if(segAct >= time2Test) return true;
	else return false;
}

#define Timer_Init() LPTMR_StartTimer(LPTMR0)

/* Init board hardware. */
void BOARD_Init(void){
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif
}



//  main
int main(void) {
	BOARD_Init();
    Timer_Init();
    Modem_Init();
    Key_Task_Init();
    Alarma_Init();
    Sensorultrasonico_1_init();
    Sensor_temperatura_Init();
    sensor_MQ3_Init();

    while(1) { // multiTaks de tareas por Polling

    	Modem_Task_Run();

    	if(tiempo_captura_datos > 0 && tiempo_captura_datos < 4000){
    		Fermentacion();
    		Destilacion();
    	}
    	if(tiempo_captura_datos > 4000 && tiempo_captura_datos < 4500){
    		Sensorultrasonico_1_Task_Run();
    	}
    	if(tiempo_captura_datos > 4500){
    		tiempo_captura_datos = 0 ;
    	}
    }
    return 0 ;
}

