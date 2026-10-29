/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32g4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
void process_gpgga_message();
void process_gpgsa_message();
#define buffsize 1024
#define MAX_NMEA_MESSAGE_SIZE 128
#define maxstor 20

char CDC_RX_BUF[buffsize];
char gpgga[MAX_NMEA_MESSAGE_SIZE];
char gpgsa[200];
char onebigstring[300];


char time[maxstor];
float latitude2;
float longitude2;
float altitude2;
int fix2;
int nofsat2;
float hdop2;
float hogeoid2;
float vdop2;
int hours;
int min;
int sec;

char latitude[maxstor];
char longitude[maxstor];
char altitude[maxstor];
char fix[maxstor];
char nofsat[maxstor];
char hdop[maxstor];
char hogeoid[maxstor];
char vdop[maxstor];
char hours2[5];
char min2[5];
char sec2[5];

volatile int latready=0;
volatile int longready=0;
volatile int altready=0;
volatile int fixready=0;
volatile int satready=0;
volatile int hdopready=0;
volatile int vdopready=0;
volatile int hogeoidready=0;
volatile int timeready=0;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void timeparse() {
    hours = (time[0] - '0') * 10 + (time[1] - '0');
    min = (time[2] - '0') * 10 + (time[3] - '0');
    sec = (time[4] - '0') * 10 + (time[5] - '0');
}
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

void findandcopygpgga(const char *startingpt, const char *endingpt) {
    char *begin = strstr(CDC_RX_BUF, startingpt);
    if (begin == NULL) {
        return;
    }

    char *end = strstr(begin, endingpt);
    if (end == NULL) {
        return;
    }

    strncpy(gpgga, begin, end - begin);
}


void findAndCopygpgsa() {
	 char *begin = strstr(CDC_RX_BUF, "$GPGNA");
	    if (begin == NULL) {
	        return ;
	    }

	    char *stop = strstr(begin, "\n");
	    if (stop == NULL) {
	        return;
	    }

	    int length = stop - begin;

	    for (int i = 0; i < length; i++) {
	        gpgsa[i] = begin[i];
	    }
	    gpgsa[length] = '\0';
}

void process_gpgga_message() {

        char *token = strtok(gpgga, ",");
        int findex = 0;
        while (token != NULL) {

            switch (findex) {
            case 1:  // Time
         strcpy(time, token);
         timeparse();
         hours2[0]=time[0];
         hours2[1]=time[1];
         hours2[2]='\0';
         min2[0]=time[2];
         min2[1]=time[3];
         min2[2]='\0';
         sec2[0]=time[4];
         sec2[1]=time[5];
         sec2[2]='\0';
         timeready=1;

         break;
            case 2:  // Latitude
            	strcpy(latitude, token);
            	latitude2=atof(latitude);
            	latready=1;

            	break;
            case 4:  // Longitude
            	strcpy(longitude, token);
            	longitude2=atof(longitude);
            	longready=1;

            	break;
            case 9:  // Altitude
            	strcpy(altitude, token);
            	altitude2=atof(altitude);
            	altready=1;

            	break;
            case 6:  // GPS FIX
            	strcpy(fix, token);
            	fix2 = atoi(fix);
            	fixready=1;

            	break;
            case 7:  // number of satellites
            	strcpy(nofsat, token);
            	nofsat2 = atoi(nofsat);
            	satready=1;

            	break;
            case 8:  // HDOP
            	strcpy(hdop, token);
            	hdop2 = atof(hdop);
            	hdopready=1;

            	break;

            }
            token = strtok(NULL, ",");
            findex++;
        }

}

void process_gpgsa_message()
{
	  char gpgsa_copy[200];
	   strcpy(gpgsa_copy, gpgsa);
	    char *tok = strtok(gpgsa_copy, ",*");
	    int fondex = 0;
	    int vdopIndex = 5 + nofsat2;
	    while (tok != NULL) {
    	 if (fondex == vdopIndex) {
	            strcpy(vdop, tok);
	             vdop2 = atof(vdop);
	              break;
	    	        }
	        tok = strtok(NULL, ",*");
	        fondex++;
	    }

}


void joinstring()
{

	strcpy(onebigstring, "Time: ");
	strcat(onebigstring, hours2);
	strcat(onebigstring, " hours ");
	strcat(onebigstring, min2);
	strcat(onebigstring, " min ");
	strcat(onebigstring, sec2);
	strcat(onebigstring, " sec ");
	strcat(onebigstring, "\n");

	strcat(onebigstring, "Latitude: ");
	strcat(onebigstring, latitude);
	strcat(onebigstring, "\n");


	strcat(onebigstring, "Longitude: ");
	strcat(onebigstring, longitude);
	strcat(onebigstring, "\n");


	strcat(onebigstring, "Altitude: ");
	strcat(onebigstring, altitude);
	strcat(onebigstring, "\n");

	strcat(onebigstring, "Hdop: ");
	strcat(onebigstring, hdop);
	strcat(onebigstring, "\n");

	strcat(onebigstring, "Vdop: ");
	strcat(onebigstring, vdop);
	strcat(onebigstring, "\n");

	strcat(onebigstring, "Fix: ");
	strcat(onebigstring, fix);
	strcat(onebigstring, "\n");

	strcat(onebigstring, "No. of Satellites: ");
	strcat(onebigstring, nofsat);
	strcat(onebigstring, "\n");
	strcat(onebigstring, "-------------------------------------------------------\n");


}


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	findandcopygpgga("$GNGGA","\n");
	process_gpgga_message();
	findAndCopygpgsa();
	process_gpgsa_message();
	joinstring();
    HAL_UART_Receive_DMA(&huart1, (uint8_t *)CDC_RX_BUF, buffsize);
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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USB_Device_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_DMA(&huart1, (uint8_t *)CDC_RX_BUF, buffsize);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  CDC_Transmit_FS((uint8_t *)onebigstring, strlen(onebigstring));
	  	  	       HAL_Delay(500);
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
