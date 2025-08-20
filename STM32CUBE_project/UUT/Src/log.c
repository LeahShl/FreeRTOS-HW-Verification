/*
 * log.c
 *
 *  Created on: Aug 20, 2025
 *      Author: leah
 */
#include "usart.h"
#include "cmsis_os2.h"

#define UART_LOG &huart3

static osMutexId_t logMutex;

int __io_putchar(int ch) {
	HAL_UART_Transmit(UART_LOG, (uint8_t*) &ch, 1, 0xFFFF);
	return ch;
}

int _write(int file, char *ptr, int len) {
	if (logMutex == NULL) logMutex = osMutexNew(NULL);

	char *cr = "\r";

	while (osMutexAcquire(logMutex, 10) != osOK);
	HAL_UART_Transmit(UART_LOG, (uint8_t*) ptr, len, 0xFFFF);
	HAL_UART_Transmit(UART_LOG, (uint8_t*) cr, 1, 0xFFFF);
	osMutexRelease(logMutex);

	return len + 1;
}
