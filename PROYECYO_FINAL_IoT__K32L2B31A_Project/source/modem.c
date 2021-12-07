/*! @file : modem.c
 * @author  José Morales Vega
 * @version 1.0.0
 * @date    21/10/2021
 * @brief   Driver para 
 * @details
 *
*/
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "modem.h"
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "K32L2B31A.h"
#include "fsl_debug_console.h"
#include "leds.h"
#include "botones.h"
#include "irq_lptmr0.h"
#include "fsl_gpio.h"
#include "sensor_ultrasonico_dp1.h"
#include "sensor_MQ3.h"
#include "cronometro.h"



/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TRUE 1
#define FALSE 0
#define CNTL_Z	0x1A  // fin de Tx envio SMS y PUB MQTT
#define APN_TIGO TRUE

#ifdef APN_CLARO
const char APN_APP[]="internet.comcel.com.co";
#endif

#ifdef APN_MOVISTAR
const char APN_APP[]="internet.movistar.com.co";
#endif

#ifdef APN_TIGO
const char APN_APP[]="internet.colombiamovil.com.co";
#endif

/*******************************************************************************
 * Private Prototypes
 ******************************************************************************/


/*******************************************************************************
 * External vars
 ******************************************************************************/



extern uint8_t minutos;
extern uint8_t segundos;
extern uint8_t horas;

extern uint8_t segundos_destilacion;
extern uint8_t minutos_destilacion;
extern uint8_t horas_destilacion;

extern float mililitros_alcohol;
extern float sensor_temperatura;
extern float alcohol;
extern float total_minutos_destilacion;




/*******************************************************************************
 * Local vars
 ******************************************************************************/


/*******************************************************************************
 * Private Source Code
 ******************************************************************************/
// Alarma.h
void Alarma_Init(void);
uint32_t Alarma_Set(uint32_t time2Wait);
char Alarma_Elapsed(uint32_t time2Test);

char Modem_Rta_Cmd(uint32_t time2Wait,char *rtaValidaStr,char estadoOk,char estadoErr);
char Modem_Rta_Cmd_2(char *rtaValidaStr,char estadoOk);



char Recibido_URC(void);
void Modem_Check_URC_Run(void);


// Modulo Modem

// Fecha Sep 24 2021
// Modulo para el manejo del Model Quectel EC200T



// recibe un apuntador a topico y  los datos a enviar
// ex:
//   Send_MQTT_Data("LAB1","Hola Mundo");

void Send_MQTT_Data(char * topicPtr,char *datosPtr){
	//
}

// retorna estado de envio
// ST_MQTT_EN_PROCESS
// ST_MQTT_ERROR_CONEX
// ST_MQTT_OK

#define SIZE_BUFFER_COMANDO	50

char mqttSendSt;
char buffer_comando_recibido[SIZE_BUFFER_COMANDO];
char rtaModemStr[50];
char modemEstadoOK,modemEstadoExcepcion;
static char procesandoComando=0;
static uint32_t timeOutRta;
char 		buffer_comando_enviar[SIZE_BUFFER_COMANDO];
uint32_t 	index_buffer_nuevo_comandio_recibido=0;


void Modem_Rta_Run(void);
int32_t numeroDeBytesDisponiblesEnBufferRx(void);
uint8_t pullByteDesdeBufferCircular(void);

void limpiarBufferComando(){
	for(int i=0;i<SIZE_BUFFER_COMANDO;i++){
		buffer_comando_recibido[i]=0x00;
	}
	index_buffer_nuevo_comandio_recibido=0;
}


char Send_MQTT_Error(void){
	return mqttSendSt;
}


char modemSt;

// Estados de modemSt
enum{
	ST_MOD_IDLE,
	ST_MOD_CFG,	 // Configuracion inicial
	ST_MOD_ECO,  // quita ECO
	ST_MOD_CFG_URC,
	ST_MOD_CFG_ALL_URC,
	ST_MOD_PWR_OFF,
	ST_MOD_APN,
	ST_MOD_PWR_ON,
	ST_MOD_SIM,
	ST_MOD_SIGNAL,
	ST_MOD_SEARCHING,
	ST_MOD_ACT_CTX,
	ST_MOD_OPEN_MQTT,
	ST_MOD_CONN_TOPIC,
	ST_MOD_CONN_PUB,
	ST_MOD_PUBLIC_DAT,
	ST_ERROR_SIM,
	ST_MOD_ERROR_SENAL,
	ST_MOD_ERROR_REG,
	ST_MOD_ERROR_IP,
	ST_MOD_ERROR_CX_MQTT,
	ST_MOD_KEEP_ALIVE,  // manda un AT y espera un OK
	ST_MOD_CHK_URC,
	ST_MOD_RING_ON,
	ST_MOD_RING_OFF,
	ST_MOD_IDENTIFICADOR,
	ST_MOD_NUMERO_2,
	ST_MOD_NUMERO_3,
	ST_MOD_NUMERO_4,
	ST_ESPERAR_10_SEGUNDOS,
	ON_VERDE,

};

void Modem_Init(void){
	modemSt = ST_MOD_CFG;
}



//uint32_t tiempopresionado;
uint32_t cuenta=1;
//uint32_t n;

bool B1,B2;
//static char recibiMsgQtt;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Modem_Task_Run(void){

	Modem_Rta_Run();
	switch(modemSt){
	case ST_MOD_IDLE: // IDLE
	break;
	case ST_MOD_CFG:
		Modem_Send_Cmd("ATE0\r\n"); 								// ATE0 Quitar ECO
      	Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_MOD_CFG_URC,ST_MOD_CFG); //rx OK
	break;
	case ST_MOD_CFG_URC:
		Modem_Send_Cmd("AT+QURCCFG=\"urcport\",\"uart1\"\r\n");
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_MOD_CFG_ALL_URC,ST_MOD_CFG_URC);
	break;
	case ST_MOD_CFG_ALL_URC:
		Modem_Send_Cmd("AT+QINDCFG=\"ALL\",1,1\r\n");
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_MOD_PWR_OFF,ST_MOD_CFG_URC);
	break;
	case ST_MOD_PWR_OFF:
		Modem_Send_Cmd("AT+CFUN=0\r\n"); // Modo Avion				//AT+CFUN=0
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_MOD_APN,ST_MOD_PWR_OFF); //rx OK
 	break;
	case ST_MOD_APN:
		buffer_comando_enviar[0] = 0;
		strcat(buffer_comando_enviar,"AT+CGDCONT=1,\"IP\"");
		strcat(buffer_comando_enviar,APN_APP);
		strcat(buffer_comando_enviar,"\r\n");
		Modem_Send_Cmd(buffer_comando_enviar); //Modem_Send_Cmd("AT+CGDCONT=1,\"IP\",\"internet.comcel.com.co\"\r\n"); //tx "AT+CGDCONT=1,"IP",APN_APP
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_MOD_PWR_ON,ST_MOD_APN); 	// rx "OK"
	break;
	case ST_MOD_PWR_ON:
		Modem_Send_Cmd("AT+CFUN=1\r\n");								//tx "AT+CFUN=1"
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_MOD_SIM,ST_MOD_PWR_ON); 	//rx "OK"
    break;
 	case ST_MOD_SIM:
 		Modem_Send_Cmd("AT+CPIN?\r\n");									//tx "AT+CPIN?"
 		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"READY",ST_MOD_SIGNAL,ST_MOD_SIM); // rx "READY"
	break;
	case ST_MOD_SIGNAL:
		Modem_Send_Cmd("AT+CSQ\r\n");									//tx "AT+CSQ"
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"+CSQ",ST_MOD_SEARCHING,ST_MOD_SIGNAL);
	break;
	case ST_MOD_SEARCHING:
		encender_led_rojo();
		apagar_led_verde();
		Modem_Send_Cmd("AT+CREG?\r\n");											//tx "AT+CREG?"
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"0,1",ST_MOD_ACT_CTX,ST_MOD_SEARCHING); 	//rx  "0,1"
	break;
	case ST_MOD_ACT_CTX:
		encender_led_verde();
		apagar_led_rojo();
		Modem_Send_Cmd("AT+QIACT=1\r\n"); 									//tx "AT+QIACT=1"
		Modem_Rta_Cmd(10000,"OK",ST_MOD_OPEN_MQTT,ST_MOD_ACT_CTX); 	//rx "OK"
	break;
	case ST_MOD_OPEN_MQTT:
		Modem_Send_Cmd("AT+QMTOPEN=0,\"52.149.151.75\",1883\r\n"); //tx "AT+QMTOPEN=0,"52.149.151.75",1883"
		Modem_Rta_Cmd(5000,"+QMTOPEN: 0,0",ST_MOD_CONN_TOPIC,ST_MOD_ACT_CTX);
	break;
	case ST_MOD_CONN_TOPIC:
		Modem_Send_Cmd("AT+QMTCONN=0,\"mensajes\"\r\n");	//tx "AT+QMTCONN=0,"TOPICO_APP""
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"+QMTCONN: 0,0,0",ST_MOD_CONN_PUB,ST_MOD_OPEN_MQTT);
	break;
	case ST_MOD_CONN_PUB:
		Modem_Send_Cmd("AT+QMTPUB=0,0,0,0,\"mensajes\"\r\n");		//tx "AT+QMTPUB=0,0,0,0,TOPICO_APP"
		Modem_Rta_Cmd(5000,">",ST_MOD_PUBLIC_DAT,ST_MOD_CONN_TOPIC);
	break;
	case ST_MOD_PUBLIC_DAT:
		//encender_led_rojo();
		printf("%d,%d,%d,%d,%d,%d,%0.1f,%0.1f,%0.1f,%0.1f\r\n",horas,minutos,segundos,horas_destilacion,minutos_destilacion,segundos_destilacion,sensor_temperatura,mililitros_alcohol,alcohol,total_minutos_destilacion);
		//printf("%0.2f\r\n",sensor_temperatura);
		putchar(CNTL_Z);
		Modem_Rta_Cmd(5000,"OK",ST_MOD_CONN_PUB,ST_MOD_CONN_PUB);
		//recibiMsgQtt = 0;*/
		//Modem_Rta_Cmd_2("RING",ST_MOD_RING_ON);
		//Key_Task_Run();

	break;


///////////////////////////////////////////////////////////////////////////////


	/*case ST_ERROR_SIM:
		// SIM no insertada
	break;
	case ST_MOD_ERROR_SENAL:
		// NO hay antena o no hay señal
	break;
	case ST_MOD_ERROR_REG:
		// No pudo registrar, decidir que hacer
	break;
	case ST_MOD_ERROR_IP:
		// no pudo obtener una IP, posible APN o falta de datos en la SIM
	break;
	case ST_MOD_ERROR_CX_MQTT:
		//notificar error de envio
		//MQTT_Error(ST_MQTT_ERROR_SEND);
	break;
	case ST_MOD_KEEP_ALIVE:
		Modem_Send_Cmd("AT\r\n");
		Modem_Rta_Cmd(3, "OK", ST_MOD_CHK_URC, ST_MOD_KEEP_ALIVE);
	break;*/
	/*case ST_MOD_CHK_URC:
		if(Recibido_URC()){
			Modem_Check_URC_Run();
			if(recibiMsgQtt){
				recibiMsgQtt = 0;
				modemSt = ST_MOD_CONN_PUB;
			}
		}
		if(Boton1_Presionado()){
			tiempopresionado=temp;
		}
		if(tiempopresionado=5){
			printf("%d\r\n",tiempopresionado);
		}
		printf("ST_MOD_CONN_PUB \r\n");
	break;*/

	}

 }


////////////////////////////////////////////////////////////AQUIII
void Key_Task_Run(void){
	encender_led_rojo();
 	B1=boton1LeerEstado();
 	B2=boton2LeerEstado();
 	if (!B1 && !boton1_presionado){
 	    	boton1_presionado=1;
 	    	tiempopresionado=0;
 	 }
 	if(tiempopresionado==1000){
 	 	  if(!B1 && !estado){
 	 		  putchar(CNTL_Z);
 	 		  Modem_Rta_Cmd(10000,"OK",ST_MOD_PUBLIC_DAT,ST_MOD_PUBLIC_DAT);
 	 		  estado=1;
 	 	  }
 	}

 	if (B1){
 		  boton1_presionado=0;
 		  estado=0;
 		  Modem_Rta_Cmd(3000,"RING",ST_MOD_RING_ON,ST_MOD_PUBLIC_DAT);
 	}


}

//////////////////////////////////////////////////////////////7AQUIII

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*char Test_Rta_Modem(char *rta2TestStr){
char *puntero_ok;
	puntero_ok=(char*)(strstr((char*)(&buffer_comando_recibido[0]),rta2TestStr));
	if(puntero_ok) return 1;
	else return 0;
}*/



// Retorna TRUE si la respuesta es la Esperada
char Respuesta_Modem(char *rtaEsperada){
unsigned char nroBytesRecibidos,k;
char *puntero_ok;

	nroBytesRecibidos = numeroDeBytesDisponiblesEnBufferRx();
	if(nroBytesRecibidos){
		for(k=0;k<nroBytesRecibidos;k++){
			buffer_comando_recibido[k]=pullByteDesdeBufferCircular();
    	}
		puntero_ok=(char*)(strstr((char*)(&buffer_comando_recibido[0]),rtaEsperada));
		if(puntero_ok == 0) return 0;
		else return 1;
 	}else return 0;
	return 0;
}

// // Descr: Esta funcion recibe como argumento el tiempo que espera del
// modem una respuesta valida y pasa al siguiente estado normal, de lo
// contrario con una respuesta diferente pasa al estado llamado estadoErr.(excepcion)
//  Arguentos:
// 	time2Wait [=] Segundos que espera la respuesta
//

// Proyecto IoT para Diplomado
//
char Modem_Rta_Cmd(uint32_t time2Wait,char *rtaValidaStr,char estadoOk,char estadoErr){
	if(!procesandoComando){
		procesandoComando = 1;
   		limpiarBufferComando();
		timeOutRta = Alarma_Set(time2Wait);
		strcpy(rtaModemStr,rtaValidaStr);
		modemEstadoOK = estadoOk;
		modemEstadoExcepcion = estadoErr;
		modemSt = ST_MOD_IDLE;
		return 1;
	} return 0;
}

void registro_llamada(void){

}

char Modem_Rta_Cmd_2(char *rtaValidaStr,char estadoOk){
		if(!procesandoComando){
			procesandoComando = 1;
	   		limpiarBufferComando();
			strcpy(rtaModemStr,rtaValidaStr);
			modemEstadoOK = estadoOk;
			modemSt = ST_MOD_IDLE;
			return 1;
	} return 0;
}

// Funcion de llamado continuo
void Modem_Rta_Run(void){
	if(procesandoComando){
		if(Alarma_Elapsed(timeOutRta)){
			if(Respuesta_Modem(rtaModemStr)){
				modemSt = modemEstadoOK;
			}else{
				modemSt = modemEstadoExcepcion;
			}
			procesandoComando = 0;
		}
	}
}

/*void Modem_Check_URC_Run(void){
	//!!! Recepcion de URCs
	if(Respuesta_Modem("+QMTRECV")){ // mensaje MQTT
		recibiMsgQtt = 1;
		if(Test_Rta_Modem("LED ON")){
			 encender_led_verde();
		 }else{
			 if(Test_Rta_Modem("LED OFF")){
				 apagar_led_verde();
			 }
		 }
	}else{  // algun otro URC
		if(Test_Rta_Modem("OK")){

		}
	}
}*/

/*char Recibido_URC(void){
static uint32_t time2waitEnd_URC;
	if(numeroDeBytesDisponiblesEnBufferRx()){
		if(Alarma_Elapsed(time2waitEnd_URC)){
			return 1;
		}
		return 0;
	}else{
		time2waitEnd_URC = Alarma_Set(10); //tiempo que espera para fin de Rx de URC
		return 0;
	}
}*/



/*******************************************************************************
 * Public Source Code
 ******************************************************************************/
