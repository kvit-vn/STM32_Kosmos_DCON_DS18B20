/*
 * usart_ring.h
 *
 *  Created on: Dec 10, 2019
 *      Author: kutuk
 */

#ifndef INC_DCON_H_
#define INC_DCON_H_
#include "main.h"
//#include "stdio.h"
/////////////////// DCON USART /////////////////////


#define DCON_RX_BUFFER_SIZE 64																	/*Длина приемного буфера*/
#define CRC_EN	1																				/*Константа "Контрольная сумма протокола DCON включена"*/
#define CRC_DIS	0																				/*Константа "Контрольная сумма протокола DCON отключена"*/
extern uint8_t _DCON_RX_Buffer[DCON_RX_BUFFER_SIZE];
extern uint8_t _DCON_TX_Buffer[DCON_RX_BUFFER_SIZE];
int8_t _DCON_RX_point;																	//Текущая позиция в приемном буфере UART
uint8_t _DCON_RX_length;																	//Длина принятой посылки по UART
uint8_t _DCON_RX_end;																	//Признак окончания принятой посылки по UART

uint8_t CHK_Control(uint8_t CHK);
uint8_t Read_DCON(uint8_t *buf);
uint8_t Write_DCON(uint8_t *buf,UART_HandleTypeDef *DCON_huart);

#endif /* INC_DCON_H_ */
