/*
 * log.c
 *
 *  Created on: Aug 20, 2025
 *      Author: leah
 */
#include "usart.h"
#include "cmsis_os2.h"
#include <stdarg.h>

#define UART_LOG &huart3

static osMutexId_t logMutex;
static osStatus_t status;

void init_log_mtx()
{
	logMutex = osMutexNew(NULL);

	if (logMutex == NULL)
		Error_Handler();
}

void safe_printf(const char* fmt, ...)
{
	status = osMutexAcquire(logMutex, osWaitForever);
    if (status == osOK)
    {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        osMutexRelease(logMutex);
    }
    else
    {
    	Error_Handler();
    }
}

int __io_putchar(int ch) {
	HAL_UART_Transmit(UART_LOG, (uint8_t*)&ch, 1, 0xFFFF);
	return ch;
}

int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(UART_LOG, (uint8_t*) ptr, len, 0xFFFF);
    return len;
}
