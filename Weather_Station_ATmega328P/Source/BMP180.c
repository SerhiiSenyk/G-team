/*
 * BMP180.c
 *
 * Created: 05.01.2021
 *  Author: Serhii Senyk
 */ 
#include "BMP180.h"
#include "I2C_one_master.h"
#include "USART.h"

uint8_t oversampling_setting = 0;
uint8_t conversion_time = 5;

void BMP180_Init(uint8_t mode)//read calibration data
{
	//BMP180_set_oversampling_setting(mode);
	oversampling_setting = mode;
	I2C_Start(CONTROL);
	I2C_Address(BMP180_I2C_ADDRESS, I2C_W);
	I2C_SendByte(BMP180_CALIB_ADDRESS);//write
	I2C_ReStart();
	I2C_Address(BMP180_I2C_ADDRESS,  I2C_R);
	for(uint8_t k = 0;k < BMP180_COUNT_CALIB_DATA - 1; ++k){
		I2C_ReadByte(ACK);
	}
	I2C_ReadByte(NACK);
	I2C_Stop();
	while(!I2C_task);
	if(I2C_task > 0)
	{
		--I2C_task;
		calibration.AC1 = ((int16_t)(read_I2C_BufIN() << 8) | read_I2C_BufIN());
		calibration.AC2 = ((int16_t)(read_I2C_BufIN() << 8) | read_I2C_BufIN());
		calibration.AC3 = ((int16_t)(read_I2C_BufIN() << 8) | read_I2C_BufIN());
		calibration.AC4 = ((uint16_t)(read_I2C_BufIN() << 8) | read_I2C_BufIN());
		calibration.AC5 = ((uint16_t)(read_I2C_BufIN() << 8) | read_I2C_BufIN());
		calibration.AC6 = ((uint16_t)(read_I2C_BufIN() << 8) | read_I2C_BufIN());
		calibration.B1 = ((int16_t)(read_I2C_BufIN() << 8) | read_I2C_BufIN());
		calibration.B2 = ((int16_t)(read_I2C_BufIN() << 8) | read_I2C_BufIN());
		calibration.MB = ((int16_t)(read_I2C_BufIN() << 8) | read_I2C_BufIN());
		calibration.MC = ((int16_t)(read_I2C_BufIN() << 8) | read_I2C_BufIN());
		calibration.MD = ((int16_t)(read_I2C_BufIN() << 8) | read_I2C_BufIN());
	}
}


int32_t BMP180_get_ut()//temperature
{
	I2C_Start(CONTROL);
	I2C_Address(BMP180_I2C_ADDRESS, I2C_W);
	I2C_SendByte(BMP180_CONTROL_REGISTER);
	I2C_SendByte(BMP180_GET_TEMPERATURE);
	I2C_Stop();	
	_delay_ms(4.5);
	while(!I2C_task);
	I2C_task = 0;
	I2C_Start(CONTROL);
	I2C_Address(BMP180_I2C_ADDRESS,  I2C_W);
	I2C_SendByte(BMP180_READ_REGISTER);
	I2C_ReStart();
	I2C_Address(BMP180_I2C_ADDRESS,  I2C_R);
	I2C_ReadByte(ACK);
	I2C_ReadByte(NACK);
	I2C_Stop();
	while(!I2C_task);
	int32_t UT = 0;
	if(I2C_task > 0)
	{
		--I2C_task;
		uint8_t MSB = read_I2C_BufIN();
		uint8_t LSB = read_I2C_BufIN();
		UT = (((int32_t)MSB << 8) + LSB);
	}
	return UT;
}

int32_t BMP180_read_temperature()
{
	int32_t UT = BMP180_get_ut();
	int32_t X1 = ((int32_t)(UT - calibration.AC6)*(int32_t)calibration.AC5) >> 15;
	int32_t X2 = ((int32_t)calibration.MC << 11)/(X1 + (int32_t)calibration.MD);
	return ((int32_t)(X1 + X2 + 8) >> 4);
}

int32_t BMP180_get_up()//pressure
{
	I2C_Start(CONTROL);
	I2C_Address(BMP180_I2C_ADDRESS, I2C_W);
	I2C_SendByte(BMP180_CONTROL_REGISTER);
	I2C_SendByte(BMP180_GET_PRESSURE + (oversampling_setting << 6));
	I2C_Stop();
	_delay_ms(26);////////////////////////////////////
	while(!I2C_task);
	I2C_task = 0;
	I2C_Start(CONTROL);
	I2C_Address(BMP180_I2C_ADDRESS,  I2C_W);
	I2C_SendByte(BMP180_READ_REGISTER);
	I2C_ReStart();
	I2C_Address(BMP180_I2C_ADDRESS,  I2C_R);
	I2C_ReadByte(ACK);
	I2C_ReadByte(ACK);
	I2C_ReadByte(NACK);
	I2C_Stop();
	while(!I2C_task);
	int32_t UP = 0;
	if(I2C_task > 0)
	{
		--I2C_task;
		uint8_t MSB = read_I2C_BufIN();
		uint8_t LSB = read_I2C_BufIN();
		uint8_t XLSB = read_I2C_BufIN();
		UP = (((int32_t)MSB << 16) + ((int32_t)LSB << 8) + XLSB) >> (8 - oversampling_setting);
	}
	return UP;
}

//return pressure in Pa
int32_t BMP180_read_pressure(void)
{
	int32_t UT = BMP180_get_ut();//temperature
	int32_t UP = BMP180_get_up();//pressure
	int32_t X1 = ((int32_t)(UT - (int32_t)calibration.AC6)*(int32_t)calibration.AC5) >> 15;
	int32_t X2 = (((int32_t)calibration.MC << 11)/(X1 + (int32_t)calibration.MD)); 
	int32_t B5 = X1 + X2;
	int32_t B6 =  B5 - 4000L;
	X1 = ((int32_t)calibration.B2*(B6*B6 >> 12)) >> 11;
	X2 = ((int32_t)calibration.AC2*B6) >> 11;
	int32_t X3 = X1 + X2;
	int32_t B3 = (((((int32_t)calibration.AC1 << 2) + X3) << oversampling_setting) + 2) >> 2;
	X1 = (int32_t)calibration.AC3*B6 >> 13;
	X2 = ((int32_t)calibration.B1*(B6*B6 >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	uint32_t B4 = ((uint32_t)calibration.AC4*(uint32_t)(X3 + 32768)) >> 15;
	uint32_t B7 = ((uint32_t)UP - B3)*(uint32_t)(50000UL >> oversampling_setting);
	int32_t pressure = 0;//pressure in Pa
	if(B7 < 0x80000000)
		pressure = (B7 << 1)/B4;
	else
		pressure = (B7 / B4) << 1;
	X1 = (pressure >> 8)*(pressure >> 8);
	X1 = ((X1*3038) >> 16);
	X2 = ((-7357*pressure) >> 16);
	pressure = pressure + ((X1 + X2 + (int32_t)3791) >> 4);
	return pressure;
}

uint16_t BMP180_Pa_To_Hg (int32_t pressure_In_Pascals)
{
	return (uint32_t)(pressure_In_Pascals * 760) / 101325;
}

void BMP180_set_oversampling_setting(uint8_t mode)
{
	oversampling_setting = mode;
	switch (oversampling_setting)
	{
		case BMP180_ULTRA_LOW_POWER:		conversion_time = 5; break;
		case BMP180_STANDART:				conversion_time = 8; break;
		case BMP180_HIGH:					conversion_time = 14; break;
		case BMP180_ULTRA_HIGH_RESOLUTION:	conversion_time = 26; break;
		default: conversion_time = 5; break;
	}	
}


//UT = 27898;
//UP = 23843;
//calibration.AC1 = 408;
//calibration.AC2 = -72;
//calibration.AC3 = -14383;
//calibration.AC4 = 32741;
//calibration.AC5 = 32757;
//calibration.AC6 = 23153;
//calibration.B1 = 6190;
//calibration.B2 = 4;
//calibration.MB = -32768;
//calibration.MC = -8711;
//calibration.MD = 2868;

//I2C_Start(CONTROL);
//I2C_Address(BMP180_I2C_ADDRESS, I2C_W);
//I2C_SendByte(BMP180_CONTROL_REGISTER);
//I2C_SendByte((oversampling_setting & 0x3) << 6);
//I2C_Stop();
//while(!I2C_task);
//I2C_task = 0;
//BMP_SET_I2CRW_REG (in_buff[1], BMP_CTRL_OSS_MASK(ratio));
//HAL_I2C_Mem_Write( &hi2c1, BMP_WRITE_ADDR, BMP_CTRL_REG, 1, in_buff, 2, BMP_I2C_TIMEOUT );

//BMP_SET_I2CRW_REG (out_buff[0], BMP_SET_PRESS_CONV);
//HAL_I2C_Mem_Write ( &hi2c1, BMP_WRITE_ADDR, BMP_CTRL_REG, 1, out_buff, 1, BMP_I2C_TIMEOUT );