/*
 * ds18b20.c
 *
 *  Created on: 5 мая 2019 г.
 *      Author: istarik.ru
 */


#include "ds18b20.h"

#define DS_PORT_0 GPIOA
#define DS_PIN_0  GPIO_PIN_0
#define DS_PORT_1 GPIOA
#define DS_PIN_1  GPIO_PIN_1
#define DS_PORT_2 GPIOA
#define DS_PIN_2  GPIO_PIN_2
//uint32_t DELAY_WAIT_CONVERT = DELAY_T_CONVERT;

void DWT_Init(void)
{
    SCB_DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // разрешаем использовать счётчик
	DWT_CONTROL |= DWT_CTRL_CYCCNTENA_Msk;   // запускаем счётчик
}

void delay_us(uint32_t us)
{
    uint32_t us_count_tic =  us * (SystemCoreClock / 1000000);
    DWT->CYCCNT = 0U; // обнуляем счётчик
    while(DWT->CYCCNT < us_count_tic);
}
//////////-------------Для первого датчика------------------------------
void Init_ds18b20_0(DS18B20_Resolution resolution)
{
	//DWT_Init();
	setResolution_0(resolution);
}

void reset_0()
{
	HAL_GPIO_WritePin(DS_PORT_0, DS_PIN_0, GPIO_PIN_RESET);
	delay_us(DELAY_RESET);
	HAL_GPIO_WritePin(DS_PORT_0, DS_PIN_0, GPIO_PIN_SET);
	delay_us(DELAY_RESET);
}


void writeBit_0(uint8_t bit)
{
	HAL_GPIO_WritePin(DS_PORT_0, DS_PIN_0, GPIO_PIN_RESET);
	delay_us(bit ? DELAY_WRITE_1 : DELAY_WRITE_0);
	HAL_GPIO_WritePin(DS_PORT_0, DS_PIN_0, GPIO_PIN_SET);
	delay_us(bit ? DELAY_WRITE_1_PAUSE : DELAY_WRITE_0_PAUSE);
}

void writeByte_0(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		writeBit_0(data >> i & 1);
		delay_us(DELAT_PROTECTION);
	}
}

uint8_t readBit_0()
{
	uint8_t bit = 0;
	HAL_GPIO_WritePin(DS_PORT_0, DS_PIN_0, GPIO_PIN_RESET);
	delay_us(DELAY_READ_SLOT);
	HAL_GPIO_WritePin(DS_PORT_0, DS_PIN_0, GPIO_PIN_SET);
	delay_us(DELAY_BUS_RELAX);
	bit = (HAL_GPIO_ReadPin(DS_PORT_0, DS_PIN_0) ? 1 : 0);
	delay_us(DELAY_READ_PAUSE);
	return bit;
}

int16_t readTemperature_0()
{
	int16_t data = 0;
	for (uint8_t i = 0; i < 16; i++)
	{
		data += (int16_t) readBit_0() << i;
	}

	//return  (uint16_t)(((float) data / 16.0) * 10.0);
	return data;
}

void ds18b20_mesureTemperature_0()
{
	reset_0();
	writeByte_0(SKIP_ROM);
	writeByte_0(CONVERT_T);

}
int16_t ds18b20_getTemperature_0()
{
	reset_0();
	writeByte_0(SKIP_ROM);
	writeByte_0(READ_SCRATCHPAD);
	return readTemperature_0();
}

void setResolution_0(DS18B20_Resolution resolution)
{
	reset_0();
	writeByte_0(SKIP_ROM);
	writeByte_0(WRITE_SCRATCHPAD);
	writeByte_0(TH_REGISTER);
	writeByte_0(TL_REGISTER);
	writeByte_0(resolution);
	//DELAY_WAIT_CONVERT = DELAY_T_CONVERT / getDevider(resolution);
}
//----------------------------------------------------------------------------
//////////-------------Для второго датчика------------------------------
void Init_ds18b20_1(DS18B20_Resolution resolution)
{
	//DWT_Init();
	setResolution_1(resolution);
}

void reset_1()
{
	HAL_GPIO_WritePin(DS_PORT_1, DS_PIN_1, GPIO_PIN_RESET);
	delay_us(DELAY_RESET);
	HAL_GPIO_WritePin(DS_PORT_1, DS_PIN_1, GPIO_PIN_SET);
	delay_us(DELAY_RESET);
}


void writeBit_1(uint8_t bit)
{
	HAL_GPIO_WritePin(DS_PORT_1, DS_PIN_1, GPIO_PIN_RESET);
	delay_us(bit ? DELAY_WRITE_1 : DELAY_WRITE_0);
	HAL_GPIO_WritePin(DS_PORT_1, DS_PIN_1, GPIO_PIN_SET);
	delay_us(bit ? DELAY_WRITE_1_PAUSE : DELAY_WRITE_0_PAUSE);
}

void writeByte_1(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		writeBit_1(data >> i & 1);
		delay_us(DELAT_PROTECTION);
	}
}

uint8_t readBit_1()
{
	uint8_t bit = 0;
	HAL_GPIO_WritePin(DS_PORT_1, DS_PIN_1, GPIO_PIN_RESET);
	delay_us(DELAY_READ_SLOT);
	HAL_GPIO_WritePin(DS_PORT_1, DS_PIN_1, GPIO_PIN_SET);
	delay_us(DELAY_BUS_RELAX);
	bit = (HAL_GPIO_ReadPin(DS_PORT_1, DS_PIN_1) ? 1 : 0);
	delay_us(DELAY_READ_PAUSE);
	return bit;
}

int16_t readTemperature_1()
{
	int16_t data = 0;
	for (uint8_t i = 0; i < 16; i++)
	{
		data += (int16_t) readBit_1() << i;
	}

	//return  (uint16_t)(((float) data / 16.0) * 10.0);
	return data;
}

void ds18b20_mesureTemperature_1()
{
	reset_1();
	writeByte_1(SKIP_ROM);
	writeByte_1(CONVERT_T);

}
int16_t ds18b20_getTemperature_1()
{
	reset_1();
	writeByte_1(SKIP_ROM);
	writeByte_1(READ_SCRATCHPAD);
	return readTemperature_1();
}

void setResolution_1(DS18B20_Resolution resolution)
{
	reset_1();
	writeByte_1(SKIP_ROM);
	writeByte_1(WRITE_SCRATCHPAD);
	writeByte_1(TH_REGISTER);
	writeByte_1(TL_REGISTER);
	writeByte_1(resolution);
	//DELAY_WAIT_CONVERT = DELAY_T_CONVERT / getDevider(resolution);
}
//--------------------------------------------------------------
//////////-------------Для третьего датчика------------------------------
void Init_ds18b20_2(DS18B20_Resolution resolution)
{
	//DWT_Init();
	setResolution_2(resolution);
}

void reset_2()
{
	HAL_GPIO_WritePin(DS_PORT_2, DS_PIN_2, GPIO_PIN_RESET);
	delay_us(DELAY_RESET);
	HAL_GPIO_WritePin(DS_PORT_2, DS_PIN_2, GPIO_PIN_SET);
	delay_us(DELAY_RESET);
}


void writeBit_2(uint8_t bit)
{
	HAL_GPIO_WritePin(DS_PORT_2, DS_PIN_2, GPIO_PIN_RESET);
	delay_us(bit ? DELAY_WRITE_1 : DELAY_WRITE_0);
	HAL_GPIO_WritePin(DS_PORT_2, DS_PIN_2, GPIO_PIN_SET);
	delay_us(bit ? DELAY_WRITE_1_PAUSE : DELAY_WRITE_0_PAUSE);
}

void writeByte_2(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		writeBit_2(data >> i & 1);
		delay_us(DELAT_PROTECTION);
	}
}

uint8_t readBit_2()
{
	uint8_t bit = 0;
	HAL_GPIO_WritePin(DS_PORT_2, DS_PIN_2, GPIO_PIN_RESET);
	delay_us(DELAY_READ_SLOT);
	HAL_GPIO_WritePin(DS_PORT_2, DS_PIN_2, GPIO_PIN_SET);
	delay_us(DELAY_BUS_RELAX);
	bit = (HAL_GPIO_ReadPin(DS_PORT_2, DS_PIN_2) ? 1 : 0);
	delay_us(DELAY_READ_PAUSE);
	return bit;
}

int16_t readTemperature_2()
{
	int16_t data = 0;
	for (uint8_t i = 0; i < 16; i++)
	{
		data += (int16_t) readBit_2() << i;
	}

	//return  (uint16_t)(((float) data / 16.0) * 10.0);
	return data;
}

void ds18b20_mesureTemperature_2()
{
	reset_2();
	writeByte_2(SKIP_ROM);
	writeByte_2(CONVERT_T);

}
int16_t ds18b20_getTemperature_2()
{
	reset_2();
	writeByte_2(SKIP_ROM);
	writeByte_2(READ_SCRATCHPAD);
	return readTemperature_2();
}

void setResolution_2(DS18B20_Resolution resolution)
{
	reset_2();
	writeByte_2(SKIP_ROM);
	writeByte_2(WRITE_SCRATCHPAD);
	writeByte_2(TH_REGISTER);
	writeByte_2(TL_REGISTER);
	writeByte_2(resolution);
	//DELAY_WAIT_CONVERT = DELAY_T_CONVERT / getDevider(resolution);
}
//--------------------------------------------------------------
