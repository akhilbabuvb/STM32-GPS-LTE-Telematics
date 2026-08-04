#include "config.h"

#include <stdio.h>
#include <string.h>
#include "gsm.h"

extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;
static uint8_t rxDataGSM;
static RingBuffer_t gsmBuffer;

static char gsmLine[256];
static uint16_t gsmIndex = 0;

static volatile uint8_t gsmCommandFinished = 0;
static volatile uint8_t gsmCommandSuccess  = 0;

void GSM_Init(void)
{
    RingBuffer_Init(&gsmBuffer);

    memset(gsmLine, 0, sizeof(gsmLine));
    gsmIndex = 0;

    HAL_UART_Receive_IT(&huart3,
                        &rxDataGSM,
                        1);
}
void GSM_UARTCallback(void)
{
    RingBuffer_Write(&gsmBuffer, rxDataGSM);

    HAL_UART_Receive_IT(&huart3,
                        &rxDataGSM,
                        1);
}

HAL_StatusTypeDef GSM_SendCommand(char *command)
{
    HAL_StatusTypeDef status;

    /* Reset command status flags */
    gsmCommandFinished = 0;
    gsmCommandSuccess  = 0;

    /* Send AT Command */
    status = HAL_UART_Transmit(&huart3,
                               (uint8_t *)command,
                               strlen(command),
                               HAL_MAX_DELAY);

    if (status != HAL_OK)
    {
        return status;
    }

    /* Send CR+LF */
    status = HAL_UART_Transmit(&huart3,
                               (uint8_t *)"\r\n",
                               2,
                               HAL_MAX_DELAY);

    if (status != HAL_OK)
    {
        return status;
    }

    /* Wait until GSM replies or timeout */
       uint32_t startTick = HAL_GetTick();

       while (gsmCommandFinished == 0)
       {
           GSM_Task();

           if ((HAL_GetTick() - startTick) > 3000)
           {
               return HAL_TIMEOUT;
           }
       }

       if (gsmCommandSuccess == 1)
       {
           return HAL_OK;
       }
       else
       {
           return HAL_ERROR;
       }
}

static void GSM_ProcessLine(char *line)
{
    /* Print complete line to Debug UART */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)line,
                      strlen(line),
                      HAL_MAX_DELAY);

    if (strstr(line, "OK") != NULL)
    {
        gsmCommandFinished = 1;
        gsmCommandSuccess  = 1;
    }
    else if (strstr(line, "ERROR") != NULL)
    {
        gsmCommandFinished = 1;
        gsmCommandSuccess  = 0;
    }
    else
    {
    	// Do Nothing
    }
}

void GSM_Task(void)
{
    uint8_t ch;

    while (RingBuffer_Read(&gsmBuffer, &ch))
    {
        gsmLine[gsmIndex++] = ch;

        if (gsmIndex >= sizeof(gsmLine)-1)
        {
        	gsmIndex = 0;
            memset(gsmLine, 0, sizeof(gsmLine));
        }

        if(ch == '\n')
        {
            gsmLine[gsmIndex] = '\0';

            GSM_ProcessLine(gsmLine);

            gsmIndex = 0;
            memset(gsmLine, 0, sizeof(gsmLine));
        }
    }
}

