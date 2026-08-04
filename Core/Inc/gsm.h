#ifndef __GSM_H
#define __GSM_H

#include "main.h"
#include "ring_buffer.h"

void GSM_Init(void);
void GSM_UARTCallback(void);
void GSM_Task(void);
HAL_StatusTypeDef GSM_SendCommand(char *command);

#endif
