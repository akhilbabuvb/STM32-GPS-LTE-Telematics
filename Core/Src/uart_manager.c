#include "uart_manager.h"
//#include "gps.h"
//#include "gsm.h"

#include "ring_buffer.h"



void UART_Manager_Init(void)
{
    GPS_Init();
    GSM_Init();
}

void UART_Manager_RxCallback(UART_HandleTypeDef *huart)
{
	  if (huart == &huart1)
	    {
	        GPS_UARTCallback();
	    }
	    else if (huart == &huart3)
	    {
	        GSM_UARTCallback();
	    }

}
