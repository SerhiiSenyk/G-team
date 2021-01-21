/*
 * I2C_one_master.h
 *
 * Created: 05.01.2021
 *  Author: Serhii Senyk
 */ 
#ifndef I2C_ONE_MASTER_H_
#define I2C_ONE_MASTER_H_

#define F_CPU 16000000UL///////
//#define F_CPU 7372800UL
#include <avr/io.h>
#include <avr/interrupt.h>

#define ACK 0 // confirmation is active
#define NACK 1             // no confirmation

#define I2C_W 0             // writing request
#define I2C_R 1             // reading request

#define CONTROL 1             //reverse control of parcel execution
#define NO_CONTROL 0             // without confirmation
#define I2C_SPEED 100000UL
#define I2C_BIT_RATE_CALC          (F_CPU/I2C_SPEED-16)/2

#define I2C_BUF_SIZE_OUT  64
#define I2C_BUF_MASK_OUT  (I2C_BUF_SIZE_OUT - 1)
#define I2C_BUF_SIZE_IN   64
#define I2C_BUF_MASK_IN  (I2C_BUF_SIZE_IN - 1)
extern volatile uint8_t  I2C_BufOUT[I2C_BUF_SIZE_OUT], I2C_StartBufOUT, I2C_EndBufOUT;
extern volatile uint8_t  I2C_BufIN[I2C_BUF_SIZE_IN], I2C_StartBufIN, I2C_EndBufIN;
extern volatile uint8_t I2C_task;   //counter start-stop packets
extern volatile uint8_t  I2C_status; //0-NACKer, 1-BufOUTer, 2-BufINer, 5-Proccess, 6-Control
extern volatile uint8_t I2C_error; // 0 - OK, 1 - error
#define  I2C_NACK_ERROR  0    // slave not response (NACK)
#define  I2C_BufOUT_ERROR 1    // buffer OUT is full
#define  I2C_BufIN_ERROR 2    // buffer IN full
#define  I2C_Proccess 5
#define  I2C_Control 6
//For ATmega328P pinout:
#define I2C_DDR DDRC
#define I2C_PORT PORTC
#define SCL_PIN PORTC5
#define SDA_PIN PORTC4

void I2C_Init();
void write_I2C_BufOUT(uint8_t value);
void write_I2C_BufIN(uint8_t value);
uint8_t read_I2C_BufOUT();
uint8_t read_I2C_BufIN();
void I2C_SearchStartStop();
void I2C_Start(uint8_t control);
void I2C_ReStart(void);
void I2C_Address(uint8_t address, uint8_t direction);
void I2C_SendByte(uint8_t byte);
void I2C_ReadByte(uint8_t ack);
void I2C_Stop();
void I2C_Action();
ISR(TWI_vect);

#endif
