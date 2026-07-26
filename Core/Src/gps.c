#include "config.h"
#include "gps.h"
#include "ring_buffer.h"
#include "nmea_parser.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/* Private Variables --------------------------------------------------------*/

static RingBuffer_t gpsBuffer;

static uint8_t rxDataGPS;
static uint8_t data;

static char message[100];
static uint16_t msgIndex = 0;

static GPS_Data_t gpsData;

/* -------------------------------------------------------------------------*/

void GPS_Init(void)
{
    RingBuffer_Init(&gpsBuffer);

    memset(message, 0, sizeof(message));
    msgIndex = 0;

    HAL_UART_Receive_IT(&huart1, &rxDataGPS, 1);
}

/* -------------------------------------------------------------------------*/

void GPS_UARTCallback(void)
{
    RingBuffer_Write(&gpsBuffer, rxDataGPS);

    HAL_UART_Receive_IT(&huart1,
                        &rxDataGPS,
                        1);
}

/* -------------------------------------------------------------------------*/

void GPS_Task(void)
{
    while (RingBuffer_Read(&gpsBuffer, &data))
    {
        /* Ignore LF */
        if (data == '\n')
            continue;

        /* End of sentence */
        if (data == '\r')
        {
            message[msgIndex] = '\0';

            if (strncmp(message, "$GNRMC", 6) == 0)
            {
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

                /* Parse complete RMC sentence */
                NMEA_ParseRMC(message, &gpsData);

#if DEBUG_GPS

                char headingText[20];

                if (gpsData.heading >= 0.0f)
                    sprintf(headingText, "%.2f°", gpsData.heading);
                else
                    strcpy(headingText, "N/A");

                char buffer[512];

                sprintf(buffer,
                        "\r\n"
                        "=====================================\r\n"
                        "UTC            : %s\r\n"
                        "Status         : %s\r\n"
                        "Latitude       : %.6f°\r\n"
                        "Longitude      : %.6f°\r\n"
                        "Speed          : %.2f km/h\r\n"
                        "Distance       : %.3f km\r\n"
                        "Heading        : %s\r\n"
                        "Date           : %s\r\n"
                        "=====================================\r\n\r\n",

                        gpsData.utcTime,
                        gpsData.status,
                        gpsData.latitude,
                        gpsData.longitude,
                        gpsData.speed,
                        gpsData.totalDistance / 1000.0f,
                        headingText,
                        gpsData.date);

                HAL_UART_Transmit(&huart2,
                                  (uint8_t *)buffer,
                                  strlen(buffer),
                                  HAL_MAX_DELAY);

#endif
            }

            /* Prepare next sentence */
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
