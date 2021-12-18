/*! @file : irq_lptmr0.c
 * @author
 * @version 1.0.0
 * @date    11/09/2021
 * @brief   Driver para 
 * @details
 *
*/
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "irq_lptmr0.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Private Prototypes
 ******************************************************************************/


/*******************************************************************************
 * External vars
 ******************************************************************************/


/*******************************************************************************
 * Local vars
 ******************************************************************************/


 volatile uint32_t boton1_presionado=0;
 volatile uint32_t boton2_presionado=0;

 volatile uint32_t lptmr0_irq_counter=0;
 volatile uint32_t tiempopresionado=0;
 volatile uint32_t tiemposensorultrasonico=0;
 volatile uint32_t tiempo_capturadato_temperatura=0;
 volatile uint32_t tiempofermentacion=0;
 volatile uint32_t tiempodestilacion=0;
 volatile uint32_t tiempo_captura_datos=0;
 volatile uint32_t tiempo_envio_a_rabbit=0;
 volatile uint32_t tiempo_conectando_a_la_red=0;



 volatile uint32_t estado=0;
/*******************************************************************************
 * Private Source Code
 ******************************************************************************/
/* LPTMR0_IRQn interrupt handler */
void LPTMR0_IRQHANDLER(void) {
  uint32_t intStatus;
  /* Reading all interrupt flags of status register */
  intStatus = LPTMR_GetStatusFlags(LPTMR0_PERIPHERAL);
  LPTMR_ClearStatusFlags(LPTMR0_PERIPHERAL, intStatus);

  /* Place your code here */

   lptmr0_irq_counter++;
   tiemposensorultrasonico++;
   tiempo_capturadato_temperatura++;
   tiempofermentacion++;
   tiempo_captura_datos++;
   tiempodestilacion++;
   tiempopresionado++;
   tiempo_envio_a_rabbit++;
   tiempo_conectando_a_la_red++;


   /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
     Store immediate overlapping exception return operation might vector to incorrect interrupt. */
  #if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
  #endif
}


/*******************************************************************************
 * Public Source Code
 ******************************************************************************/
