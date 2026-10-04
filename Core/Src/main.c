
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "gsm.h"
#include "gps.h"
#include <string.h>
#include <stdio.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

HAL_StatusTypeDef status;
uint32_t lastThingSpeakUpdate = 0;
char thingSpeakData[300];
GPS_Data_t *gps;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	  unsigned int utcTime,utcDate;

	                  unsigned int hour = 0;
	                  unsigned int minute = 0;
	                  unsigned int second = 0;

	                  unsigned int day = 0;
	                  unsigned int month = 0;
	                  unsigned int year = 0;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	//HAL_StatusTypeDef status;

	/*----------------------------------------------------------
	 * * HAL initialization
	 **----------------------------------------------------------*/

	HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /*----------------------------------------------------------
   * * Peripheral initialization
   * *----------------------------------------------------------*/

  MX_GPIO_Init();
  MX_USART1_UART_Init();// GPS
  MX_USART2_UART_Init();// Debug terminal
  MX_USART3_UART_Init();// GSM

  /* USER CODE BEGIN 2 */
 // RingBuffer_Init(&gpsBuffer);
 // RingBuffer_Write(&gpsBuffer, 'A');
 // RingBuffer_Write(&gpsBuffer, 'B');
 // RingBuffer_Write(&gpsBuffer, 'C');

  /*----------------------------------------------------------
   *  * Initialize UART manager
   *  * This should initialize the GSM/GPS UART receive
   *  * interrupt and the corresponding ring buffers.
   *----------------------------------------------------------*/

  UART_Manager_Init();

  /*----------------------------------------------------------
   * * Give A7670C time to boot
   *  *----------------------------------------------------------*/

  HAL_Delay(3000);

  /*----------------------------------------------------------
   * * Basic GSM module communication test
   *  *----------------------------------------------------------*/

  /* Test GSM */
  status = GSM_SendCommand("AT");

      if(status == HAL_OK)
      {
          HAL_UART_Transmit(&huart2,
                            (uint8_t *)"AT Success\r\n",
							strlen("AT Success\r\n"),
                            HAL_MAX_DELAY);
      }
      else if(status == HAL_TIMEOUT)
      {
          HAL_UART_Transmit(&huart2,
                            (uint8_t *)"AT Timeout\r\n",
							strlen("AT Timeout\r\n"),
                            HAL_MAX_DELAY);
      }
      else
      {
          HAL_UART_Transmit(&huart2,
                            (uint8_t *)"AT Failed\r\n",
							strlen("AT Failed\r\n"),
                            HAL_MAX_DELAY);
      }
/*Testing */



      /*Testing Finish */
    //  GSM_SendCommand("AT+HTTPTERM");
      /*----------------------------------------------------------
       * HTTP POST TEST *
       * Execute only once for now. *
       * Replace YOUR_API_KEY with your actual ThingSpeak *
       * Write API Key. *
       * field1=25 is only a test value.
       *----------------------------------------------------------*/
//      if (status == HAL_OK)
//     {
//    	  /* Test ThingSpeak HTTP POST */
//    	    status = GSM_HTTP_Post(
//    	        "http://api.thingspeak.com/update?api_key=VWCXX045QMU34DGJ",
//            "field1=25");
//    	  if (status == HAL_OK)
//    	  {
//    		  HAL_UART_Transmit(&huart2, (uint8_t *)"HTTP POST Success\r\n", strlen("HTTP POST Success\r\n"), HAL_MAX_DELAY);
//    	  }
//    	  else if (status == HAL_TIMEOUT)
//    	  {
//    		  HAL_UART_Transmit(&huart2, (uint8_t *)"HTTP POST Timeout\r\n", strlen("HTTP POST Timeout\r\n"), HAL_MAX_DELAY);
//    	  }
//    	  else
//    	  {
//    		  HAL_UART_Transmit(&huart2, (uint8_t *)"HTTP POST Failed\r\n", strlen("HTTP POST Failed\r\n"), HAL_MAX_DELAY);
//    	  }
//      }
      /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  while (1)
//      {
//    	 // GPS_Task();
//    	  GSM_Task();
//    /* USER CODE END WHILE */
//
//    /* USER CODE BEGIN 3 */
//      }
 // while (1)
 // {

	// HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	// HAL_Delay(100);
	// HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	// HAL_Delay(100);
	// HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	// HAL_Delay(600);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
 // }

      /*----------------------------------------------------------
       * * Main application loop
       * *
       * * GSM_Task() processes asynchronous GSM responses.
       * * GPS_Task() processes incoming GPS NMEA data.
       * * Do NOT continuously transmit test strings to UART3.
       * * UART3 is now dedicated to the A7670C module.
       *----------------------------------------------------------*/
      while (1)
      {
          /* Process GPS received data */
          GPS_Task();

          /* Process GSM received data */
          GSM_Task();

          /* Get latest GPS data */
          gps = GPS_GetData();

          if (gps == NULL)
          {
              HAL_Delay(10);
              continue;
          }

          /* GPS has a valid fix */
          if (gps->status[0] == 'A')
          {
              /* Send to ThingSpeak every 50 seconds */
              if ((HAL_GetTick() - lastThingSpeakUpdate) >= 50000UL)
              {
                  lastThingSpeakUpdate = HAL_GetTick();

                  /* ThingSpeak GET update.
                   * Use numeric channel fields only. ThingSpeak records the
                   * server timestamp automatically; GPS time/date/status do
                   * not need to be placed into field values.
                   */
                  /* convert UTC time , data strings to int  */

                  sscanf(gps->utcTime, "%2u:%2u:%2u",
                         &hour, &minute, &second);

                  utcTime = hour * 10000 + minute * 100 + second;


                  sscanf(gps->date, "%2u/%2u/%4u",
                         &day, &month, &year);

                  utcDate = year * 10000 + month * 100 + day;

                  snprintf(thingSpeakData,
                           sizeof(thingSpeakData),
                           "api_key=VWCXX045QMU34DGJ"
						   "&field1=%u"
						   "&field2=1"
                           "&field3=%.6f"
                           "&field4=%.6f"
                           "&field5=%.2f"
                           "&field6=%.3f"
                           "&field7=%.2f"
						   "&field8=%u",
						   utcTime,
                           gps->latitude,
                           gps->longitude,
                           gps->speed,
                           gps->totalDistance / 1000.0f,
                           gps->heading,
						   utcDate);

                  /* Show data being sent */
                  HAL_UART_Transmit(&huart2,
                                    (uint8_t *)"\r\nThingSpeak Data:\r\n",
                                    strlen("\r\nThingSpeak Data:\r\n"),
                                    HAL_MAX_DELAY);

                  HAL_UART_Transmit(&huart2,
                                    (uint8_t *)thingSpeakData,
                                    strlen(thingSpeakData),
                                    HAL_MAX_DELAY);

                  HAL_UART_Transmit(&huart2,
                                    (uint8_t *)"\r\n",
                                    2,
                                    HAL_MAX_DELAY);

                  /* Send GPS data to ThingSpeak using HTTP GET.
                   * This avoids the intermittent HTTPDATA/UART interaction
                   * and follows ThingSpeak's documented GET update API. */
                  status = GSM_HTTP_Get(
                      "http://api.thingspeak.com/update",
                      thingSpeakData);

                  if (status == HAL_OK)
                  {
                      HAL_UART_Transmit(&huart2,
                                        (uint8_t *)"GPS -> ThingSpeak OK\r\n",
                                        strlen("GPS -> ThingSpeak OK\r\n"),
                                        HAL_MAX_DELAY);
                  }
                  else if (status == HAL_TIMEOUT)
                  {
                      HAL_UART_Transmit(&huart2,
                                        (uint8_t *)"GPS -> ThingSpeak TIMEOUT\r\n",
                                        strlen("GPS -> ThingSpeak TIMEOUT\r\n"),
                                        HAL_MAX_DELAY);
                  }
                  else
                  {
                      HAL_UART_Transmit(&huart2,
                                        (uint8_t *)"GPS -> ThingSpeak FAILED\r\n",
                                        strlen("GPS -> ThingSpeak FAILED\r\n"),
                                        HAL_MAX_DELAY);
                  }
              }
          }

          /* Keep the loop responsive */
          HAL_Delay(10);
      }
/* start*/
//      while (1)
//      {
//          /* =========================================
//           * Run GPS task
//           * ========================================= */
//          GPS_Task();
//
//          /* =========================================
//           * Run GSM task
//           * ========================================= */
//          GSM_Task();
//
//          /* =========================================
//           * Get latest GPS data
//           * ========================================= */
//          gps = GPS_GetData();
//
//          if (gps == NULL)
//          {
//              continue;
//          }
//
//          /* =========================================
//           * Send ThingSpeak every 30 seconds
//           * ========================================= */
//          if ((HAL_GetTick() - lastThingSpeakUpdate) >= 30000UL)
//          {
//              lastThingSpeakUpdate = HAL_GetTick();
//
//              /* =====================================
//               * Only send when GPS fix is valid
//               * ===================================== */
//              if (gps->status[0] == 'A')
//              {
//                  /* =================================
//                   * Build ThingSpeak data
//                   *
//                   * IMPORTANT:
//                   * field1 has NO '&'
//                   * field2 onward uses '&'
//                   * ================================= */
//                  snprintf(thingSpeakData,
//                           sizeof(thingSpeakData),
//                           "field1=%s"
//                           "&field2=%s"
//                           "&field3=%.6f"
//                           "&field4=%.6f"
//                           "&field5=%.2f"
//                           "&field6=%.3f"
//                           "&field7=%.2f"
//                           "&field8=%s",
//                           gps->utcTime,
//                           gps->status,
//                           gps->latitude,
//                           gps->longitude,
//                           gps->speed,
//                           gps->totalDistance / 1000.0f,
//                           gps->heading,
//                           gps->date);
//
//                  /* =================================
//                   * Debug: print ThingSpeak data
//                   * through UART2
//                   * ================================= */
//                  HAL_UART_Transmit(&huart2,
//                                    (uint8_t *)"\r\nThingSpeak Data:\r\n",
//                                    strlen("\r\nThingSpeak Data:\r\n"),
//                                    HAL_MAX_DELAY);
//
//                  HAL_UART_Transmit(&huart2,
//                                    (uint8_t *)thingSpeakData,
//                                    strlen(thingSpeakData),
//                                    HAL_MAX_DELAY);
//
//                  HAL_UART_Transmit(&huart2,
//                                    (uint8_t *)"\r\n",
//                                    2,
//                                    HAL_MAX_DELAY);
//
//                  /* =================================
//                   * Send to ThingSpeak
//                   * ================================= */
//                  status = GSM_HTTP_Post(
//                              "http://api.thingspeak.com/update?api_key=VWCXX045QMU34DG",
//                              thingSpeakData);
//
//                  /* =================================
//                   * Check result
//                   * ================================= */
//                  if (status == HAL_OK)
//                  {
//                      HAL_UART_Transmit(&huart2,
//                                        (uint8_t *)
//                                        "GPS -> ThingSpeak SUCCESS\r\n",
//                                        strlen(
//                                        "GPS -> ThingSpeak SUCCESS\r\n"),
//                                        HAL_MAX_DELAY);
//                  }
//                  else if (status == HAL_TIMEOUT)
//                  {
//                      HAL_UART_Transmit(&huart2,
//                                        (uint8_t *)
//                                        "GPS -> ThingSpeak TIMEOUT\r\n",
//                                        strlen(
//                                        "GPS -> ThingSpeak TIMEOUT\r\n"),
//                                        HAL_MAX_DELAY);
//                  }
//                  else
//                  {
//                      HAL_UART_Transmit(&huart2,
//                                        (uint8_t *)
//                                        "GPS -> ThingSpeak FAILED\r\n",
//                                        strlen(
//                                        "GPS -> ThingSpeak FAILED\r\n"),
//                                        HAL_MAX_DELAY);
//                  }
//              }
//              else
//              {
//                  /* =================================
//                   * GPS data not valid
//                   * ================================= */
//                  HAL_UART_Transmit(&huart2,
//                                    (uint8_t *)
//                                    "GPS FIX INVALID - NOT SENT\r\n",
//                                    strlen(
//                                    "GPS FIX INVALID - NOT SENT\r\n"),
//                                    HAL_MAX_DELAY);
//              }
//          }
//      }
      /* end*/
//  while (1)
 // {
    //  HAL_UART_Receive(&huart1,
   //                    &rxData,
     //                  1,
    //                   HAL_MAX_DELAY);

    //  HAL_UART_Transmit(&huart1,
       //                 &rxData,
       //                 1,
       //                 HAL_MAX_DELAY);

    //   char newline[] = "\r\n";
      // HAL_UART_Transmit(&huart1, (uint8_t *)newline, 2, HAL_MAX_DELAY);
     //  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
          // HAL_Delay(1000);
 // }
 // while (1)
  //{


     // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
     // HAL_Delay(500);
	 // if (RingBuffer_Read(&gpsBuffer, &data))
	  //   {
		//  if (data == '\n')
		//  {
		   //   continue;
		//  }

	    //     if (data == '\r')
	    //     {
	             // Make it a valid C string
	      //       message[msgIndex] = '\0';

	             // Send the complete message
	       //      HAL_UART_Transmit(&huart1,
	        //                       (uint8_t *)message,
	        //                       msgIndex,
	        //                       HAL_MAX_DELAY);

	             // Send a new line (optional)
	         //    char newline[] = "\r\n";
	         //    HAL_UART_Transmit(&huart1,
	           //                    (uint8_t *)newline,
	           //                    strlen(newline),
	            //                   HAL_MAX_DELAY);

	             // Prepare for the next message
	          //   msgIndex = 0;
	      //   }
	   //      else
	      //   {
	             // Prevent buffer overflow
	        //     if (msgIndex < sizeof(message) - 1)
	        //     {
	             //    message[msgIndex] = data;
	             //    msgIndex++;
	           //  }
	     //    }
	    // }
	   //HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	     // HAL_Delay(500);
  //}

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
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
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

	  /* USER CODE END USART2_Init 2 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

	  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    HAL_UART_Transmit(&huart1, &rxData, 1, HAL_MAX_DELAY);
    // Prepare for the next byte
//    HAL_UART_Receive_IT(&huart1, &rxData, 1);
//}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    UART_Manager_RxCallback(huart);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3)
    {
        /* Do not transmit on another UART from inside the ISR. It can block
           the CPU long enough to cause another USART3 overrun. */
        GSM_UARTErrorCallback();
    }
    else if (huart == &huart1)
    {
        GPS_UARTErrorCallback();
    }
}
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
  * @param  line: assert_param line number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the assert_param error */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


