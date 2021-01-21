/*
 * USART.h
 *
 * Created: 05.01.2021
 *  Author: Serhii Senyk
 */ 


#ifndef USART_H_
#define USART_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

#define BAUD_RS_232 9600UL
#define UBRR_CALC_RS_232 (F_CPU/(BAUD_RS_232*16L) - 1)
#define BUF_SIZE_RX 64
#define BUF_SIZE_TX 64
#define BUF_MASK_RX (BUF_SIZE_RX - 1)
#define BUF_MASK_TX (BUF_SIZE_TX - 1)

volatile int8_t bufRX[BUF_SIZE_RX];
volatile int8_t bufTX[BUF_SIZE_RX];
volatile uint8_t rxIn, rxOut, txIn, txOut;

ISR(USART_RXC_vect);
ISR(USART_UDRE_vect);
void USART_Setup();
uint8_t readBufRX(void);
void writeBufTX(uint8_t value);
void print(char *str);

#endif /* USART_H_ */