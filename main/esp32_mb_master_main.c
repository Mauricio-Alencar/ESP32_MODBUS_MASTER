/*
 * Autor: Maurício Alencar
 * Exemplo de codigo main para uso da biblioteca Modbus RTU MASTER
 * Site: coldpack.com.br
 *
 * Biblioteca Modbus RTU: MASTER
 * Copyright (C) 2019 Coldpack Mechatronics Systems <mauricioalencarf@gmail.com.br>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
*/


/******************************* LIBRARYS *****************************/
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


/******************************* DEFINES *****************************/
static const char* TAG = "MB_MASTER_MAIN";

#define DEBUG_ESP32     TRUE

#define MB_SLAVE1_ADDRESS                   ( 0X01 )
#define MB_SLAVE10_ADDRESS                  ( 0X0A )
#define MB_SLAVE11_ADDRESS                  ( 0X0B )

/******************************* PROTOTIPOS *****************************/
void mb_task(void *pvParameters); 

/******************************* CALLBACK FUNCTIONS MODBUS *****************************/
// ----------------------------------------------- CALL BACK FUNC CODE 01 ---------------------------------------------
/**
 * @brief      { Função 01 Modbus: Read Coils }
 *
 * @param[in]  ucSlaveAddress   O endereço do slave;
 * @param      ucBufferCoils    Vetor de bytes;
 * @param[in]  usNumberOfCoils  Número de coils;
 *
 * @return     { verdadeiro }
 */

/*
BOOL MBEventReadCoils( UCHAR ucSlaveAddress, UCHAR * ucBufferCoils, USHORT usNumberOfCoils ) {
   
    CHAR statebit; 
   
    if( ucSlaveAddress == MB_SLAVE0_ADDRESS )
    {     
        if(usNumberOfCoils >= 1)
        {
        	//Verifica se o primeiro bit de ucBufferCoils[0] é verdadeiro ou falso;
        	 
           statebit = ((ucBufferCoils[0] & (1<<0)) == 1);
           if (DEBUG_ESP32) ESP_LOGI(TAG,"valor do coil 0: %d", statebit);
        }
    } 
    return TRUE;
}
*/

// ----------------------------------------------- CALL BACK FUNC CODE 03 ---------------------------------------------
/**
 * @brief      { Function 03 Modbus RTU RS485 Master }
 *
 * @param[in]  ucSlaveAddress    The uc slave address
 * @param      ucRegHolding      The uc register holding
 * @param[in]  usNumberOfRegisters  The number of Registradores
 *
 * @return     { description_of_the_return_value }
 */
/*
BOOL MBEventReadRegisters( UCHAR ucSlaveAddress,UCHAR * ucRegHolding, USHORT  usNumberOfRegisters )
{
    static USHORT value;

    if( ucSlaveAddress == MB_SLAVE0_ADDRESS )
    { 
        for(USHORT i = 0 ; i < usNumberOfRegisters; ++i)
        {
             if(i == 0)
             {
                value = (ucRegHolding[0] << 8) | ucRegHolding[1];
                 
                if (DEBUG_ESP32) ESP_LOGI(TAG,"valor do registrador 1: %u", (USHORT) value);  
             }
             if(i == 1)
             {
                value = (ucRegHolding[2] << 8) | ucRegHolding[3];
                if (DEBUG_ESP32) ESP_LOGI(TAG,"valor do registrador 2: %u", (USHORT) value);
             }
        }
    }

    return TRUE;
}
*/

// ----------------------------------------------- CALL BACK FUNC CODE 06 ---------------------------------------------
/**
 * @brief      { Função 06 Modbus RTU RS485 Master }
 *
 * @param[in]  ucSlaveAddress  The uc slave address
 * @param[in]  usStartAddress  The start address
 * @param[in]  usValue         The value
 *
 * @return     { true }
 */

/*
BOOL MBEventWriteRegister( UCHAR ucSlaveAddress, USHORT usStartAddress, USHORT usValue )
{
    static USHORT value;

    if( ucSlaveAddress == MB_SLAVE1_ADDRESS )
    {
        // Endereço 3 
        if( 3 == usStartAddress )
        {
            if (DEBUG_ESP32) ESP_LOGI(TAG,"valor do reg3 no slave 1: %u", (USHORT) value);     
        }
    } 
    else if( ucSlaveAddress == MB_SLAVE10_ADDRESS )
    {
        // Endereço 4
        if( 4 == usStartAddress )
        {
            if (DEBUG_ESP32) ESP_LOGI(TAG,"valor do reg4 no slave 10: %u", (USHORT) value); 
        }
    }
     return TRUE;
}
*/

// ----------------------------------------------- CALL BACK FUNC CODE 16 ---------------------------------------------
/**
 * @brief      { Função 16 Modbus RTU RS485 Master }
 *
 * @param[in]  ucSlaveAddress  The uc slave address
 * @param[in]  usStartAddress  The start address
 * @param[in]  usValue         The value
 *
 * @return     { true }
 */
BOOL MBEventWriteRegister( UCHAR ucSlaveAddress, USHORT usStartAddress, USHORT usValue )
{
     return TRUE;
}

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
    
    // ------------------------- EXEMPLO PARA LER BOBINA (FUNC 01) --------------------------
    //descomentar abaixo para poder utilizar
     /**
     * Realiza a leitura de 2 Coils a partir do endereço 0x01 do slave 0x01;
     * Aguarda o retorno da mensagem (enviada do Slave para o Master) durante 100ms;
     * O Timeout deverá ser maior que 60ms sempre para 9600bps (tempo de resposta do slave no barramento);
     *
    if( !MBReadCoils(MB_SLAVE1_ADDRESS, 0x01, 2, (TIMEOUT_1MS*100)) )
    {
        //timeout...
        if (DEBUG_ESP32) ESP_LOGI(TAG,"timeout de req para slave");
    } */




    // ------------------------- EXEMPLO PARA LER REGISTRADOR (FUNC 03) --------------------------
    //descomentar abaixo para poder utilizar
    /**
      * Realiza a leitura de 2 registradores (4 bytes) a partir do endereço 1 do
      * slave MB_SLAVE1_ADDRESS. Os dados precisam ser recebidos em até 100ms;
      *
    if( !MBReadRegisters( MB_SLAVE1_ADDRESS, 1, 2, (TIMEOUT_1MS*100)  ))
    {
        //Timeout...
        if (DEBUG_ESP32) ESP_LOGI(TAG,"timeout de req para slave");
    }*/
    
    

    // ----------------------- EXEMPLO PARA ESCREVER UNICO REGISTRADOR (FUNC 06) -----------------------
    //descomentar abaixo para poder utilizar 
    /*
    static USHORT counter = 0;
    
    //Realiza a escrita de 1 registradores (2 bytes) a partir do endereço 3 do
    //slave MB_SLAVE1_ADDRESS. Os dados precisam ser recebidos em até 100ms;
    
    if(!MBWriteSingleRegister( MB_SLAVE1_ADDRESS, 3, counter++, (TIMEOUT_1MS*100) ))
    {
        //timeout...
        if (DEBUG_ESP32) ESP_LOGI(TAG,"timeout de req para slave");
    }

    
    //Realiza a escrita de 1 registradores (2 bytes) a partir do endereço 4 do
    //slave MB_SLAVE10_ADDRESS. Os dados precisam ser recebidos em até 100ms;
    
    if(!MBWriteSingleRegister( MB_SLAVE10_ADDRESS, 4, counter, (TIMEOUT_1MS*100) ))
    {
        //timeout...
        if (DEBUG_ESP32) ESP_LOGI(TAG,"timeout de req para slave");
    }*/

    // ------------------- EXEMPLO PARA ESCREVER MULTIPLOS REGISTRADORES (FUNC 16) ----------------------- 
    //descomentar abaixo para poder utilizar 
    
    UCHAR frame[6];

    static USHORT counter1 = 1234;
    static USHORT counter2 = 4321;
    static USHORT counter3 = 9876;
    
    static USHORT counter = 0;

    frame[0] = (UCHAR) counter1;
    frame[1] = (UCHAR) (counter1>>8);
    
    frame[2] = (UCHAR) counter2;
    frame[3] = (UCHAR) (counter2>>8);
    
    frame[4] = (UCHAR) counter3;
    frame[5] = (UCHAR) (counter3>>8);

        //Endereço do Slave, Endereço inicial (offset), buffer, número de registradores, timeout;
    if( !MBWriteMultipleRegisters( MB_SLAVE10_ADDRESS, 5, frame, 3, (TIMEOUT_1MS*100)  ))
    {
        //timeout...
        if (DEBUG_ESP32) ESP_LOGI(TAG,"timeout de req para slave");
    }  

    // ----------------------------- DELAY PARA CADA REQ AO SLAVE -------------------------
    vTaskDelay(5000 / portTICK_PERIOD_MS);

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

BOOL MBEventReadCoils( UCHAR ucSlaveAddress, UCHAR * ucBufferCoils, USHORT usNumberOfCoils ) {
     return TRUE;
}


BOOL MBEventReadRegisters( UCHAR ucSlaveAddress,UCHAR * ucRegHolding, USHORT  usNumberOfPoints ) {
   return TRUE;  
}

/*
BOOL MBEventWriteRegister( UCHAR ucSlaveAddress, USHORT usStartAddress, USHORT usValue ) {
   return TRUE;
}*/