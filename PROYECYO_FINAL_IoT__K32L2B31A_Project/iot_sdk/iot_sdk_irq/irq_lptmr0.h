/*! @file : irq_lptmr0.h
 * @author
 * @version 1.0.0
 * @date    11/09/2021
 * @brief   Driver para 
 * @details
 *
 */
#ifndef IOT_SDK_IRQ_IRQ_LPTMR0_H_
#define IOT_SDK_IRQ_IRQ_LPTMR0_H_
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include"peripherals.h"

/*!
 * @addtogroup IRQ
 * @{
 */
/*!
 * @addtogroup LPRMR0
 * @{
 */
/*******************************************************************************
 * Public Definitions
 ******************************************************************************/

/*******************************************************************************
 * External vars
 ******************************************************************************/

/*******************************************************************************
 * Public vars
 ******************************************************************************/
extern volatile uint32_t lptmr0_irq_counter;
extern volatile uint32_t tiempopresionado;
extern volatile uint32_t tiemposensorultrasonico;
extern volatile uint32_t tiempo_capturadato_temperatura;
extern volatile uint32_t tiempo_captura_datos;
extern volatile uint32_t tiempofermentacion;
extern volatile uint32_t tiempodestilacion;
extern volatile uint32_t boton1_presionado;
extern volatile uint32_t estado;
extern volatile uint32_t tiempo_envio_a_rabbit;
extern volatile uint32_t tiempo_conectando_a_la_red;
/*******************************************************************************
 * Public Prototypes
 ******************************************************************************/

/** @} */ // end of X LPTMR0
/** @} */ // end of X IRQ

#endif /* IOT_SDK_IRQ_IRQ_LPTMR0_H_ */
