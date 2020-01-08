/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dcon.h"
#include "addr_pages.h"
#include "ds18b20.h"
//#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
	uint32_t	dwFirstRun;
	uint16_t	address;
	float	fltCa1;
	float	fltCa2;
	float	fltCa3;
	float	fltTempSet;
	float	fltTempDiff;
	uint16_t wCrc;
}tpParams;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PAR_ADDR ADDR_FLASH_PAGE_63
#define	PARAMS_WORDS sizeof(params)/4
#define COUNT_ADC_MAX	128
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t Address_Device=1;																				//Адресс устройства
uint8_t buf[DCON_RX_BUFFER_SIZE] = {0,};
unsigned char error;																						//Общий флаг ошибки
uint8_t char4[4];																							//Массив для преобразования числа в строку
int16_t OutTemperature[3] = {0,};																					//Массив температур для передачи по DCON
uint8_t flag=1;
float prev_t;
float prev_t1;
float prev_t2;
float filter=2;

 tpParams	params;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void FLASH_ReadSettings(void);
void FLASH_WriteSettings(void);
void Poll_DCON(void);
uint8_t char2_to_hex(uint8_t s1, uint8_t s2);
void hex_to_char2(uint8_t s1);
int8_t char4_to_dec(uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4);
void dec_to_char4(int16_t s1);
void Preparation_Response(uint8_t er);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim4)
	{
		HAL_TIM_Base_Stop_IT(htim);
		flag = 2;
	}
}
////////////чтение настроек из флеша/////////////////
void FLASH_ReadSettings(void) {
    //Read settings
    uint32_t *source_addr = (uint32_t *)PAR_ADDR;
    uint32_t *dest_addr = (void *)&params;
    for (uint16_t i=0; i<PARAMS_WORDS; i++) {
        *dest_addr = *(uint32_t*)source_addr;
        source_addr++;
        dest_addr++;
    }
}
////////////////////функция записи параметров во флеш/////////////
void FLASH_WriteSettings(void)
{
	static FLASH_EraseInitTypeDef EraseInitStruct; // структура для очистки флеша

	  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES; // постраничная очистка, FLASH_TYPEERASE_MASSERASE - очистка всего флеша
	  EraseInitStruct.PageAddress = PAR_ADDR; // адрес 63-й страницы
	  EraseInitStruct.NbPages = 1;                       // кол-во страниц для стирания
	  //EraseInitStruct.Banks = FLASH_BANK_1; // FLASH_BANK_2 - банк №2, FLASH_BANK_BOTH - оба банка

	  uint32_t page_error = 0; // переменная, в которую запишется адрес страницы при неудачном стирании

	  //char tmp_str[64] = {0,};

	  ////////////////////////////// ОЧ�?СТКА ///////////////////////////////////
	  HAL_FLASH_Unlock(); // разблокировать флеш

	  if(HAL_FLASHEx_Erase(&EraseInitStruct, &page_error) != HAL_OK)
	  {
	         // uint32_t er = HAL_FLASH_GetError();
	          //snprintf(tmp_str, 64, "ER %lu\n", er);
	         // HAL_UART_Transmit(&huart1, (uint8_t*)tmp_str, strlen(tmp_str), 100);
	          while(1){}
	  }

	  //HAL_UART_Transmit(DEBUG, (uint8_t*)"Erase OK\n", 9, 100);

	  HAL_FLASH_Lock(); // заблокировать флеш
	  ///////////////запись////////////////////////
	  HAL_FLASH_Unlock(); // разблокировать флеш

	    //uint32_t address = PAR_ADDR; // запись в начало страницы 63
	    //uint16_t idata[] = {0x1941, 0x1945};    // массив из двух чисел для записи
	    uint32_t *source_addr = (void *)&params;
	    uint32_t dest_addr =  PAR_ADDR;
	    for(uint8_t i = 0; i < PARAMS_WORDS; i++)
	    {
	            if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dest_addr, *source_addr) != HAL_OK)
	            {
	                   // uint32_t er = HAL_FLASH_GetError();
	                    //snprintf(tmp_str, 64, "ER %lu\n", er);
	                    //HAL_UART_Transmit(DEBUG, (uint8_t*)tmp_str, strlen(tmp_str), 100);
	                    while(1){}
	            }

	            source_addr++;
	            dest_addr+=4;;
	    }

	    //HAL_UART_Transmit(DEBUG, (uint8_t*)"Write Params OK\n", strlen("Write Params OK\n"), 100);

	    HAL_FLASH_Lock(); // заблокировать флеш
}
//=================== Функция перевода шеснадцатеричного двухсимвольного числа в двоичное ==================
uint8_t char2_to_hex(uint8_t s1, uint8_t s2)
{
	if ((s1>=0x30) && (s1<=0x39)) s1=s1-0x30;																//Если первый символ цифра (0-9)
	else
	if ((s1>=0x41) && (s1<=0x46)) s1=s1-0x37;																//Если первый символ буква (A-F)
	else error=1;																							//�?наче синтаксическая ошибка

	if ((s2>=0x30) && (s2<=0x39)) s2=s2-0x30;																//Если второй символ цифра (0-9)
	else
	if ((s2>=0x41) && (s2<=0x46)) s2=s2-0x37;																//Если второй символ буква (A-F)
	else error=1;																							//�?наче синтаксическая ошибка

	return (s1<<4)+s2;																						//Склеивание старшей и младшей частей
}

//=================== Функция перевода двоичного числа в двухсильвольное шеснадцатеричное ==================
void hex_to_char2(uint8_t s1)
{
	char temp;																								//Вспомогательная переменная

	temp=s1>>4;
	if (temp<=9) char4[0]=temp+0x30;																		//Если первый символ цифра (0-9)
	else char4[0]=temp+0x37;																				//Если первый символ буква (A-F)
	temp=s1 & 0x0F;
	if (temp<=9) char4[1]=temp+0x30;																		//Если второй символ цифра (0-9)
	else char4[1]=temp+0x37;																				//Если второй символ буква (A-F)
}

//----------------------------------------------------------------------------------------------------------//
//==================== Функция перевода десятичного четырехсимвольного числа в двоичное ====================//
//----------------------------------------------------------------------------------------------------------//
int8_t char4_to_dec(uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4)
{
	if ((s1>=0x30)&&(s1<=0x39)) s1=s1-0x30;																	//Если 1 символ цифра (0-9)
	else error=1;																							//�?наче синтаксическая ошибка

	if ((s2>=0x30)&&(s2<=0x39)) s2=s2-0x30;																	//Если 2 символ цифра (0-9)
	else error=1;																							//�?наче синтаксическая ошибка

	if ((s3>=0x30)&&(s3<=0x39)) s3=s3-0x30;																	//Если 3 символ цифра (0-9)
	else error=1;																							//�?наче синтаксическая ошибка

	if ((s4>=0x30)&&(s4<=0x39)) s4=s4-0x30;																	//Если 4 символ цифра (0-9)
	else error=1;																							//�?наче синтаксическая ошибка

	return s1*1000+s2*100+s3*10+s4;																			//Склеивание всех частей
}

//==================== Функция перевода двоичного числа в четырехсильвольное десятичное ====================
void dec_to_char4(int16_t s1)
{
	char temp;																								//Вспомогательная переменная

	temp=s1 % 10;
	s1/=10;
	char4[3]=temp+0x30;																						//4 символ

	temp=s1 % 10;
	s1/=10;
	char4[2]=temp+0x30;																						//3 символ

	temp=s1 % 10;
	s1/=10;
	char4[1]=temp+0x30;																						//2 символ
	char4[0]=s1+0x30;																						//1 символ
}

//================================= Функция подготовки ответного сообщения =================================
void Preparation_Response(uint8_t er)
{
	hex_to_char2(Address_Device>>8);																		//Преобразование адреса к символьному виду (старшая часть)
	buf[1]=char4[0];
	buf[2]=char4[1];
	hex_to_char2(Address_Device & 0x00FF);																	//Преобразование адреса к символьному виду (младшая часть)
	buf[3]=char4[0];
	buf[4]=char4[1];

	if (er==0) buf[0]='!';																					//Если команда принята
	else buf[0]='?';																						//Если команда с ошибкой

	buf[5]=0;																								//Признак конца строки (в стандарте ANSI)
}
void Poll_DCON(void)
{
	int16_t int_temp;																							//Вспомогательные переменные
	int16_t int_temp2;																							//Вспомогательные переменные
	uint8_t hex1;																							//Переменные используемые при декодировании команд
	uint8_t length;																						//Длина принятой команды

	if (Read_DCON(buf)==0x01)																		//Если есть не прочитанные данные, протокол совпадает и нет ошибки контрольной суммы (см. описание библиотек)
  	{
		length=buf[0];																						//Определение длинны принятой команды

		if (buf[1]=='^' && buf[2]=='R' && buf[3]=='E' && buf[4]=='S' && buf[5]=='E' && buf[6]=='T')			//Если пришла команда ^RESET
		{
			//if (((PINB & (1<<PB4))==0) && (Address_Device==0x0000))											//Если модуль был запущен в режиме "Init"
			//{																								//Запись в EEPROM заводских настроек

			params.dwFirstRun=0x0001;
			params.address=1;
			params.fltCa1=0;
			params.fltCa2=0;
			params.fltCa3=0;
			params.fltTempSet=24.0;
			params.fltTempDiff=2.0;
			void *p =&params;
			params.wCrc=0;
			for (char i=0; i< (PARAMS_WORDS*4 - 2);i++ )
			{
				params.wCrc += *(((char*)p)+i);
			}
			FLASH_WriteSettings();
			Address_Device=params.address;
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);														//Переключаем RS-485 на передачу
			Write_DCON((uint8_t*)"!RESET_OK", &huart1);																	//Отправить ответ в COM порт
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);														//Переключаем RS-485 на прием
			//}
		}
		else
		{
			int_temp=(char2_to_hex(buf[3],buf[4])<<8)+char2_to_hex(buf[5],buf[6]);							//Декодирование полученного адреса
			if (int_temp!=Address_Device) return;															//Если адрес устройства не совпадает, досрочный выход без ответа (т.к. обращение происходит к другому устройству)

			if (buf[1]!='^')																				//Если не верный символ идентификации команды
			{
				Preparation_Response(1);																	//Подготовка ответа (команда не распознана)
			}
			else
			{
				switch(buf[2])
				{
				case 'R':
				case 'T':
				//-------------------- Команда чтения температуры с датчиков модуля (^RAAAA/r или ^TAAAA/r) --------------------
					if (length!=7)																			//Проверка длины команды
					{
						Preparation_Response(1);															//Подготовка ответа (команда не распознана)
						break;
					}

					Preparation_Response(0);																//Подготовка ответа (команда принята)

					int_temp2=OutTemperature[0];																//Чтение значение температуры первого датчика из массива
					hex1=0;
					if (int_temp2<0)																		//Определение знака
					{
						buf[5]='-';
						int_temp=0-int_temp2;
						hex1=1;
					}
					else int_temp=int_temp2;

					dec_to_char4(int_temp);																	//Преобразование температуры к символьному виду

					if (hex1==0) buf[5]=char4[0];
					buf[6]=char4[1];
					buf[7]=char4[2];
					buf[8]=char4[3];

					int_temp2=OutTemperature[1];															//Чтение значение температуры второго датчика из массива
					hex1=0;
					if (int_temp2<0)																		//Определение знака
					{
						buf[9]='-';
						int_temp=0-int_temp2;
						hex1=1;
					}
					else int_temp=int_temp2;
					dec_to_char4(int_temp);															//Преобразование температуры второго датчика к символьному виду
					if (hex1==0) buf[9]=char4[0];
					buf[10]=char4[1];
					buf[11]=char4[2];
					buf[12]=char4[3];

					int_temp2=OutTemperature[2];															//Чтение значение температуры третьего датчика из массива
					hex1=0;
					if (int_temp2<0)																		//Определение знака
					{
						buf[13]='-';
						int_temp=0-int_temp2;
						hex1=1;
					}
					else int_temp=int_temp2;

					dec_to_char4(int_temp);																	//Преобразование температуры к символьному виду
					if (hex1==0) buf[13]=char4[0];
					buf[14]=char4[1];
					buf[15]=char4[2];
					buf[16]=char4[3];

					buf[17]=0;																			//Конец сообщения
					break;
				case 'S':
				//-------------------------- Команда смены адреса модуля (^SAAAANNNN/r) ---------------------------
					error=0;
					int_temp=(char2_to_hex(buf[7],buf[8])<<8)+char2_to_hex(buf[9],buf[10]);					//Чтение нового адреса
					if ((length!=11) || (error==1) || (int_temp==0))										//Проверка корректности команды
					{
						Preparation_Response(1);															//Подготовка ответа (команда не распознана)
						break;
					}
					Address_Device=int_temp;

					params.address=Address_Device;
					FLASH_WriteSettings();
					Preparation_Response(0);																//Подготовка ответа (команда принята)
					break;
				case 'P':
				//----------------------------- Команда установки степени открытия жалюзей (^PAAAACC/r) ------------------------------
					error=0;
					hex1=char2_to_hex(buf[7],buf[8]);
					if ((error) || (length!=9) || (hex1<0) || (hex1>0x64))									//Проверка корректности команды
					{
						Preparation_Response(1);															//Подготовка ответа (команда не распознана)
						break;
					}

					TIM1->CCR1 = (uint32_t)600 * (uint32_t)hex1;													//Запись положения открытия жалюзей в процентах
					Preparation_Response(0);																//Подготовка ответа (команда принята)
					break;
				default: Preparation_Response(1);															//Подготовка ответа (команда не распознана)
				}
			}
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);														//Переключаем RS-485 на передачу
			Write_DCON(buf, &huart1);																				//Отправка ответа на команду
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);														//Переключаем RS-485 на прием
		}
  	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
  HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_1);


  FLASH_ReadSettings();
   if (params.dwFirstRun==0xFFFFFFFF)
   {
 	  params.dwFirstRun=0x0001;
 	  params.address=1;
 	  params.fltCa1=0;
 	  params.fltCa2=0;
 	  params.fltCa3=0;
 	  params.fltTempSet=24.0;
 	  params.fltTempDiff=2.0;
 	  void *p =&params;
 	  params.wCrc=0;
 	  for (char i=0; i< (PARAMS_WORDS*4 - 2);i++ )
 	  {
 		  params.wCrc += *(((char*)p)+i);
 	  }
 	  FLASH_WriteSettings();
   }
 	  Address_Device=params.address;

 	  DWT_Init();
 	  Init_ds18b20_0(DS18B20_Resolution_12_bit);											// �?нициализируем первый датчик
 	  Init_ds18b20_1(DS18B20_Resolution_12_bit);											// �?нициализируем второй датчик
 	  Init_ds18b20_2(DS18B20_Resolution_12_bit);											// �?нициализируем третий датчик
 	  //Для инициализации переменных с префиксом prev_ инициализируем изменерения и запрашиваем у датчиков температуру
 	  //Переменные с префиксом prev_ необходимы для реализации простого фильтра:
 	  //Если текущее значение температуры больше или меньше на значение в переменной filter(случайная помеха)
 	  //то используем значение предыдущего измерения
 	  ds18b20_mesureTemperature_0();
 	  ds18b20_mesureTemperature_1();
 	  ds18b20_mesureTemperature_2();
 	  delay_us(760000);													// Здержка необходимая датчикам для измерения температуры
 	  prev_t = ds18b20_getTemperature_0()/16.0;
 	  prev_t1 = ds18b20_getTemperature_0()/16.0;
 	  prev_t2 = ds18b20_getTemperature_0()/16.0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  Poll_DCON();
	  if(flag == 1)
	  {
		  flag = 0;
		  ds18b20_mesureTemperature_0();
		  ds18b20_mesureTemperature_1();
		  ds18b20_mesureTemperature_2();
		  __HAL_TIM_SET_AUTORELOAD(&htim4, DELAY_T_CONVERT);
		  HAL_TIM_Base_Start_IT(&htim4); // запускаем таймер
	  }
	  else if(flag == 2) // когда таймер отсчитает 750мс, он установит flag == 2
	  {
		  flag=0;
		  int16_t temp = ds18b20_getTemperature_0();
		  int16_t temp1 = ds18b20_getTemperature_1();
		  int16_t temp2 = ds18b20_getTemperature_2();
	  	  float t = temp / 16.0;
	  	  float t1 = temp1 / 16.0;
	  	  float t2 = temp2 / 16.0;
	  	  if( (prev_t - t) > filter || (t - prev_t) > filter  ) t = prev_t; else prev_t = t;
	  	  if( (prev_t1 - t1) > filter || (t1 - prev_t1) > filter  ) t1 = prev_t1; else prev_t1 = t1;
	  	  if( (prev_t2 - t2) > filter || (t2 - prev_t2) > filter  ) t2 = prev_t2; else prev_t2 = t2;
	  	  OutTemperature[0]= t*10;
	  	  OutTemperature[1]= t1*10;
	  	  OutTemperature[2]= t2*10;
	  	  flag = 1; // запускаем новое измерение.
	  }


	  Poll_DCON();

	  HAL_Delay(100);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
