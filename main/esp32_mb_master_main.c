#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
//#include "esp_event_loop.h"
#include "esp_event.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "soc/uart_struct.h"
#include "string.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "cJSON.h"

#include "ESP32_ModbusRTU_MASTER.h"

static const char* TAG = "MB_MASTER_MAIN";

#define DEBUG_ESP32     TRUE

#define MB_SLAVE0_ADDRESS  (0X01)

void mb_task(void *pvParameters); 

// ----------------------------------------------- FUNC CODE 01 ---------------------------------------------
/**
 * @brief      { Função 01 Modbus: Read Coils }
 *
 * @param[in]  ucSlaveAddress   O endereço do slave;
 * @param      ucBufferCoils    Vetor de bytes;
 * @param[in]  usNumberOfCoils  Número de coils;
 *
 * @return     { verdadeiro }
 */
BOOL MBEventReadCoils( UCHAR ucSlaveAddress, UCHAR * ucBufferCoils, USHORT usNumberOfCoils ) {
   
    CHAR statebit; 
   
    if( ucSlaveAddress == MB_SLAVE0_ADDRESS )
    {     
        if(usNumberOfCoils >= 1)
        {
        	/**
        	 * Verifica se o primeiro bit de ucBufferCoils[0] é verdadeiro ou falso;
        	 */
           statebit = ((ucBufferCoils[0] & (1<<0)) == 1);
           pinMode( LED, OUTPUT );
           digitalWrite( LED, statebit );  
        }
    } 
    return TRUE;
}

// -------------------------------------------- FIM FUNC CODE 01 --------------------------------------------

// -------------------------------------------- MAIN --------------------------------------------------------
void app_main()
{
	MBInit();
    xTaskCreate(mb_task, "mb_task", 1024*4, NULL, 2, NULL);
} 
// ------------------------------------------- FIM MAIN ------------------------------------------------------

// -------------------------------------------- TAREFAS ------------------------------------------------------
void mb_task(void *pvParameters)
{
    while(1)
    {
        /**
     * Realiza a leitura de 2 Coils a partir do endereço 0x01 do slave 0x0A;
     * Aguarda o retorno da mensagem (enviada do Slave para o Master) durante 83ms;
     * O Timeout deverá ser maior que 60ms;
     */
    if( !MBReadCoils(MB_SLAVE0_ADDRESS, 0x01, 2, (TIMEOUT_1MS*83)) )
    {
        //timeout...
    }
    
    vTaskDelay(200 / portTICK_PERIOD_MS);

    }
}

// ------------------------------------------ FIM TAREFAS ------------------------------------------------------

// -------------------------------------------- EXCEPTION ------------------------------------------------------

void MBException( USHORT usExceptionCode ) {
   return; 
}

// --------------------------------------- FIM EXCEPTION -------------------------------------------------------

/*

 Callback das Funções do Stack Modbus; 
 Para Habilitar ou desabilitar cada uma das funções individualmente, configure
 o arquivo mb_config.h.
 Caso as funções estejam habilitadas, mantenha a função de callback em seu programa e comentar apenas a que estiver usando acima

*/

BOOL MBEventReadRegisters( UCHAR ucSlaveAddress,UCHAR * ucRegHolding, USHORT  usNumberOfPoints ) {
   return TRUE;  
}

BOOL MBEventWriteRegister( UCHAR ucSlaveAddress, USHORT usStartAddress, USHORT usValue ) {
   return TRUE;
}