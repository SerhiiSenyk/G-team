/*
 * BMP180.h
 *
 * Created: 05.01.2021 23:50:40
 *  Author: Serhii-PC
 */ 


#ifndef BMP180_H_
#define BMP180_H_
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "I2C_one_master.h"
#define BMP180_I2C_ADDRESS 0x77
#define BMP180_CALIB_ADDRESS 0xAA
#define BMP180_COUNT_CALIB_DATA 22
#define BMP180_GET_TEMPERATURE	0x2E
#define BMP180_GET_PRESSURE		0x34
#define BMP180_CONTROL_REGISTER 0xF4
#define BMP180_READ_REGISTER	0xF6
#define BMP180_ULTRA_LOW_POWER			0
#define BMP180_STANDART					1
#define BMP180_HIGH						2
#define BMP180_ULTRA_HIGH_RESOLUTION	3
struct BMP180_calibration_data{
	int16_t AC1;
	int16_t AC2;
	int16_t AC3;
	uint16_t AC4;
	uint16_t AC5;
	uint16_t AC6;
	int16_t B1;
	int16_t B2;
	int16_t MB;
	int16_t MC;
	int16_t MD;
}calibration;
extern uint8_t oversampling_setting;
extern uint8_t conversion_time;
void BMP180_Init(uint8_t mode);
int32_t BMP180_read_temperature(void);
int32_t BMP180_read_pressure(void);
void BMP180_set_oversampling_setting(uint8_t mode);
uint16_t BMP180_Pa_To_Hg (int32_t pressure_In_Pascals);


#endif /* BMP180_H_ */