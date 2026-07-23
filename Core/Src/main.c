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
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <math.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ring_buffer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PI              3.14159265358979323846f
#define EARTH_RADIUS    6371000.0f      // meters

#define DEBUG_GPS    1
#define DEBUG_LTE    1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

RingBuffer_t gpsBuffer;

uint8_t data;
uint8_t rxData;

uint16_t msgIndex = 0;

char message[100];


char utcTime[20];
char gpsStatus[5];

char latitude[20];
char ns[5];

char longitude[20];
char ew[5];

char speed[20];
char heading[20];

char date[20];

char formattedTime[20];
char formattedDate[20];

float previousLatitude = 0.0f;
float previousLongitude = 0.0f;

float totalDistance = 0.0f;

uint8_t firstFix = 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void GetField(char *sentence,
              uint8_t fieldNumber,
              char *destination,
              uint8_t maxLength)
{
    uint8_t commaCount = 0;
    uint8_t index = 0;

    while (*sentence != '\0')
    {
        if (*sentence == ',')
        {
            commaCount++;

            if (commaCount > fieldNumber)
            {
                break;
            }

            sentence++;
            continue;
        }

        if (commaCount == fieldNumber)
        {
            if (index < (maxLength - 1))
            {
                destination[index++] = *sentence;
            }
        }

        sentence++;
    }

    destination[index] = '\0';
}


float ConvertCoordinate(char *coordinate)
{
    float value = atof(coordinate);

    int degrees = (int)(value / 100);

    float minutes = value - (degrees * 100);

    return degrees + (minutes / 60.0f);
}

void FormatTime(char *utc, char *output)
{
    sprintf(output,
            "%c%c:%c%c:%c%c",
            utc[0], utc[1],
            utc[2], utc[3],
            utc[4], utc[5]);
}

void FormatDate(char *date, char *output)
{
    sprintf(output,
            "%c%c/%c%c/20%c%c",
            date[0], date[1],
            date[2], date[3],
            date[4], date[5]);
}
float DegreesToRadians(float degrees)
{
    return degrees * PI / 180.0f;
}

float CalculateDistance(float lat1,
                        float lon1,
                        float lat2,
                        float lon2)
{
    float dLat = DegreesToRadians(lat2 - lat1);
    float dLon = DegreesToRadians(lon2 - lon1);

    lat1 = DegreesToRadians(lat1);
    lat2 = DegreesToRadians(lat2);

    float a =
        sinf(dLat / 2.0f) * sinf(dLat / 2.0f) +
        cosf(lat1) *
        cosf(lat2) *
        sinf(dLon / 2.0f) *
        sinf(dLon / 2.0f);

    float c = 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));

    return EARTH_RADIUS * c;
}
float CalculateHeading(float lat1,
                       float lon1,
                       float lat2,
                       float lon2)
{
    lat1 = DegreesToRadians(lat1);
    lat2 = DegreesToRadians(lat2);

    float dLon = DegreesToRadians(lon2 - lon1);

    float y = sinf(dLon) * cosf(lat2);

    float x = cosf(lat1) * sinf(lat2)
            - sinf(lat1) * cosf(lat2) * cosf(dLon);

    float bearing = atan2f(y, x);

    bearing = bearing * 180.0f / PI;

    if (bearing < 0)
    {
        bearing += 360.0f;
    }

    return bearing;
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
//  RingBuffer_Init(&gpsBuffer);
 // RingBuffer_Write(&gpsBuffer, 'A');
 // RingBuffer_Write(&gpsBuffer, 'B');
 // RingBuffer_Write(&gpsBuffer, 'C');


  RingBuffer_Init(&gpsBuffer);

  memset(message, 0, sizeof(message));
  msgIndex = 0;

  HAL_UART_Receive_IT(&huart1, &rxData, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
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
  //while (1)
 // {
    //  char msg[] = "STM32 UART Working\r\n";

     // HAL_UART_Transmit(&huart1,
      //                  (uint8_t *)msg,
      //                  strlen(msg),
      //                  HAL_MAX_DELAY);

     // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

     // HAL_Delay(1000);
 // }
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
  while (1)
  {
      if (RingBuffer_Read(&gpsBuffer, &data))
      {
          /* Ignore Line Feed */
          if (data == '\n')
          {
              continue;
          }

          /* End of NMEA sentence */
          if (data == '\r')
          {
              message[msgIndex] = '\0';

              if (strncmp(message, "$GNRMC", 6) == 0)
              {
                  /* ---------- DEBUG ---------- */
#if DEBUG_GPS
                  HAL_UART_Transmit(&huart2,
                                    (uint8_t *)"\r\nRAW: ",
                                    7,
                                    HAL_MAX_DELAY);

                  HAL_UART_Transmit(&huart2,
                                    (uint8_t *)message,
                                    strlen(message),
                                    HAL_MAX_DELAY);

                  HAL_UART_Transmit(&huart2,
                                    (uint8_t *)"\r\n\r\n",
                                    4,
                                    HAL_MAX_DELAY);
#endif
                  /* --------------------------- */

                  /* Parse RMC fields */
                  GetField(message, 1, utcTime, sizeof(utcTime));
                  GetField(message, 2, gpsStatus, sizeof(gpsStatus));
                  GetField(message, 3, latitude, sizeof(latitude));
                  GetField(message, 4, ns, sizeof(ns));
                  GetField(message, 5, longitude, sizeof(longitude));
                  GetField(message, 6, ew, sizeof(ew));
                  GetField(message, 7, speed, sizeof(speed));
                  GetField(message, 8, heading, sizeof(heading));
                  GetField(message, 9, date, sizeof(date));

                  FormatTime(utcTime, formattedTime);
                  FormatDate(date, formattedDate);

                  float latitudeDecimal = 0.0f;
                  float longitudeDecimal = 0.0f;
                  float speedKmph = 0.0f;
                  float calculatedHeading = -1.0f;
                  /* Convert Latitude */
                  if (strlen(latitude))
                  {
                      latitudeDecimal = ConvertCoordinate(latitude);

                      if (ns[0] == 'S')
                      {
                          latitudeDecimal = -latitudeDecimal;
                      }
                  }

                  /* Convert Longitude */
                  if (strlen(longitude))
                  {
                      longitudeDecimal = ConvertCoordinate(longitude);

                      if (ew[0] == 'W')
                      {
                          longitudeDecimal = -longitudeDecimal;
                      }
                  }

                  /* Convert Speed (Knots -> km/h) */
                  if (strlen(speed))
                  {
                      speedKmph = atof(speed) * 1.852f;
                  }

                  /* ------------ Distance & Heading Calculation ------------ */
                  if (gpsStatus[0] == 'A')
                  {
                      if (firstFix)
                      {
                          previousLatitude = latitudeDecimal;
                          previousLongitude = longitudeDecimal;
                          firstFix = 0;
                      }
                      else
                      {
                          float distance = CalculateDistance(previousLatitude,
                                                             previousLongitude,
                                                             latitudeDecimal,
                                                             longitudeDecimal);

                          if (distance > 2.0f)
                          {
                              totalDistance += distance;

                              calculatedHeading = CalculateHeading(
                                                      previousLatitude,
                                                      previousLongitude,
                                                      latitudeDecimal,
                                                      longitudeDecimal);

                              previousLatitude = latitudeDecimal;
                              previousLongitude = longitudeDecimal;
                          }
                      }
                  }
                  /* -------------------------------------------------------- */

                  char headingText[20];

                  if (strlen(heading))
                  {
                      strcpy(headingText, heading);
                  }
                  else if (calculatedHeading >= 0.0f)
                  {
                      sprintf(headingText, "%.2f°", calculatedHeading);
                  }
                  else
                  {
                      strcpy(headingText, "N/A");
                  }

                  char buffer[512];

                  sprintf(buffer,
                          "\r\n"
                          "=====================================\r\n"
                          "UTC            : %s\r\n"
                          "Status         : %s\r\n"
                          "Raw Latitude   : %s %s\r\n"
                          "Raw Longitude  : %s %s\r\n"
                          "Latitude       : %.6f°\r\n"
                          "Longitude      : %.6f°\r\n"
                          "Speed          : %.2f km/h\r\n"
                          "Distance       : %.3f km\r\n"//"Distance       : %.3f km\r\n"
                          "Heading        : %s\r\n"
                          "Date           : %s\r\n"
                          "=====================================\r\n\r\n",
                          formattedTime,
                          gpsStatus,
                          latitude,
                          ns,
                          longitude,
                          ew,
                          latitudeDecimal,
                          longitudeDecimal,
                          speedKmph,
                          totalDistance / 1000.0f,
						  headingText,
                          formattedDate);
#if DEBUG_GPS
                  HAL_UART_Transmit(&huart2,
                                    (uint8_t *)buffer,
                                    strlen(buffer),
                                    HAL_MAX_DELAY);
#endif
              }

              /* Ready for next sentence */
              msgIndex = 0;
              memset(message, 0, sizeof(message));
          }
          else
          {
              if (msgIndex < sizeof(message) - 1)
              {
                  message[msgIndex++] = data;
              }
          }
      }
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

  /* USER CODE END USART2_Init 1 */
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
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    RingBuffer_Write(&gpsBuffer, rxData);

    HAL_UART_Receive_IT(&huart1, &rxData, 1);
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
