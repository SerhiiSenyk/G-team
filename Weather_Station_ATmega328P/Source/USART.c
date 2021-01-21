/*
 * USART.c
 *
 * Created: 05.01.2021
 *  Author: Serhii Senyk
 */ 

#include "USART.h"

volatile int8_t bufTX[BUF_SIZE_TX];
volatile int8_t bufRX[BUF_SIZE_RX];
volatile uint8_t rxIn = 0, rxOut = 0;//, txIn = 0, txOut = 0;

void USART_Setup()
{
	UCSR0A = 0;
	//interruption of receive, work receiver and transmiter
	UCSR0B = (1 << RXCIE0)|(1 << RXEN0)|(1 << TXEN0);
	//8 bit, no parity, 1 stop bit:
	UCSR0C = (1 << UCSZ01)|(1 << UCSZ00);
	UBRR0H = (uint8_t)(UBRR_CALC_RS_232 >> 8);
	UBRR0L = (uint8_t)(UBRR_CALC_RS_232);
}

ISR(USART_RX_vect)
{
	bufRX[rxIn++] = UDR0;
	rxIn &= BUF_MASK_RX;
}

uint8_t readBufRX(void)
{
	uint8_t value = bufRX[rxOut++];
	rxOut &= BUF_MASK_RX;
	return value;
}

ISR(USART_UDRE_vect)
{
	UDR0 = bufTX[txOut++];
	txOut &= BUF_MASK_TX;
	if(txOut == txIn)
		UCSR0B &= ~(1 << UDRIE0);
}

void writeBufTX(uint8_t value)
{
	bufTX[txIn++] = value;
	txIn &= BUF_MASK_TX;
	UCSR0B |= (1 << UDRIE0);
}

void print(char *str)
{
	uint8_t i = 0;
	while(str[i] != '\0')
		writeBufTX(str[i++]);
}
