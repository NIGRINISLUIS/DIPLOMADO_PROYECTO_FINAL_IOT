/*! @file : cronometro.c
 * @author  José Morales Vega
 * @version 1.0.0
 * @date    7/11/2021
 * @brief   Driver para 
 * @details
 *
*/
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "cronometro.h"
#include "irq_lptmr0.h"
#include "pin_mux.h"
#include "board.h"
#include "leds.h"
#include "control_temperatura.h"
#include "sensor_MQ3.h"
#include "sensor_ultrasonico_dp1.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Private Prototypes
 ******************************************************************************/


/*******************************************************************************
 * External vars
 ******************************************************************************/
extern float sensor_temperatura;
extern float alcohol;
extern volatile float sensor_1_ultrasonico;

/*******************************************************************************
 * Local vars
 ******************************************************************************/
uint8_t segundos;
uint8_t minutos=0;
uint8_t horas=0;
uint8_t segundos_destilacion;
uint8_t minutos_destilacion=0;
uint8_t horas_destilacion=0;
uint8_t pulsador_fermentacion = 1;
uint8_t pulsador_destilacion = 1;
float mililitros_alcohol = 0 ;
float total_minutos_destilacion = 0 ;

/*******************************************************************************
 * Private Source Code
 ******************************************************************************/
void delay_200ms_block(){
  	uint32_t l;
  	for(l=0;l<0xCCCCD;l++);
}
void Fermentacion(void){

	if(GPIO_PinRead(GPIOC,1)==0){
		 pulsador_fermentacion = pulsador_fermentacion + 1;
		 delay_200ms_block();
	}
	if(pulsador_fermentacion % 2 == 0){
   		tiempofermentacion=0;
		segundos=0;
		minutos=0;
		horas=0;
		encender_led_verde();
	}
	if(pulsador_fermentacion%2==1  && GPIO_PinRead(GPIOC,1)!=0){
		segundos=tiempofermentacion*0.001;
		if(segundos==60){
			minutos++;
			if(minutos==60){
				horas++;
   				minutos=0;
				segundos=0;
			}
			segundos=0;
			tiempofermentacion=0;
		}
		if(segundos>=1){
			apagar_led_verde();
		}

	}
}


void Destilacion(void){
	if(GPIO_PinRead(GPIOE,1)==0){
		pulsador_destilacion = pulsador_destilacion + 1;
		  delay_200ms_block();
	}

	if(pulsador_destilacion % 2 == 0){
		tiempodestilacion=0;
		segundos_destilacion=0;
		minutos_destilacion=0;
		horas_destilacion=0;
		encender_led_verde();
		sensor_temperatura = 0;
		alcohol = 0;
		total_minutos_destilacion = 0;
		mililitros_alcohol = sensor_1_ultrasonico*0.0;
		GPIO_PinWrite(GPIOA,13,0);
	}

	if(pulsador_destilacion%2==1  && GPIO_PinRead(GPIOE,1)!=0){
		segundos_destilacion=tiempodestilacion*0.001;
		if(segundos_destilacion==60){
			minutos_destilacion++;
			if(minutos_destilacion==60){
				horas_destilacion++;
				minutos_destilacion=0;
				segundos_destilacion=0;
			}
			segundos_destilacion=0;
			tiempodestilacion=0;
		}
		if(segundos_destilacion>=1){
			apagar_led_verde();
		}
		Sensor_temperatura_Task_Run();
	    sensor_MQ3_Task_Run();
	    mililitros_alcohol = sensor_1_ultrasonico;
	    if(sensor_temperatura > 35.0){
	    	GPIO_PinWrite(GPIOA,13,0);
	    }
	    total_minutos_destilacion = minutos_destilacion + segundos_destilacion*0.017 + horas_destilacion*60;
	    if(sensor_temperatura < 33.0){
	    	GPIO_PinWrite(GPIOA,13,1);
	    }
	}

}
/*******************************************************************************
 * Public Source Code
 ******************************************************************************/
