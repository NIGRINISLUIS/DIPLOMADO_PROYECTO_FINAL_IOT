/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "fsl_common.h"
#include "fsl_lptmr.h"
#include "fsl_lpuart.h"
#include "fsl_clock.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
/* Definitions for BOARD_InitPeripherals functional group */
/* BOARD_InitPeripherals defines for LPTMR0 */
/* Definition of peripheral ID */
#define LPTMR0_PERIPHERAL LPTMR0
/* Definition of the clock source frequency */
#define LPTMR0_CLK_FREQ 8000000UL
/* Definition of the prescaled clock source frequency */
#define LPTMR0_INPUT_FREQ 8000000UL
/* Definition of the timer period in us */
#define LPTMR0_USEC_COUNT 1000UL
/* Definition of the timer period in number of ticks */
#define LPTMR0_TICKS 8000UL
/* LPTMR0 interrupt vector ID (number). */
#define LPTMR0_IRQN LPTMR0_IRQn
/* LPTMR0 interrupt handler identifier. */
#define LPTMR0_IRQHANDLER LPTMR0_IRQHandler
/* Definition of peripheral ID */
#define LPUART0_PERIPHERAL LPUART0
/* Definition of the clock source frequency */
#define LPUART0_CLOCK_SOURCE 48000000UL
/* LPUART0 interrupt vector ID (number). */
#define LPUART0_SERIAL_RX_TX_IRQN LPUART0_IRQn
/* LPUART0 interrupt handler identifier. */
#define LPUART0_SERIAL_RX_TX_IRQHANDLER LPUART0_IRQHandler

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
extern const lptmr_config_t LPTMR0_config;
extern const lpuart_config_t LPUART0_config;

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/

void BOARD_InitPeripherals(void);

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void);

#if defined(__cplusplus)
}
#endif

#endif /* _PERIPHERALS_H_ */
