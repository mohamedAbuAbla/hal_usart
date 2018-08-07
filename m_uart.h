/*
 * m_uart.h
 *
 * Created: 8/7/2018 12:37:09 PM
 *  Author: Eng. Mohamed
 */ 


#ifndef M_UART_H_
#define M_UART_H_

#include <avr/interrupt.h>

#define _UART_SETTINGS_VIEW 0	// 1 for Setting View Enable, 0 for Setting View Disable

#define _NO_OF_DATA_BITS 8	// 8 for EIGHT Bits , 9 for NINE Bits
#define _PARITY	0		// 0 For EVEN , 1 for odd
#define _NO_OF_STOP_BITS 1	// 1 For ONE Stop bit , 2 for TWO stop bits
#define _DOUBLE_SPEED 0		// 1 For DUBLE SPEED ENABLE, 0 For DOUBLE SPEED DISABLE
#define _UART_RECIEVE_INTERRUPT 1	// 1 For Enable, 0 For Disable
#define _UART_TRANSMIT_INTERRUPT 1	// 1 For Enable, 0 For Disable 


typedef enum
{	
	_NO_ERRORS = 1 ,
	_INVALED_PARAMETERS = -10 ,
	_NULL_CB_FUNCTION = -9 ,
	_NULL_PTR = -8 ,
	_FUNCTION_BLOCKED = -7 ,

}RETURN_ERROR_t;


 /* Init the USART */
void USART_Init(uint16_t ubrr);  

 /* when sending "send_data" the "send_cb" will be called */
uint8_t USART_Transmit_Dispatch(uint8_t send_data, void (*send_cb)());

 /* Code will wait until Transmit Start */
void USART_Must_Transmit( uint8_t data );

 /* Call Once, after complete "send_complete_cb" will be called, you can't call again until complete */
uint8_t USART_Interrupted_Transmit_Array(uint8_t* data_start_address,uint16_t no_of_bytes, void (*send_complete_cb)()); 

 /* If New Data Received it will be written to "receive_data" and "receive_cb" will be called */
uint8_t USART_Receive_Dispatch(uint8_t* received_data, void (*receive_cb)());

 /* Call Once, Any Received Data will be written to "receive_data" and "receive_cb" will be called, Call again to update Arguments */
uint8_t USART_Interrupted_Receive(uint8_t* received_data, void (*receive_cb)());

 /* Code will wait until New Char Received */
uint8_t USART_Must_Receive( void );



#endif /* M_UART_H_ */
