/*
 * Weather_station_ATmega328P.c
 *
 * Created: 05.01.2021
 * Author : Serhii Senyk
 * Copyright : G-team
 * All right reserved
 * G-team project
 */ 
//Weather_Station_ATmega328P\Debug\Weather_Station_ATmega328P.hex
#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#include "BMP180.h"
#include "USART.h"

void Setup();
//8030
//557
//4832
//320
int main(void)
{
    cli();
	Setup();
	sei();
	int32_t temperature = 0;
	int32_t pressure = 0;
	char buf[20];
	
    while (1) 
    {
			print("Temperature = ");
			temperature = BMP180_read_temperature();
			sprintf(buf, "%d\n", (int16_t)temperature);
			print(buf);
			pressure = BMP180_read_pressure();
			//writeBufTX(pressure >> 24);
			//writeBufTX(pressure >> 16);
			//writeBufTX(pressure >> 8);
			//writeBufTX(pressure);
			
			if(I2C_error){
				print("BMP180 Error!!!\n");
				old = error;
			}
			else{
				print("Pressure = ");
				sprintf(buf, "%d\n", BMP180_Pa_To_Hg(pressure));
				print(buf);
			}
			_delay_ms(1000);
			if(I2C_task)
				print("ERROR");
			
			//_delay_ms(10);
    }
	return 0;
}

void Setup()
{
	I2C_Init();
	USART_Setup();
	BMP180_Init(3);
}
//oss = 3
//68 A4  - clear data TEMPERATURE
//00 00 67 8F - UT

// FF FC E2 E8 - UP
//
//AC1 = 7728
//AC2 = -1026
//AC3 = -14586
//AC4 = 33374
//AC5 = 25691
//AC6 = 18634
//B1 = 6515
//B2 = 34
//MB = -32768
//MC = -11786
//MD = 2399

//ac1 = 7728
//ac2 = -1026
//ac3 = -14586
//ac4 = 33374
//ac5 = 25691
//ac6 = 18634
//b1 = 6515
//b2 = 34
//mb = -32768
//mc = -11786
//md = 2399
