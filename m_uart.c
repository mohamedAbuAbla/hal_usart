/*
 * m_uart.c
 *
 * Created: 8/7/2018 12:36:49 PM
 *  Author: Eng. Mohamed
 */ 

#define NULL ((void*)0)
#include <avr/io.h>
#include <avr/interrupt.h>
#include "m_uart.h"

static struct
{
	uint8_t* receive_buffer_ptr;
	void (*ISR_Receive_cb)() ;
	
}Interrupted_Receive_Struct;


static struct 
{
	uint16_t send_array_length ;
	uint16_t send_array_index ;
	uint8_t block;
	uint8_t* send_buffer_ptr;
	void (*ISR_Send_cb)();
	
}Interrupted_Transmit_Array_Struct;


void USART_Init( uint16_t ubrr)
{	
	Interrupted_Transmit_Array_Struct.block=0;
	/* Set baud rate */
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	/* Enable receiver and transmitter */
	UCSRB |= (1<<RXEN)|(1<<TXEN);

	#if (_UART_RECIEVE_INTERRUPT == 1)
		/* Enable Double Speed */
		UCSRB |= (1<<RXCIE);
		sei();
			#if (_UART_SETTINGS_VIEW ==1)
				# warning "UART RECIEVE COMPLETE INTERRUPT ENABLED"
			#endif
	
	#endif

	#if (_UART_TRANSMIT_INTERRUPT == 1)
	/* Enable Double Speed */
	UCSRB |= (1<<UDRIE);
	sei();
		#if (_UART_SETTINGS_VIEW ==1)
			# warning "UART TRANSMIT INTERRUPT ENABLED"
		#endif
	
	#endif
	
	#if (_DOUBLE_SPEED == 1)
		/* Enable Double Speed */
		UCSRA |= (1<<U2X);
		
		#if (_UART_SETTINGS_VIEW ==1)
			# warning "UART DOUBLE SPEED ENABLED"
		#endif
		
	#endif
	
	#if (_DOUBLE_SPEED == 0)
		/* Disable Double Speed */
		UCSRA &= ~(1<<U2X);
		
		#if (_UART_SETTINGS_VIEW == 1)
			# warning "UART DOUBLE SPEED DISABLED"
		#endif
		
	#endif
	
	#if (_NO_OF_DATA_BITS == 8) 
		/* Set frame format: 8 data */
		UCSRC |= (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);
		
		#if (_UART_SETTINGS_VIEW ==1)
			# warning "UART 8 DATA BITS SET"
		#endif
		
	#endif	
	
	#if (_NO_OF_DATA_BITS == 9)	
		/* Set frame format: 9 data */
		UCSRC |= (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1)|(1<<UCSZ2);	

		#if (_UART_SETTINGS_VIEW ==1)
			# warning "UART 9 DATA BITS SET"
		#endif

	#endif
	
	#if (_PARITY == 0)	
		/* SET EVEN PARITY */
		UCSRC |= (1<<URSEL)|(1<<UPM1);

		#if (_UART_SETTINGS_VIEW ==1)
			# warning "UART EVEN PARITY SET"
		#endif

	#endif
	
	#if (_PARITY == 1)
		/* SET ODD PARITY */
		UCSRC |=(1<<URSEL)|(1<<UPM1)|(1<<UPM0);

		#if (_UART_SETTINGS_VIEW ==1)
			# warning "UART ODD PARITY SET"
		#endif

	#endif
	
	#if (_NO_OF_STOP_BITS == 1)
		/* SET 1 STOP BITS */
		UCSRC |= (1<<URSEL);
		UCSRC &= ~(1<<USBS);
		
		#if (_UART_SETTINGS_VIEW ==1)
			# warning "UART 1 STOP BITS SET"
		#endif
			
	#endif
	
	#if (_NO_OF_STOP_BITS == 2)
		/* SET 2 STOP BITS */
		UCSRC |= (1<<URSEL)|(1<<USBS);
		
		#if (_UART_SETTINGS_VIEW ==1)
			# warning "UART 2 STOP BITS SET"
		#endif
		
	#endif
	
	/* INVALID UART SETTINGS */
	#if (((_NO_OF_DATA_BITS !=9 )&&(_NO_OF_DATA_BITS !=8 ))|((_PARITY !=0)&&(_PARITY!=1))|((_NO_OF_STOP_BITS!=1)&&(_NO_OF_STOP_BITS!=2))|((_DOUBLE_SPEED!=1)&&(_DOUBLE_SPEED!=0))|((_UART_TRANSMIT_INTERRUPT!=1)&&(_UART_TRANSMIT_INTERRUPT!=0))|((_UART_RECIEVE_INTERRUPT!=1)&&(_UART_RECIEVE_INTERRUPT!=0)))
		#error "INVALID UART SETTINGS"
	#endif
}


void USART_Must_Transmit( uint8_t data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) );
	/* Put data into buffer, sends the data */
	UDR = data;
}

uint8_t USART_Must_Receive( void )
{
	while ( !(UCSRA & (1 << RXC)) ); //Wait for the RXC to not have 0
	return UDR; //Get that data outa there and back to the main program!
}


uint8_t USART_Transmit_Dispatch(uint8_t send_data, void (*send_cb)())
{
	RETURN_ERROR_t error = _NO_ERRORS ;
	if (( UCSRA & (1<<UDRE))) // If Transmit Buffer is Empty
	{
		//* Put send_data in send buffer /
		
		if (send_cb!=NULL)
		{	
			UDR = send_data;
			send_cb();
		}
		else
		{
			error = _NULL_CB_FUNCTION;
		}
	}
	return error;
}


uint8_t USART_Receive_Dispatch(uint8_t* received_data, void (*receive_cb)())
{
	RETURN_ERROR_t error = _NO_ERRORS ;
	if ((UCSRA & (1 << RXC))) // If Receive Flag is Set
	{
		//* Put received data in received_data /
		
		if ((receive_cb != NULL)&&(received_data != NULL))
		{	
			(*received_data) = UDR ;
			receive_cb();
		}
		else
		{
			if (received_data == NULL)
			{
				error = _NULL_PTR;
			}
			else
			{
				error = _NULL_CB_FUNCTION;
			}
		}
	}
	return error;
}

#if (_UART_RECIEVE_INTERRUPT==1)

	uint8_t USART_Interrupted_Receive(uint8_t* received_data, void (*receive_cb)())
	{
		RETURN_ERROR_t error = _NO_ERRORS ;
		if ((receive_cb != NULL)&&(received_data != NULL))
		{
			Interrupted_Receive_Struct.receive_buffer_ptr = received_data ;
			Interrupted_Receive_Struct.ISR_Receive_cb = receive_cb;
		}
		else
		{
			if (received_data == NULL)
			{
				error = _NULL_PTR;
			}
			else
			{
				error = _NULL_CB_FUNCTION;
			}
		}
		return error;
	}

	ISR(USART_RXC_vect)
	{
		(*(Interrupted_Receive_Struct.receive_buffer_ptr))=UDR;
		Interrupted_Receive_Struct.ISR_Receive_cb();
	}

#endif
#if (_UART_TRANSMIT_INTERRUPT==1)

	uint8_t USART_Interrupted_Transmit_Array(uint8_t* data_start_address,uint16_t no_of_bytes, void (*send_complete_cb)())
	{
		RETURN_ERROR_t error = _NO_ERRORS ;
		if (Interrupted_Transmit_Array_Struct.block == 0)
		{
			if ((send_complete_cb != NULL)&&(data_start_address != NULL))
			{	
				Interrupted_Transmit_Array_Struct.send_array_index = 0;
				Interrupted_Transmit_Array_Struct.send_array_length = no_of_bytes ;
				Interrupted_Transmit_Array_Struct.send_buffer_ptr = data_start_address ;
				Interrupted_Transmit_Array_Struct.ISR_Send_cb = send_complete_cb ;
				Interrupted_Transmit_Array_Struct.block = 1;
				
				if (( UCSRA & (1<<UDRE))) // If Transmit Buffer is Empty
				{
					if (Interrupted_Transmit_Array_Struct.send_array_index <= Interrupted_Transmit_Array_Struct.send_array_length)
					{
						UDR = (*(Interrupted_Transmit_Array_Struct.send_buffer_ptr + Interrupted_Transmit_Array_Struct.send_array_index));
						Interrupted_Transmit_Array_Struct.send_array_index++;
					}
					else
					{
						Interrupted_Transmit_Array_Struct.block = 0;
						Interrupted_Transmit_Array_Struct.ISR_Send_cb() ;
					}
				}
			}
			else
			{
				if (data_start_address == NULL)
				{
					error = _NULL_PTR;
				}
				else
				{
					error = _NULL_CB_FUNCTION;
				}
			}
		}
		else
		{
			error = _FUNCTION_BLOCKED;
		}
		return error;
	}

	ISR(USART_UDRE_vect)
	{
		if (Interrupted_Transmit_Array_Struct.send_array_index <= Interrupted_Transmit_Array_Struct.send_array_length)
		{
			UDR = (*(Interrupted_Transmit_Array_Struct.send_buffer_ptr + Interrupted_Transmit_Array_Struct.send_array_index));
			Interrupted_Transmit_Array_Struct.send_array_index++;
		}
		else
		{	
			Interrupted_Transmit_Array_Struct.block = 0;
			Interrupted_Transmit_Array_Struct.ISR_Send_cb() ;
		}
	}


#endif
