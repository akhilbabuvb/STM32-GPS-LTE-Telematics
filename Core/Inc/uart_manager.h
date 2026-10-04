#ifndef UART_MANAGER_H
#define UART_MANAGER_H

#include "main.h"
#include "gps.h"
#include "gsm.h"
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
void UART_Manager_Init(void);
void UART_Manager_RxCallback(UART_HandleTypeDef *huart);

#endif
