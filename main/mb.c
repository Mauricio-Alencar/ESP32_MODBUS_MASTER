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

#include "mb_config.h"
#include "mb_crc.h"
#include "mb_timer.h"
#include "mb_serial.h"
#include "mb_event.h"

/* ----------------------- Variables----------------------------------------*/

struct SMBException
{
  UCHAR MBFunctionCode;
  UCHAR MBExceptionStatus;
  UCHAR MBExceptionCode;
} sMBException;
    
    
#ifdef MB_MASTER

/* ######################################################################## */
/* MBMasterEvent: Função chamada sempre que um frame for recebido.
 */
static BOOL 
MBMasterEvent( void  )
{
    BOOL 		Retorno = TRUE;
	USHORT 		usCRC; 
	UCHAR 		ucHigh, ucLow;
    UCHAR       ucSlaveAddress;

	MBTimerOff();
	MBUartRXDisable();

	if(mb_buffer_indice > 0)
	{
        /**
        * Certifica que a mensagem de retorno definitivamente veio dos Slaves cadastrados; 
        * Para vários Slaves comente o if a seguir, pois assim, qualquer SlaveID será tratado;
        */
        //if( (mb_buffer[0] == MB_SLAVE_ADDRESS) || (mb_buffer[0] == MB_SLAVE1_ADDRESS) || (mb_buffer[0] == MB_SLAVE2_ADDRESS) )
		{

			/* MB_FUNCTION01_READ_COILS */	
#if ENABLE_MB_FUNCTION01_READ_COILS >0

			if( mb_buffer[1] == MB_FUNCTION01_READ_COILS )
			{
	         	usCRC = usMBCRC16( mb_buffer, 2 + mb_buffer[2] + 1 );         
	         	ucHigh  = (UCHAR)(usCRC>>8);
	         	ucLow   =  (UCHAR)(usCRC);
	         
	            if( mb_buffer[(2 + mb_buffer[2] + 2)] == ucHigh && mb_buffer[(2 + mb_buffer[2] + 1)] == ucLow)
	            {
                   ucSlaveAddress =  mb_buffer[0]; 
	         	   MBMasterFunc01CallBack( ucSlaveAddress, &mb_buffer[3],  mb_buffer[2] );
		        } else {
		         	//MBException(MBEXCEPTION_01); // Erro de CRC
		         	Retorno = FALSE;
		        }
	        }  
#endif

	        /* MB_FUNCTION03_READ_HOLDING_REGISTERS */
#if ENABLE_MB_FUNCTION03_READ_HOLDING_REGISTERS > 0

	        if( mb_buffer[1] == MB_FUNCTION03_READ_HOLDING_REGISTERS )
	        {
	        	usCRC = usMBCRC16(mb_buffer, 2 + mb_buffer[2] + 1 );         
				ucHigh = (UCHAR)(usCRC>>8);
				ucLow =  (UCHAR)(usCRC);
				if( mb_buffer[(2 + mb_buffer[2] + 2)] == ucHigh && mb_buffer[(2 + mb_buffer[2] + 1)] == ucLow)
				{
                    ucSlaveAddress =  mb_buffer[0]; 
					MBMasterFunc03CallBack( ucSlaveAddress, &mb_buffer[3], mb_buffer[2] );
				} else {
		         	//MBException(MBEXCEPTION_01); // Erro de CRC
		         	Retorno = FALSE;
				}

	        } 
#endif
	        /* MB_FUNCTION06_WRITE_SINGLE_REGISTER */
#if ENABLE_MB_FUNCTION06_WRITE_SINGLE_REGISTER > 0

	        if( mb_buffer[1] == MB_FUNCTION06_WRITE_SINGLE_REGISTER )
	        {
	        	usCRC = usMBCRC16(mb_buffer, 6 );         
				ucHigh = (UCHAR)(usCRC>>8);
				ucLow =  (UCHAR)(usCRC);

				if( mb_buffer[7] == ucHigh && mb_buffer[6] == ucLow)
				{
					USHORT usStartAddress, usValue;

					usStartAddress = (mb_buffer[2] << 8) | mb_buffer[3];
					usValue = (mb_buffer[4] << 8) | mb_buffer[5];

                    ucSlaveAddress =  mb_buffer[0]; 
				    MBMasterFunc06CallBack( ucSlaveAddress, usStartAddress, usValue );

				} else {
		         	//MBException(MBEXCEPTION_01); // Erro de CRC
		         	Retorno = FALSE;
				}

	        } 
#endif
	         /* MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS */
#if ENABLE_MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS > 0

	        if( mb_buffer[1] == MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS )
	        {
	        	usCRC = usMBCRC16(mb_buffer, 6 );         
				ucHigh = (UCHAR)(usCRC>>8);
				ucLow =  (UCHAR)(usCRC);

				if( mb_buffer[7] == ucHigh && mb_buffer[6] == ucLow)
				{
					USHORT usStartAddress, usValue;

					usStartAddress = (mb_buffer[2] << 8) | mb_buffer[3];
					usValue = (mb_buffer[4] << 8) | mb_buffer[5];

                    ucSlaveAddress =  mb_buffer[0]; 
				    //MBMasterFunc16CallBack( ucSlaveAddress, usStartAddress, usValue );      
                    MBMasterFunc06CallBack( ucSlaveAddress, usStartAddress, usValue );                    

				} else {
		         	//MBException(MBEXCEPTION_01); // Erro de CRC
		         	Retorno = FALSE;
				}

	        }
#endif
#if ENABLE_MB_EXCEPTIONS > 0

                switch( mb_buffer[1] )
                {
                    case 0x81:
                    case 0x82:
                    case 0x83:
                    case 0x84:
                    case 0x85:
                    case 0x86:
                    case 0x8F:
                    case 0x90:
                                usCRC = usMBCRC16(mb_buffer, 3 );
                                ucHigh = (UCHAR)(usCRC>>8);
                                ucLow =  (UCHAR)(usCRC);
                                if( mb_buffer[4] == ucHigh && mb_buffer[3] == ucLow)
                                {
                                   MBException( mb_buffer[1] );
                                }
                                break;
                    
                }
#endif
	    }
	}

	mb_buffer_indice = 0;
	MBUartRXEnable();

	return Retorno;
}

/* ######################################################################## */
/* MB_FUNCTION01_READ_COILS:
 * Função Modbus Master responsável pela solicitação de leitura dos Coils
 * MB Master -> 0x01-Read Coils -> MB Slave
 */
#if ENABLE_MB_FUNCTION01_READ_COILS > 0

BOOL 
MBMasterFunc01ReadCoils(  UCHAR  ucSlaveAddress, 	//MB_Slave Address
                          USHORT usStartAddress, 	//Endereço inicial de leitura dos Registradores Holding
                          USHORT usNumberOfCoils) 	//Número de registradores a serem lidos
{
        UCHAR i;
        UCHAR frame[8];
        USHORT usCRC;

        MBUartRXDisable();							// Desabilita a recepção Serial
        											// e habilita modo de transmissão do RS485
        
        frame[0] = ucSlaveAddress;                  // Endereço do Modbus Slave
        frame[1] = MB_FUNCTION01_READ_COILS;    	// Function Code 0x01 
        frame[2] = (UCHAR)(usStartAddress >> 8);    // Starting Address High
        frame[3] = (UCHAR)usStartAddress;           // Starting Address Low
        frame[4] = (UCHAR)(usNumberOfCoils >> 8);   // Quantidade de Registradores High
        frame[5] = (UCHAR)usNumberOfCoils;          // Quantidade de Registradores Low
        usCRC = usMBCRC16(frame, 6);         		// Calcula o valor do CRC16 do frame[0] a [5]
        frame[6] = (UCHAR)(usCRC & 0xFF);    		// Armazena a parte baixa do CRC
        frame[7] = (UCHAR)((usCRC >> 8) & 0xFF); 	// Armazena a parte alta do CRC

        for(i = 0; i < sizeof(frame); ++i)		    // Envia pela Uart o frame de leitura dos Coils
        {
            MBUartRxSend( frame[i] );
        }

        MBUartRXEnable();							// Habilita a recepção Serial
        											// e habilita o modo de recepção do RS485

        return TRUE;
}

#endif 

/* ######################################################################## */
/* MB_FUNCTION03_READ_HOLDING_REGISTERS:
 * Função Modbus Master responsável pela solicitação de leitura dos Holding Registers
 * MB Master -> 0x03-Holding -> MB Slave
 */

#if ENABLE_MB_FUNCTION03_READ_HOLDING_REGISTERS > 0

BOOL
MBMasterFunc03ReadHolding(
                            UCHAR ucSlaveAddress,    // Endereço do Modbus Slave
                            USHORT usStartAddress,   //Endereço inicial de leitura dos Registradores Holding
                            USHORT usNumberOfPoints) //Número de registradores a serem lidos
{
        USHORT usCRC;
        UCHAR i;
        UCHAR frame[8];

        MBUartRXDisable();

        frame[0] = ucSlaveAddress;                
        frame[1] = MB_FUNCTION03_READ_HOLDING_REGISTERS;                    
        frame[2] = (UCHAR)(usStartAddress >> 8);  
        frame[3] = (UCHAR)usStartAddress;         
        frame[4] = (UCHAR)(usNumberOfPoints >> 8);
        frame[5] = (UCHAR)usNumberOfPoints;       
        usCRC = usMBCRC16(frame, 6);         
        frame[6] = (UCHAR)(usCRC & 0xFF);    
        frame[7] = (UCHAR)((usCRC >> 8) & 0xFF); 

        for(i = 0; i < sizeof(frame); ++i)		    // Envia pela Uart o frame de leitura dos Coils
        {
            MBUartRxSend( frame[i] );
        }

		MBUartRXEnable();

        return TRUE;
}

#endif

/* ######################################################################## */
/* MB_FUNCTION06_WRITE_SINGLE_REGISTER:
 * Função Modbus Master responsável pela solicitação de leitura dos Holding Registers
 * MB Master -> 0x06-WRITE_SINGLE_REGISTER -> MB Slave
 */

#if ENABLE_MB_FUNCTION06_WRITE_SINGLE_REGISTER > 0

BOOL 
MBMasterFunc06WriteSingleRegister(
                            UCHAR  ucSlaveAddress,  // Endereço do Modbus Slave
                            USHORT usStartAddress,  // Endereço inicial de escrita do Registrador Holding
                            USHORT usWriteRegister) // Valor a ser gravado no Slave
{
        USHORT usCRC;
        UCHAR i;
        UCHAR frame[8];

        MBUartRXDisable();

        frame[0] = ucSlaveAddress;                
        frame[1] = MB_FUNCTION06_WRITE_SINGLE_REGISTER;                   
        frame[2] = (UCHAR)(usStartAddress >> 8);  
        frame[3] = (UCHAR)usStartAddress;         
        frame[4] = (UCHAR)(usWriteRegister >> 8);
        frame[5] = (UCHAR)usWriteRegister;      
        usCRC = usMBCRC16(frame, 6);         
        frame[6] = (UCHAR)(usCRC & 0xFF);    
        frame[7] = (UCHAR)((usCRC >> 8) & 0xFF); 

        for(i = 0; i < sizeof(frame); ++i)		   
        {
            MBUartRxSend( frame[i] );
        }

		MBUartRXEnable();

        return TRUE;
}

#endif

/* ######################################################################## */
/* MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS:
 * Função Modbus Master responsável pela solicitação de leitura dos Holding Registers
 * MB Master -> 0x16-WRITE_MULTIPLE_REGISTER -> MB Slave
 */

#if ENABLE_MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS > 0

BOOL
MBMasterFunc16WriteMultipleRegisters(
                            UCHAR 	ucSlaveAddress, 
                            USHORT 	usStartAddress, 
                            UCHAR * usWriteRegister,      
                            UCHAR   ucNumberOfPoints) 
{
        USHORT usCRC;
        UCHAR i = 0, t = 0, y = 0;
        UCHAR frame[MB_BUFFER_SIZE];
        USHORT ucNumberOfBytes;

        if(ucNumberOfPoints == 0) return FALSE;

        MBUartRXDisable();

        ucNumberOfBytes =ucNumberOfPoints*2;
           
        frame[0] = ucSlaveAddress;               
        frame[1] = MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS;                    // Function Code do Modbus
        frame[2] = (UCHAR)(usStartAddress >> 8);  
        frame[3] = (UCHAR)usStartAddress;         
        frame[4] = (UCHAR)(ucNumberOfPoints >> 8);
        frame[5] = (UCHAR)ucNumberOfPoints;       
        frame[6] = (UCHAR)ucNumberOfBytes;        
       
         while(ucNumberOfPoints--)
        {
              frame[7 + i++] = (UCHAR)(usWriteRegister[t++ +1]);
              frame[7 + i++] = (UCHAR)(usWriteRegister[t++ -1]);
        }
       
        usCRC = usMBCRC16(frame, 7 + i);         // Calcula o valor do CRC do frame[0] até frame[n]
        frame[7 + i++] = (UCHAR)(usCRC & 0xFF);
        frame[7 + i++] = (UCHAR)((usCRC >> 8) & 0xFF);

        for(y = 0; y < 7 + i ; ++y)
        {
            MBUartRxSend( frame[y] );
        }

        MBUartRXEnable();

        return TRUE;
}

#endif

#endif //MB_MASTER

/* ######################################################################## */
/* MBLoop:
 * Função responsável em analisar se um frame foi completamente recebido.
 * Caso sim, chama a função para o processamento do frame.
 */


BOOL 
MB( void )
{
     BOOL EventStatus = FALSE;
     if( MBEventGet() )
     {   
        MBMasterEvent(  );
        
        MBEventPost(FALSE);
        EventStatus = TRUE;
     }
     return EventStatus;
}


void 
MBInit( void )
{
    MBUartInit();
    MBUartIE();
    MBTimerInit();
}


#if ( ENABLE_MB_FUNCTION01_READ_COILS > 0 && defined MB_MASTER )
BOOL 
MBReadCoils(  UCHAR  ucSlaveAddress, USHORT usStartAddress, USHORT usNumberOfCoils, USHORT usTimeout ) 
{
    MBEventPost(FALSE);
    MBMasterFunc01ReadCoils(ucSlaveAddress, usStartAddress, usNumberOfCoils);

    do
    {
        __delay1ms();
        if(MB()) 
            return TRUE; 
    }while(usTimeout--);
    return FALSE;
}  
#endif 

#if ( ENABLE_MB_FUNCTION03_READ_HOLDING_REGISTERS > 0 && defined MB_MASTER )
BOOL
MBReadRegisters( UCHAR ucSlaveAddress, USHORT usStartAddress, USHORT usNumberOfPoints, USHORT usTimeout )
{
    MBEventPost(FALSE);
    MBMasterFunc03ReadHolding(ucSlaveAddress, usStartAddress, usNumberOfPoints);
    do
    {
        __delay1ms();
        if(MB())
            return TRUE;
    } while(usTimeout--);
    return FALSE;
}

#endif 

#if ( ENABLE_MB_FUNCTION06_WRITE_SINGLE_REGISTER > 0 && defined MB_MASTER )
BOOL
MBWriteSingleRegister( UCHAR  ucSlaveAddress, USHORT usStartAddress, USHORT usWriteRegister, USHORT usTimeout )
{
    MBEventPost(FALSE);
    MBMasterFunc06WriteSingleRegister(ucSlaveAddress, usStartAddress, usWriteRegister);
    do
    {
        __delay1ms();
        if(MB())
            return TRUE;
    } while(usTimeout--);
    return FALSE;
}
#endif 

#if ( ENABLE_MB_FUNCTION16_WRITE_MULTIPLE_REGISTERS > 0 && defined MB_MASTER )
BOOL
MBWriteMultipleRegisters( UCHAR  ucSlaveAddress, USHORT usStartAddress, UCHAR * usWriteRegister, UCHAR ucNumberOfbytes, USHORT usTimeout )
{
    MBEventPost(FALSE);
    MBMasterFunc16WriteMultipleRegisters( ucSlaveAddress, usStartAddress, usWriteRegister, ucNumberOfbytes );
    do
    {
        __delay1ms();
        if(MB())
            return TRUE;
    } while(usTimeout--);
    return FALSE;
}

#endif 