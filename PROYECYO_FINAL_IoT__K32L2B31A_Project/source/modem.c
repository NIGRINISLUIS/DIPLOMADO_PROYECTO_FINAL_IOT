/*! @file : modem.c
 * @author
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
float  volumen ;
float temperatura_grados_enviar ;
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
	ST_ACTIVAR_MSJ,
	ST_ACTIVAR_GSM,
	ST_MENSAJE_TEXTO,
	ST_ENVIAR_ALERTA,
	ST_ESPERAR_PARA_ENVIAR_DATOS,
	ST_MENSAJE_DESTILACION,
	ST_MENSAJE_DESTILACION_15M,
	ST_MENSAJE_DESTILACION_30M,
	ST_MENSAJE_DESTILACION_45M,
	ST_MENSAJE_DESTILACION_60M,
	ST_ESPERAR_PARA_RECIBIR_DATOS,
	ST_MENSAJE_RECIBIDO_MQTT_1,
	ST_ESPERAR_RECIBIR_DATOS,
	ST_MENSAJE_ALERTA_MQQTT,

};

void Modem_Init(void){
	modemSt = ST_MOD_CFG;
	GPIO_PinWrite(GPIOE,0,0);
	GPIO_PinWrite(GPIOB,3,1);
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
		GPIO_PinWrite(GPIOB,3,0);
		Modem_Send_Cmd("ATE0\r\n"); 								// ATE0 Quitar ECO
      	Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_MOD_CFG_URC,ST_MOD_CFG); //rx OK
	break;
	case ST_MOD_CFG_URC:
		GPIO_PinWrite(GPIOB,3,1);
		Modem_Send_Cmd("AT+QURCCFG=\"urcport\",\"uart1\"\r\n");
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_MOD_CFG_ALL_URC,ST_MOD_CFG_URC);
	break;
	case ST_MOD_CFG_ALL_URC:
		GPIO_PinWrite(GPIOB,3,0);
		Modem_Send_Cmd("AT+QINDCFG=\"ALL\",1,1\r\n");
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_MOD_PWR_OFF,ST_MOD_CFG_URC);
	break;
	case ST_MOD_PWR_OFF:
		GPIO_PinWrite(GPIOB,3,1);
		Modem_Send_Cmd("AT+CFUN=0\r\n"); // Modo Avion				//AT+CFUN=0
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_MOD_APN,ST_MOD_PWR_OFF); //rx OK
 	break;
	case ST_MOD_APN:
		GPIO_PinWrite(GPIOB,3,0);
		buffer_comando_enviar[0] = 0;
		strcat(buffer_comando_enviar,"AT+CGDCONT=1,\"IP\"");
		strcat(buffer_comando_enviar,APN_APP);
		strcat(buffer_comando_enviar,"\r\n");
		Modem_Send_Cmd(buffer_comando_enviar); //Modem_Send_Cmd("AT+CGDCONT=1,\"IP\",\"internet.comcel.com.co\"\r\n"); //tx "AT+CGDCONT=1,"IP",APN_APP
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_MOD_PWR_ON,ST_MOD_APN); 	// rx "OK"
	break;
	case ST_MOD_PWR_ON:
		GPIO_PinWrite(GPIOB,3,1);
		Modem_Send_Cmd("AT+CFUN=1\r\n");								//tx "AT+CFUN=1"
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_MOD_SIM,ST_MOD_PWR_ON); 	//rx "OK"
    break;
 	case ST_MOD_SIM:
 		GPIO_PinWrite(GPIOB,3,0);
 		Modem_Send_Cmd("AT+CPIN?\r\n");									//tx "AT+CPIN?"
 		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"READY",ST_MOD_SIGNAL,ST_MOD_SIM); // rx "READY"
	break;
	case ST_MOD_SIGNAL:
		GPIO_PinWrite(GPIOB,3,1);
		Modem_Send_Cmd("AT+CSQ\r\n");									//tx "AT+CSQ"
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"+CSQ",ST_MOD_SEARCHING,ST_MOD_SIGNAL);
	break;
	case ST_MOD_SEARCHING:
		GPIO_PinWrite(GPIOB,3,0);
		Modem_Send_Cmd("AT+CREG?\r\n");											//tx "AT+CREG?"
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"0,1",ST_ACTIVAR_MSJ,ST_MOD_SEARCHING); 	//rx  "0,1"
	break;
	case ST_ACTIVAR_MSJ:
		GPIO_PinWrite(GPIOB,3,1);
		Modem_Send_Cmd("AT+CMGF=1\r\n");											//tx "AT+CREG?"
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_ACTIVAR_GSM,ST_MOD_SEARCHING);
	break;

	case ST_ACTIVAR_GSM :
		GPIO_PinWrite(GPIOB,3,0);
		Modem_Send_Cmd("AT+CSCS=\"GSM\"\r\n");											//tx "AT+CREG?"
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"OK",ST_MOD_ACT_CTX,ST_MOD_SEARCHING);
	break;

	case ST_MOD_ACT_CTX:
		GPIO_PinWrite(GPIOB,3,1);
		Modem_Send_Cmd("AT+QIACT=1\r\n"); 									//tx "AT+QIACT=1"
		Modem_Rta_Cmd(10000,"OK",ST_MOD_OPEN_MQTT,ST_MOD_ACT_CTX); 	//rx "OK"
	break;
	case ST_MOD_OPEN_MQTT:
		GPIO_PinWrite(GPIOB,3,0);
		Modem_Send_Cmd("AT+QMTOPEN=0,\"20.121.64.231\",1883\r\n"); //tx "AT+QMTOPEN=0,"20.121.64.231",1883"
		Modem_Rta_Cmd(5000,"+QMTOPEN: 0,0",ST_MOD_CONN_TOPIC,ST_MOD_ACT_CTX);
	break;
	case ST_MOD_CONN_TOPIC:
		GPIO_PinWrite(GPIOB,3,1);
		Modem_Send_Cmd("AT+QMTCONN=0,\"mensajes\"\r\n");	//tx "AT+QMTCONN=0,"TOPICO_APP""
		Modem_Rta_Cmd(TIME_WAIT_RTA_CMD,"+QMTCONN: 0,0,0",ST_MOD_CONN_PUB,ST_MOD_OPEN_MQTT);
	break;
	///////////////////////////////////Aqui empieza los estados de envio y recibo de datos por mqqtt///////

	case ST_MOD_CONN_PUB:
		GPIO_PinWrite(GPIOB,3,1);
		Modem_Send_Cmd("AT+QMTPUB=0,0,0,0,\"mensajes\"\r\n");		//tx "AT+QMTPUB=0,0,0,0,TOPICO_APP"
		Modem_Rta_Cmd(5000,">",ST_MOD_PUBLIC_DAT,ST_MOD_CONN_TOPIC);
		GPIO_PinWrite(GPIOE,0,0);
	break;

	case ST_MOD_PUBLIC_DAT:
		if(mililitros_alcohol > 85 ){
			volumen = 0;
		}
		if(mililitros_alcohol < 85 && mililitros_alcohol > 77){
			volumen = 100;
		}
		if(mililitros_alcohol < 77 && mililitros_alcohol > 60){
			volumen = 150;
		}
		if(mililitros_alcohol < 60 && mililitros_alcohol > 66){
			volumen = 200;
		}
		if(mililitros_alcohol < 66 && mililitros_alcohol > 62){
			volumen = 250;
		}
		if(mililitros_alcohol < 62 && mililitros_alcohol > 55){
			volumen = 300;
		}
		if(mililitros_alcohol < 55 && mililitros_alcohol > 49){
			volumen = 400;
		}
		if(mililitros_alcohol < 49){
			volumen = 923.8095-9.524*mililitros_alcohol;
		}
		if(sensor_temperatura >  10 ){
		    if(sensor_temperatura > 79.0){
		    	GPIO_PinWrite(GPIOA,13,0);
		    }
		    if(sensor_temperatura < 77.0){
		    	GPIO_PinWrite(GPIOA,13,1);
		    }

		    temperatura_grados_enviar = sensor_temperatura;
		}



		GPIO_PinWrite(GPIOB,3,0);
		printf("%d,%d,%d,%d,%d,%d,%0.1f,%0.1f,%0.1f,%0.1f\r\n",horas,minutos,segundos,horas_destilacion,minutos_destilacion,segundos_destilacion,temperatura_grados_enviar,volumen,alcohol,total_minutos_destilacion);
		putchar(CNTL_Z);
		GPIO_PinWrite(GPIOE,0,1);
		Modem_Rta_Cmd(500,"ESPERAR...",ST_ESPERAR_PARA_ENVIAR_DATOS,ST_ESPERAR_PARA_ENVIAR_DATOS);

	break;


	case ST_ESPERAR_PARA_ENVIAR_DATOS:
		GPIO_PinWrite(GPIOB,3,1);
		Modem_Send_Cmd("AT+QMTSUB=0,1,\"mensajes\",1\r\n");		//rx "
		Modem_Rta_Cmd(100,"esperarar...",ST_ESPERAR_RECIBIR_DATOS,ST_ESPERAR_RECIBIR_DATOS);
	break;

	case ST_ESPERAR_RECIBIR_DATOS:
		GPIO_PinWrite(GPIOB,3,1);
		Modem_Rta_Cmd(10000,"PRECAUCION",ST_MENSAJE_RECIBIDO_MQTT_1,ST_ENVIAR_ALERTA);
	break;

	case ST_ENVIAR_ALERTA:
		if(total_minutos_destilacion > 5 && total_minutos_destilacion < 6){
			Modem_Send_Cmd("AT+CMGS=\"3052646735\"\r\n");		//tx "AT+QMTPUB=0,0,0,0,TOPICO_APP"
			Modem_Rta_Cmd(2000,">",ST_MENSAJE_DESTILACION_15M,ST_MOD_CONN_PUB);
		}
		if(total_minutos_destilacion > 10 && total_minutos_destilacion < 11){
			Modem_Send_Cmd("AT+CMGS=\"3052646735\"\r\n");		//tx "AT+QMTPUB=0,0,0,0,TOPICO_APP"
			Modem_Rta_Cmd(2000,">",ST_MENSAJE_DESTILACION_30M,ST_MOD_CONN_PUB);
		}
		if(total_minutos_destilacion > 15 && total_minutos_destilacion < 16){
			Modem_Send_Cmd("AT+CMGS=\"3052646735\"\r\n");		//tx "AT+QMTPUB=0,0,0,0,TOPICO_APP"
			Modem_Rta_Cmd(2000,">",ST_MENSAJE_DESTILACION_45M,ST_MOD_CONN_PUB);
		}
		if(total_minutos_destilacion > 20 && total_minutos_destilacion < 21){
			Modem_Send_Cmd("AT+CMGS=\"3052646735\"\r\n");		//tx "AT+QMTPUB=0,0,0,0,TOPICO_APP"
			Modem_Rta_Cmd(2000,">",ST_MENSAJE_DESTILACION_60M,ST_MOD_CONN_PUB);
		}

		if(sensor_temperatura > 90){
			Modem_Send_Cmd("AT+CMGS=\"3052646735\"\r\n");		//tx "AT+QMTPUB=0,0,0,0,TOPICO_APP"
			Modem_Rta_Cmd(2000,">",ST_MENSAJE_TEXTO,ST_ESPERAR_PARA_ENVIAR_DATOS);
		}else{
			Modem_Rta_Cmd(3000,"ESPERAR..",ST_MOD_CONN_PUB,ST_MOD_CONN_PUB);
		}
		GPIO_PinWrite(GPIOB,3,0);
		GPIO_PinWrite(GPIOE,0,1);
	break;
	case ST_MENSAJE_TEXTO:
		GPIO_PinWrite(GPIOB,3,0);
		printf("ALERTA TEMPERATURA MAYOR 90 GRADOS... \r\n");
		putchar(CNTL_Z);
		GPIO_PinWrite(GPIOE,0,1);
		Modem_Rta_Cmd(1000,"ESPERAR...",ST_MOD_CONN_PUB,ST_MOD_CONN_PUB);
	break;

	case ST_MENSAJE_DESTILACION_15M:
		printf("5 MINUTOS DESTILANDO....TEMPERATURA %0.2f Grados Centigrados \r\n",sensor_temperatura);
		putchar(CNTL_Z);
		GPIO_PinWrite(GPIOE,0,1);
		Modem_Rta_Cmd(1000,"ESPERAR...",ST_MOD_CONN_PUB,ST_MOD_CONN_PUB);
	break;
	case ST_MENSAJE_DESTILACION_30M:
		printf("10 MINUTOS DESTILANDO-->TEMPERATURA %0.2f Grados Centigrados\r\n",sensor_temperatura);
		putchar(CNTL_Z);
		GPIO_PinWrite(GPIOE,0,1);
		Modem_Rta_Cmd(1000,"ESPERAR...",ST_MOD_CONN_PUB,ST_MOD_CONN_PUB);
	break;

	case ST_MENSAJE_DESTILACION_45M:
		printf("15 MINUTOS DESTILANDO.....TEMPERATURA %0.2f Grados Centigrados\r\n",sensor_temperatura);
		putchar(CNTL_Z);
		GPIO_PinWrite(GPIOE,0,1);
		Modem_Rta_Cmd(1000,"ESPERAR...",ST_MOD_CONN_PUB,ST_MOD_CONN_PUB);
	break;

	case ST_MENSAJE_DESTILACION_60M:
		printf("20 MINUTOS DESTILANDo...TEMPERATURA %0.2f Grados Centigrados\r\n",sensor_temperatura);
		putchar(CNTL_Z);
		GPIO_PinWrite(GPIOE,0,1);
		Modem_Rta_Cmd(1000,"ESPERAR...",ST_MOD_CONN_PUB,ST_MOD_CONN_PUB);
	break;


	case ST_MENSAJE_RECIBIDO_MQTT_1:
		if(temperatura_grados_enviar > 84){
			printf("ACCION  DETECTADO\r\n");
			Modem_Send_Cmd("AT+CMGS=\"3052646735\"\r\n");
			Modem_Rta_Cmd(3000,">",ST_MENSAJE_ALERTA_MQQTT,ST_MOD_CONN_PUB);
		}
		if(temperatura_grados_enviar < 84){
			Modem_Rta_Cmd(1000,"eperar..",ST_MOD_CONN_PUB,ST_MOD_CONN_PUB);
		}
		GPIO_PinWrite(GPIOE,0,1);
	break;
	case ST_MENSAJE_ALERTA_MQQTT:
		printf("PRECAUCION TEMPERATURA PROXIMA MAYOR A 85 GRADOS\r\n");
		putchar(CNTL_Z);
		GPIO_PinWrite(GPIOE,0,1);
		Modem_Rta_Cmd(1000,"ESPERAR...",ST_MOD_CONN_PUB,ST_MOD_CONN_PUB);
	break;

	}

 }

////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////////////////////

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

/*******************************************************************************
 * Public Source Code
 ******************************************************************************/
