
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

#ifndef __MB_CONFIG_H__
#define __MB_CONFIG_H__

/* ----------------------- Modbus includes ----------------------------------*/
#include "defs.h"


/* ----------------------- Defines ------------------------------------------*/
#define MB_MASTER

#define __ESP32__
#define DEBUG_MB TRUE

/* ----------------------- ENABLE FUNCTIONS-----------------------------------*/   

#if defined(__ESP32__)
    #define UART1
    //#define UART2 

#elif defined(__STM32__) 
    //#define UART0
    //#define UART1 
    #define UART2

#endif

 /* ----------------------- PORT --------------------------------------------*/
//timeout deve ser para 10ms, pois geralmente os ticks de RTOS trabalhando em 100hz.
#define __delay10ms()                 vTaskDelay( 10 / portTICK_PERIOD_MS) 
#define TIMEOUT_10MS                  1

//#define ENTER_CRITICAL_SECTION_MB()   cli() 
//#define EXIT_CRITICAL_SECTION_MB()    sei() 

/* ----------------------- Variables ----------------------------------------*/

#define MB_BUFFER_SIZE 						                      (255)

/* ----------------------- ENABLE FUNCTIONS-----------------------------------*/

#define ENABLE_MB_FUNCTION01_READ_COILS               ( 1 )
#define ENABLE_MB_FUNCTION03_READ_HOLDING_REGISTERS   ( 1 )
#define ENABLE_MB_FUNCTION06_WRITE_SINGLE_REGISTER    ( 1 )
#define ENABLE_MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS ( 1 )
#define ENABLE_MB_EXCEPTIONS                          ( 1 )

/* ----------------------- Definições do Modbus--------------------------------*/
#define MB_FUNCTION01_READ_COILS                      ( 0X01 )
#define MB_FUNCTION03_READ_HOLDING_REGISTERS          ( 0X03 )
#define MB_FUNCTION06_WRITE_SINGLE_REGISTER           ( 0X06 )
#define MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS        ( 0X10 )

/* ----------------------- Exceptions----------------------------------------*/
#define MB_EXCEPTION_ILEGAL_DATA_VALUE                 (0X03)
#define MB_EXCEPTION_ILEGAL_FUNCTION                   (0X01)

/* ----------------------- Interfaces ---------------------------------------*/

#define MBEventReadCoils(x,y,z) 				MBMasterFunc01CallBack(x,y,z) 
#define MBEventReadRegisters(x,y,z) 			MBMasterFunc03CallBack(x,y,z) 
#define MBEventWriteRegister(x,y,z) 	        MBMasterFunc06CallBack(x,y,z)
#define MBEventWriteSingleRegister(x,y,z) 	    MBMasterFunc06CallBack(x,y,z)
#define MBEventWriteMultipleRegisters(x,y,z) 	MBMasterFunc16CallBack(x,y,z)

/* ----------------------- Supporting functions -----------------------------*/
BOOL 	MBReadCoils(  UCHAR  ucSlaveAddress, USHORT usStartAddress, USHORT usNumberOfCoils, USHORT usTimeout ); 
BOOL 	MBReadRegisters( UCHAR ucSlaveAddress, USHORT usStartAddress, USHORT usNumberOfPoints, USHORT usTimeout );
BOOL    MBWriteSingleRegister( UCHAR  ucSlaveAddress, USHORT usStartAddress, USHORT usWriteRegister, USHORT usTimeout );
BOOL 	MBWriteMultipleRegisters( UCHAR  ucSlaveAddress, USHORT usStartAddress, UCHAR * usWriteRegister, UCHAR   ucNumberOfbytes, USHORT usTimeout );

/* ----------------------- Supporting functions -----------------------------*/
void MBInit( void );
BOOL MB( void );
static BOOL MBMasterEvent( void );
void MBException( USHORT usExceptionCode );

/* ----------------------- Supporting functions -----------------------------*/
static BOOL MBMasterFunc01ReadCoils( UCHAR ucSlaveAddress, USHORT usStartAddress, USHORT usNumberOfCoils );
BOOL MBMasterFunc01CallBack( UCHAR ucSlaveAddress, UCHAR * ucCoils, USHORT usNumberOfCoils );

static BOOL MBMasterFunc03ReadHolding( UCHAR ucSlaveAddress, USHORT usStartAddress, USHORT usNumberOfPoints );
BOOL MBMasterFunc03CallBack( UCHAR ucSlaveAddress, UCHAR * ucRegHolding,  USHORT  usNumberOfPoints );

static BOOL MBMasterFunc06WriteSingleRegister( UCHAR  ucSlaveAddress, USHORT usStartAddress, USHORT usWriteRegister);                                       
BOOL MBMasterFunc06CallBack( UCHAR ucSlaveAddress, USHORT usStartAddress, USHORT value );

static BOOL MBMasterFunc16WriteMultipleRegisters(  UCHAR 	ucSlaveAddress, USHORT 	usStartAddress,  UCHAR * usWriteRegister, UCHAR   ucNumberOfbytes);
BOOL MBMasterFunc16CallBack( UCHAR ucSlaveAddress, USHORT usStartAddress, USHORT usValue );
/* -------------------------------------------------------------------------*/
/* ----------------------------------------------------------------------------*/

#endif