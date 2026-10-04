#include "config.h"

#include <stdio.h>
#include <string.h>

#include "gsm.h"


extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;


/* ============================================================
 * GSM RX
 * ============================================================ */

static uint8_t rxDataGSM;
static RingBuffer_t gsmBuffer;

static char gsmLine[256];
static uint16_t gsmIndex = 0;


/* ============================================================
 * GSM COMMAND STATE
 * ============================================================ */

static volatile uint8_t gsmCommandFinished = 0;
static volatile uint8_t gsmCommandSuccess  = 0;


/* ============================================================
 * HTTPACTION STATE
 * ============================================================ */

static volatile uint8_t httpActionFinished = 0;
static volatile uint8_t httpActionSuccess  = 0;

static volatile int httpStatusCode = -1;
static volatile uint32_t httpDataLength = 0;


/* ============================================================
 * HTTPDATA STATE
 * ============================================================ */

static volatile uint8_t httpDownloadReady = 0;
static volatile uint32_t gsmRxOverflowCount = 0;


/* ============================================================
 * TIMEOUTS
 * ============================================================ */

#define GSM_COMMAND_TIMEOUT      10000UL
#define GSM_HTTPDATA_TIMEOUT     15000UL
#define GSM_HTTPACTION_TIMEOUT   50000UL


/* ============================================================
 * GSM INIT
 * ============================================================ */

void GSM_Init(void)
{
    RingBuffer_Init(&gsmBuffer);

    memset(gsmLine, 0, sizeof(gsmLine));
    gsmIndex = 0;

    gsmCommandFinished = 0;
    gsmCommandSuccess  = 0;

    httpActionFinished = 0;
    httpActionSuccess  = 0;
    httpStatusCode     = -1;
    httpDataLength     = 0;

    httpDownloadReady = 0;

    /* Clear a possible stale UART overrun/error state before starting RX. */
    __HAL_UART_CLEAR_OREFLAG(&huart3);
    HAL_UART_Receive_IT(&huart3, &rxDataGSM, 1);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"\r\nGSM_Init OK\r\n",
                      strlen("\r\nGSM_Init OK\r\n"),
                      HAL_MAX_DELAY);
}


/* ============================================================
 * GSM UART RX CALLBACK
 * ============================================================ */

void GSM_UARTCallback(void)
{
    if (!RingBuffer_Write(&gsmBuffer, rxDataGSM))
    {
        gsmRxOverflowCount++;
    }

    HAL_UART_Receive_IT(&huart3,
                        &rxDataGSM,
                        1);
}

void GSM_UARTErrorCallback(void)
{
    __HAL_UART_CLEAR_OREFLAG(&huart3);
    __HAL_UART_CLEAR_FEFLAG(&huart3);
    __HAL_UART_CLEAR_NEFLAG(&huart3);
    __HAL_UART_CLEAR_PEFLAG(&huart3);
    HAL_UART_Receive_IT(&huart3, &rxDataGSM, 1);
}


/* ============================================================
 * SEND GSM COMMAND
 * ============================================================ */

HAL_StatusTypeDef GSM_SendCommand(char *command)
{
    HAL_StatusTypeDef status;
    uint32_t startTick;

    gsmCommandFinished = 0;
    gsmCommandSuccess  = 0;


    /* --------------------------------------------------------
     * DEBUG TX
     * -------------------------------------------------------- */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"\r\nGSM TX: ",
                      strlen("\r\nGSM TX: "),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)command,
                      strlen(command),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"\r\n",
                      2,
                      HAL_MAX_DELAY);


    /* --------------------------------------------------------
     * SEND COMMAND TO MODEM
     * -------------------------------------------------------- */

    status = HAL_UART_Transmit(&huart3,
                               (uint8_t *)command,
                               strlen(command),
                               HAL_MAX_DELAY);

    if (status != HAL_OK)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"GSM TX ERROR\r\n",
                          strlen("GSM TX ERROR\r\n"),
                          HAL_MAX_DELAY);

        return status;
    }


    /* --------------------------------------------------------
     * CRLF
     * -------------------------------------------------------- */

    status = HAL_UART_Transmit(&huart3,
                               (uint8_t *)"\r\n",
                               2,
                               HAL_MAX_DELAY);

    if (status != HAL_OK)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"GSM CRLF ERROR\r\n",
                          strlen("GSM CRLF ERROR\r\n"),
                          HAL_MAX_DELAY);

        return status;
    }


    /* --------------------------------------------------------
     * WAIT FOR RESPONSE
     * -------------------------------------------------------- */

    startTick = HAL_GetTick();

    while (gsmCommandFinished == 0)
    {
        GSM_Task();

        if ((HAL_GetTick() - startTick) >= GSM_COMMAND_TIMEOUT)
        {
            char timeoutMsg[96];
            snprintf(timeoutMsg, sizeof(timeoutMsg),
                     "GSM COMMAND TIMEOUT (RX overflow=%lu)\r\n",
                     (unsigned long)gsmRxOverflowCount);
            HAL_UART_Transmit(&huart2,
                              (uint8_t *)timeoutMsg,
                              strlen(timeoutMsg),
                              HAL_MAX_DELAY);

            /* Make sure RX interrupt is armed again after a timeout. */
            __HAL_UART_CLEAR_OREFLAG(&huart3);
            HAL_UART_Receive_IT(&huart3, &rxDataGSM, 1);

            return HAL_TIMEOUT;
        }
    }


    /* --------------------------------------------------------
     * RESULT
     * -------------------------------------------------------- */

    if (gsmCommandSuccess == 1)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"GSM COMMAND OK\r\n",
                          strlen("GSM COMMAND OK\r\n"),
                          HAL_MAX_DELAY);

        return HAL_OK;
    }


    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"GSM COMMAND ERROR\r\n",
                      strlen("GSM COMMAND ERROR\r\n"),
                      HAL_MAX_DELAY);

    return HAL_ERROR;
}


/* ============================================================
 * PROCESS ONE COMPLETE GSM LINE
 * ============================================================ */

static void GSM_ProcessLine(char *line)
{
    char *httpActionPtr;


    /* --------------------------------------------------------
     * PRINT COMPLETE MODEM RESPONSE
     * -------------------------------------------------------- */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"GSM RX: ",
                      strlen("GSM RX: "),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)line,
                      strlen(line),
                      HAL_MAX_DELAY);


    /* ========================================================
     * HTTPDATA DOWNLOAD
     * ======================================================== */

    if (strstr(line, "DOWNLOAD") != NULL)
    {
        httpDownloadReady = 1;

        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"HTTPDATA: DOWNLOAD READY\r\n",
                          strlen("HTTPDATA: DOWNLOAD READY\r\n"),
                          HAL_MAX_DELAY);

        return;
    }


    /* ========================================================
     * HTTPACTION RESPONSE
     *
     * Expected:
     *
     * +HTTPACTION: 1,200,123
     *
     * The modem may send:
     *
     * \r\n+HTTPACTION: 1,200,123\r\n
     *
     * Therefore do NOT use sscanf(line, ...)
     * directly. First find +HTTPACTION:
     * ======================================================== */

    httpActionPtr = strstr(line, "+HTTPACTION:");

    if (httpActionPtr != NULL)
    {
        int method;
        int status;
        int length;


        if (sscanf(httpActionPtr,
                   "+HTTPACTION: %d,%d,%d",
                   &method,
                   &status,
                   &length) == 3)
        {
            httpStatusCode = status;
            httpDataLength = length;

            httpActionFinished = 1;


            if (status >= 200 && status < 300)
            {
                httpActionSuccess = 1;
            }
            else
            {
                httpActionSuccess = 0;
            }


            /* ------------------------------------------------
             * PRINT PARSED RESULT
             * ------------------------------------------------ */

            char buffer[128];

            snprintf(buffer,
                     sizeof(buffer),
                     "HTTPACTION PARSED: method=%d status=%d length=%d\r\n",
                     method,
                     status,
                     length);

            HAL_UART_Transmit(&huart2,
                              (uint8_t *)buffer,
                              strlen(buffer),
                              HAL_MAX_DELAY);
        }
        else
        {
            HAL_UART_Transmit(&huart2,
                              (uint8_t *)
                              "HTTPACTION PARSE ERROR\r\n",
                              strlen("HTTPACTION PARSE ERROR\r\n"),
                              HAL_MAX_DELAY);
        }

        return;
    }


    /* ========================================================
     * ERROR RESPONSE
     * ======================================================== */

    if (strstr(line, "ERROR") != NULL)
    {
        gsmCommandFinished = 1;
        gsmCommandSuccess  = 0;

        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"GSM RESPONSE: ERROR\r\n",
                          strlen("GSM RESPONSE: ERROR\r\n"),
                          HAL_MAX_DELAY);

        return;
    }


    /* ========================================================
     * OK RESPONSE
     * ======================================================== */

    if (strstr(line, "OK") != NULL)
    {
        gsmCommandFinished = 1;
        gsmCommandSuccess  = 1;

        return;
    }
}


/* ============================================================
 * HTTPDATA
 * ============================================================ */

static HAL_StatusTypeDef GSM_HTTP_SendData(char *data)
{
    HAL_StatusTypeDef status;

    uint32_t startTick;

    char command[64];

    uint32_t dataLength;


    /* --------------------------------------------------------
     * DATA LENGTH
     * -------------------------------------------------------- */

    dataLength = strlen(data);


    /* --------------------------------------------------------
     * RESET DOWNLOAD FLAG
     * -------------------------------------------------------- */

    httpDownloadReady = 0;


    /* --------------------------------------------------------
     * BUILD HTTPDATA COMMAND
     *
     * AT+HTTPDATA=<length>,10000
     * -------------------------------------------------------- */

    snprintf(command,
             sizeof(command),
             "AT+HTTPDATA=%lu,10000",
             (unsigned long)dataLength);


    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"\r\nHTTPDATA TX: ",
                      strlen("\r\nHTTPDATA TX: "),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)command,
                      strlen(command),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"\r\n",
                      2,
                      HAL_MAX_DELAY);


    /* --------------------------------------------------------
     * SEND HTTPDATA COMMAND
     * -------------------------------------------------------- */

    status = HAL_UART_Transmit(&huart3,
                               (uint8_t *)command,
                               strlen(command),
                               HAL_MAX_DELAY);

    if (status != HAL_OK)
    {
        return status;
    }


    status = HAL_UART_Transmit(&huart3,
                               (uint8_t *)"\r\n",
                               2,
                               HAL_MAX_DELAY);

    if (status != HAL_OK)
    {
        return status;
    }


    /* --------------------------------------------------------
     * WAIT FOR DOWNLOAD
     * -------------------------------------------------------- */

    startTick = HAL_GetTick();

    while (httpDownloadReady == 0)
    {
        GSM_Task();

        if ((HAL_GetTick() - startTick) >= GSM_HTTPDATA_TIMEOUT)
        {
            HAL_UART_Transmit(&huart2,
                              (uint8_t *)
                              "HTTPDATA DOWNLOAD TIMEOUT\r\n",
                              strlen("HTTPDATA DOWNLOAD TIMEOUT\r\n"),
                              HAL_MAX_DELAY);

            return HAL_TIMEOUT;
        }
    }


    /* --------------------------------------------------------
     * DOWNLOAD READY
     * SEND ACTUAL DATA
     * -------------------------------------------------------- */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"HTTPDATA BODY TX: ",
                      strlen("HTTPDATA BODY TX: "),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)data,
                      strlen(data),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"\r\n",
                      2,
                      HAL_MAX_DELAY);


    status = HAL_UART_Transmit(&huart3,
                               (uint8_t *)data,
                               strlen(data),
                               HAL_MAX_DELAY);

    if (status != HAL_OK)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"HTTPDATA BODY TX ERROR\r\n",
                          strlen("HTTPDATA BODY TX ERROR\r\n"),
                          HAL_MAX_DELAY);

        return status;
    }


    /* --------------------------------------------------------
     * WAIT FOR HTTPDATA OK
     * -------------------------------------------------------- */

    gsmCommandFinished = 0;
    gsmCommandSuccess  = 0;

    startTick = HAL_GetTick();

    while (gsmCommandFinished == 0)
    {
        GSM_Task();

        if ((HAL_GetTick() - startTick) >= GSM_HTTPDATA_TIMEOUT)
        {
            HAL_UART_Transmit(&huart2,
                              (uint8_t *)
                              "HTTPDATA COMPLETE TIMEOUT\r\n",
                              strlen("HTTPDATA COMPLETE TIMEOUT\r\n"),
                              HAL_MAX_DELAY);

            return HAL_TIMEOUT;
        }
    }


    /* --------------------------------------------------------
     * CHECK HTTPDATA RESULT
     * -------------------------------------------------------- */

    if (gsmCommandSuccess == 1)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"HTTPDATA COMPLETE\r\n",
                          strlen("HTTPDATA COMPLETE\r\n"),
                          HAL_MAX_DELAY);

        return HAL_OK;
    }


    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"HTTPDATA FAILED\r\n",
                      strlen("HTTPDATA FAILED\r\n"),
                      HAL_MAX_DELAY);

    return HAL_ERROR;
}


/* ============================================================
 * HTTP GET
 * ============================================================ */

HAL_StatusTypeDef GSM_HTTP_Get(char *url, char *query)
{
    HAL_StatusTypeDef status;
    uint32_t startTick;
    char command[512];
    char fullUrl[700];

    httpActionFinished = 0;
    httpActionSuccess  = 0;
    httpStatusCode     = -1;
    httpDataLength     = 0;

    snprintf(fullUrl, sizeof(fullUrl), "%s?%s", url, query);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"\r\n=====================================\r\n",
                      strlen("\r\n=====================================\r\n"),
                      HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"HTTP GET START\r\nURL: ",
                      strlen("HTTP GET START\r\nURL: "),
                      HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart2,
                      (uint8_t *)fullUrl,
                      strlen(fullUrl),
                      HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);

    /* STEP 1: HTTPINIT */
    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"STEP 1: HTTPINIT\r\n",
                      strlen("STEP 1: HTTPINIT\r\n"),
                      HAL_MAX_DELAY);

    status = GSM_SendCommand("AT");
    if (status != HAL_OK)
        return status;

    /* HTTPTERM may legitimately return ERROR when HTTP is not initialized. */
    GSM_SendCommand("AT+HTTPTERM");
    HAL_Delay(100);

    status = GSM_SendCommand("AT+HTTPINIT");
    if (status != HAL_OK)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"HTTPINIT retry...\r\n",
                          strlen("HTTPINIT retry...\r\n"),
                          HAL_MAX_DELAY);
        HAL_Delay(300);
        status = GSM_SendCommand("AT+HTTPINIT");
    }

    if (status != HAL_OK)
    {
        GSM_SendCommand("AT+HTTPTERM");
        return status;
    }

    /* STEP 2: URL */
    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"STEP 2: HTTPPARA URL\r\n",
                      strlen("STEP 2: HTTPPARA URL\r\n"),
                      HAL_MAX_DELAY);

    snprintf(command, sizeof(command), "AT+HTTPPARA=\"URL\",\"%s\"", fullUrl);
    status = GSM_SendCommand(command);
    if (status != HAL_OK)
    {
        GSM_SendCommand("AT+HTTPTERM");
        return status;
    }

    /* STEP 3: GET */
    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"STEP 3: HTTPACTION GET\r\n",
                      strlen("STEP 3: HTTPACTION GET\r\n"),
                      HAL_MAX_DELAY);

    httpActionFinished = 0;
    httpActionSuccess = 0;

    status = HAL_UART_Transmit(&huart3,
                               (uint8_t *)"AT+HTTPACTION=0\r\n",
                               strlen("AT+HTTPACTION=0\r\n"),
                               HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        GSM_SendCommand("AT+HTTPTERM");
        return status;
    }

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"HTTP GET SENT\r\nWAITING FOR HTTPACTION RESULT...\r\n",
                      strlen("HTTP GET SENT\r\nWAITING FOR HTTPACTION RESULT...\r\n"),
                      HAL_MAX_DELAY);

    startTick = HAL_GetTick();
    while (httpActionFinished == 0)
    {
        GSM_Task();

        if ((HAL_GetTick() - startTick) >= GSM_HTTPACTION_TIMEOUT)
        {
            HAL_UART_Transmit(&huart2,
                              (uint8_t *)"HTTPACTION TIMEOUT\r\n",
                              strlen("HTTPACTION TIMEOUT\r\n"),
                              HAL_MAX_DELAY);
            GSM_SendCommand("AT+HTTPTERM");
            return HAL_TIMEOUT;
        }
    }

    if (httpActionSuccess)
    {
        char result[96];
        snprintf(result, sizeof(result),
                 "HTTP RESULT: status=%d length=%lu\r\n",
                 httpStatusCode, (unsigned long)httpDataLength);
        HAL_UART_Transmit(&huart2, (uint8_t *)result, strlen(result), HAL_MAX_DELAY);
        GSM_SendCommand("AT+HTTPTERM");
        return HAL_OK;
    }

    {
        char result[96];
        snprintf(result, sizeof(result),
                 "HTTP SERVER ERROR: status=%d length=%lu\r\n",
                 httpStatusCode, (unsigned long)httpDataLength);
        HAL_UART_Transmit(&huart2, (uint8_t *)result, strlen(result), HAL_MAX_DELAY);
    }

    GSM_SendCommand("AT+HTTPTERM");
    return HAL_ERROR;
}


/* ============================================================
 * HTTP POST
 * ============================================================ */

HAL_StatusTypeDef GSM_HTTP_Post(char *url, char *data)
{
    HAL_StatusTypeDef status;

    uint32_t startTick;

    char command[256];


    /* ========================================================
     * RESET HTTPACTION STATE
     * ======================================================== */

    httpActionFinished = 0;
    httpActionSuccess  = 0;

    httpStatusCode = -1;
    httpDataLength = 0;

    httpDownloadReady = 0;


    /* ========================================================
     * DEBUG
     * ======================================================== */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "\r\n=====================================\r\n",
                      strlen("\r\n=====================================\r\n"),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "HTTP POST START\r\n",
                      strlen("HTTP POST START\r\n"),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"URL: ",
                      strlen("URL: "),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)url,
                      strlen(url),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"\r\nDATA: ",
                      strlen("\r\nDATA: "),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)data,
                      strlen(data),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"\r\n",
                      2,
                      HAL_MAX_DELAY);


    /* ========================================================
     * STEP 1
     * HTTPINIT
     * ======================================================== */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "STEP 1: HTTPINIT\r\n",
                      strlen("STEP 1: HTTPINIT\r\n"),
                      HAL_MAX_DELAY);


    /* Verify the modem is responsive immediately before HTTP setup. */
    status = GSM_SendCommand("AT");

    if (status != HAL_OK)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"FAILED: MODEM AT CHECK\r\n",
                          strlen("FAILED: MODEM AT CHECK\r\n"),
                          HAL_MAX_DELAY);
        return status;
    }

    /* A previous interrupted transaction can leave HTTP initialized.
       HTTPTERM is harmless if it was not initialized, so clean it first. */
    GSM_SendCommand("AT+HTTPTERM");
    HAL_Delay(200);

    status = GSM_SendCommand("AT+HTTPINIT");

    /* One retry is intentional: it also recovers from a transient UART
       error/overrun without changing the rest of the application. */
    if (status != HAL_OK)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"HTTPINIT retry...\r\n",
                          strlen("HTTPINIT retry...\r\n"),
                          HAL_MAX_DELAY);

        __HAL_UART_CLEAR_OREFLAG(&huart3);
        HAL_UART_Receive_IT(&huart3, &rxDataGSM, 1);
        HAL_Delay(300);

        status = GSM_SendCommand("AT+HTTPINIT");
    }

    if (status != HAL_OK)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"FAILED: HTTPINIT\r\n",
                          strlen("FAILED: HTTPINIT\r\n"),
                          HAL_MAX_DELAY);

        GSM_SendCommand("AT+HTTPTERM");
        return status;
    }


    /* ========================================================
     * STEP 2
     * URL
     * ======================================================== */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "STEP 2: HTTPPARA URL\r\n",
                      strlen("STEP 2: HTTPPARA URL\r\n"),
                      HAL_MAX_DELAY);


    snprintf(command,
             sizeof(command),
             "AT+HTTPPARA=\"URL\",\"%s\"",
             url);


    status = GSM_SendCommand(command);

    if (status != HAL_OK)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)
                          "FAILED: HTTPPARA URL\r\n",
                          strlen("FAILED: HTTPPARA URL\r\n"),
                          HAL_MAX_DELAY);

        GSM_SendCommand("AT+HTTPTERM");

        return status;
    }


    /* ========================================================
     * STEP 3
     * CONTENT TYPE
     * ======================================================== */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "STEP 3: HTTPPARA CONTENT\r\n",
                      strlen("STEP 3: HTTPPARA CONTENT\r\n"),
                      HAL_MAX_DELAY);


    status = GSM_SendCommand(
        "AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\""
    );


    if (status != HAL_OK)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)
                          "FAILED: HTTPPARA CONTENT\r\n",
                          strlen("FAILED: HTTPPARA CONTENT\r\n"),
                          HAL_MAX_DELAY);

        GSM_SendCommand("AT+HTTPTERM");

        return status;
    }


    /* ========================================================
     * STEP 4
     * HTTPDATA
     * ======================================================== */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "STEP 4: HTTPDATA\r\n",
                      strlen("STEP 4: HTTPDATA\r\n"),
                      HAL_MAX_DELAY);


    status = GSM_HTTP_SendData(data);


    if (status != HAL_OK)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)
                          "FAILED: HTTPDATA\r\n",
                          strlen("FAILED: HTTPDATA\r\n"),
                          HAL_MAX_DELAY);

        GSM_SendCommand("AT+HTTPTERM");

        return status;
    }


    /* ========================================================
     * STEP 5
     * HTTPACTION
     *
     * IMPORTANT:
     *
     * AT+HTTPACTION=1
     *
     * normally has TWO stages:
     *
     * 1. Immediate command response:
     *
     *    OK
     *
     * 2. Asynchronous HTTP result:
     *
     *    +HTTPACTION: 1,200,<length>
     *
     * We handle them separately.
     * ======================================================== */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "STEP 5: HTTPACTION POST\r\n",
                      strlen("STEP 5: HTTPACTION POST\r\n"),
                      HAL_MAX_DELAY);


    /* --------------------------------------------------------
     * RESET STATES
     * -------------------------------------------------------- */

    gsmCommandFinished = 0;
    gsmCommandSuccess  = 0;

    httpActionFinished = 0;
    httpActionSuccess  = 0;

    httpStatusCode = -1;
    httpDataLength = 0;


    /* --------------------------------------------------------
     * SEND HTTPACTION
     * -------------------------------------------------------- */

    status = HAL_UART_Transmit(&huart3,
                               (uint8_t *)
                               "AT+HTTPACTION=1\r\n",
                               strlen("AT+HTTPACTION=1\r\n"),
                               HAL_MAX_DELAY);


    if (status != HAL_OK)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)
                          "HTTPACTION TX ERROR\r\n",
                          strlen("HTTPACTION TX ERROR\r\n"),
                          HAL_MAX_DELAY);

        GSM_SendCommand("AT+HTTPTERM");

        return status;
    }


    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "HTTPACTION SENT\r\n",
                      strlen("HTTPACTION SENT\r\n"),
                      HAL_MAX_DELAY);


    /* ========================================================
     * STEP 5A
     * WAIT FOR IMMEDIATE OK
     * ======================================================== */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "WAITING FOR HTTPACTION COMMAND OK...\r\n",
                      strlen("WAITING FOR HTTPACTION COMMAND OK...\r\n"),
                      HAL_MAX_DELAY);


    startTick = HAL_GetTick();


    while (gsmCommandFinished == 0)
    {
        GSM_Task();

        if ((HAL_GetTick() - startTick) >= GSM_COMMAND_TIMEOUT)
        {
            HAL_UART_Transmit(&huart2,
                              (uint8_t *)
                              "HTTPACTION COMMAND OK TIMEOUT\r\n",
                              strlen("HTTPACTION COMMAND OK TIMEOUT\r\n"),
                              HAL_MAX_DELAY);

            GSM_SendCommand("AT+HTTPTERM");

            return HAL_TIMEOUT;
        }
    }


    /* --------------------------------------------------------
     * CHECK IMMEDIATE COMMAND RESULT
     * -------------------------------------------------------- */

    if (gsmCommandSuccess == 0)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)
                          "HTTPACTION COMMAND ERROR\r\n",
                          strlen("HTTPACTION COMMAND ERROR\r\n"),
                          HAL_MAX_DELAY);

        GSM_SendCommand("AT+HTTPTERM");

        return HAL_ERROR;
    }


    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "HTTPACTION COMMAND OK\r\n",
                      strlen("HTTPACTION COMMAND OK\r\n"),
                      HAL_MAX_DELAY);


    /* ========================================================
     * STEP 5B
     * WAIT FOR +HTTPACTION
     * ======================================================== */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "WAITING FOR +HTTPACTION RESULT...\r\n",
                      strlen("WAITING FOR +HTTPACTION RESULT...\r\n"),
                      HAL_MAX_DELAY);


    startTick = HAL_GetTick();


    while (httpActionFinished == 0)
    {
        GSM_Task();

        if ((HAL_GetTick() - startTick) >= GSM_HTTPACTION_TIMEOUT)
        {
            HAL_UART_Transmit(&huart2,
                              (uint8_t *)
                              "HTTPACTION TIMEOUT AFTER 50 SEC\r\n",
                              strlen("HTTPACTION TIMEOUT AFTER 50 SEC\r\n"),
                              HAL_MAX_DELAY);

            /*
             * The modem may still be busy with the HTTP transaction.
             * We try HTTPTERM, but do not assume it will succeed.
             */

            GSM_SendCommand("AT+HTTPTERM");

            return HAL_TIMEOUT;
        }
    }


    /* ========================================================
     * STEP 6
     * HTTP RESULT
     * ======================================================== */

    {
        char resultBuffer[160];

        snprintf(resultBuffer,
                 sizeof(resultBuffer),
                 "HTTP RESULT: status=%d length=%lu\r\n",
                 httpStatusCode,
                 (unsigned long)httpDataLength);

        HAL_UART_Transmit(&huart2,
                          (uint8_t *)resultBuffer,
                          strlen(resultBuffer),
                          HAL_MAX_DELAY);
    }


    /* --------------------------------------------------------
     * CHECK HTTP STATUS
     * -------------------------------------------------------- */

    if (httpActionSuccess == 0)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)
                          "HTTP SERVER ERROR\r\n",
                          strlen("HTTP SERVER ERROR\r\n"),
                          HAL_MAX_DELAY);

        GSM_SendCommand("AT+HTTPTERM");

        return HAL_ERROR;
    }


    /* ========================================================
     * STEP 7
     * HTTPTERM
     * ======================================================== */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "STEP 7: HTTPTERM\r\n",
                      strlen("STEP 7: HTTPTERM\r\n"),
                      HAL_MAX_DELAY);


    status = GSM_SendCommand("AT+HTTPTERM");


    if (status != HAL_OK)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)
                          "HTTPTERM FAILED\r\n",
                          strlen("HTTPTERM FAILED\r\n"),
                          HAL_MAX_DELAY);

        return status;
    }


    /* ========================================================
     * SUCCESS
     * ======================================================== */

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "HTTP POST SUCCESS\r\n",
                      strlen("HTTP POST SUCCESS\r\n"),
                      HAL_MAX_DELAY);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)
                      "=====================================\r\n\r\n",
                      strlen("=====================================\r\n\r\n"),
                      HAL_MAX_DELAY);


    return HAL_OK;
}


/* ============================================================
 * GSM TASK
 *
 * Reads bytes from GSM ring buffer and builds complete lines.
 * ============================================================ */

void GSM_Task(void)
{
    uint8_t ch;


    while (RingBuffer_Read(&gsmBuffer, &ch))
    {
        /* ----------------------------------------------------
         * PREVENT LINE BUFFER OVERFLOW
         * ---------------------------------------------------- */

        if (gsmIndex >= sizeof(gsmLine) - 1)
        {
            gsmIndex = 0;

            memset(gsmLine,
                   0,
                   sizeof(gsmLine));
        }


        /* ----------------------------------------------------
         * STORE BYTE
         * ---------------------------------------------------- */

        gsmLine[gsmIndex++] = ch;


        /* ----------------------------------------------------
         * COMPLETE LINE
         *
         * GSM responses normally end with \r\n.
         * We process when \n arrives.
         * ---------------------------------------------------- */

        if (ch == '\n')
        {
            gsmLine[gsmIndex] = '\0';


            GSM_ProcessLine(gsmLine);


            gsmIndex = 0;

            memset(gsmLine,
                   0,
                   sizeof(gsmLine));
        }
    }
}
