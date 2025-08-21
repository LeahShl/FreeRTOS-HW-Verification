/**
 * @file uart_test.c
 * @author leah
 * @date 27-06-2025
 * 
 * @brief Implementation of UART test
 * 
 * UART testing protocol:
 *  1. UART4 sends a bit pattern to UART5.
 *  2. UART5 sends the bit pattern back to UART4.
 *  3. The loopbacked bit pattern is compared to the original via CRC.
 *  4. The test succeeds if the CRC codes match.
 * 
 * DMA is only implemented on RX pins in order to save DMA streams for 
 * more important applications.
 */

#include "hw_verif_crc.h"
#include "stm32f7xx_hal.h"
#include "main.h"
#include "hw_verif_sys.h"
#include "cmsis_os.h"
#include "log.h"
#include <stdio.h>
#include <stdint.h>

/*************************
 * GLOBALS               *
 *************************/

extern UART_HandleTypeDef huart4;                   /** UART4 handle */
extern UART_HandleTypeDef huart5;                   /** UART5 handle */

extern osMessageQueueId_t uartQueueHandle;          /** UART tests to be done */
extern osMessageQueueId_t outMsgQueueHandle;        /** Result queue to responder */

osSemaphoreId_t uart4RxSem;                         /** RX done on uart4? */
osSemaphoreId_t uart5RxSem;                         /** RX done on uart5? */

/*************************
 * FUNCTION DECLARATIONS *
 *************************/

uint8_t UART_Test_Perform(uint8_t *msg, uint8_t msg_len);

/****************************
 * FUNCTION IMPLEMENTATION  *
 ****************************/

void UartTestTask(void)
{
	TestData_t test_data;
	OutMsg_t out_msg;
	uint8_t result;
	osStatus_t status;
	uint8_t i;


	uart4RxSem = osSemaphoreNew(1, 0, NULL);
	if(uart4RxSem == NULL)
	{
		LOG_ERR("Could not create uart4 rx semaphore");
	}

	uart5RxSem = osSemaphoreNew(1, 0, NULL);
	if(uart5RxSem == NULL)
	{
		LOG_ERR("Could not create uart5 rx semaphore");
	}

	while (1)
	{
		status = osMessageQueueGet(uartQueueHandle, &test_data, 0, osWaitForever);
		if(status == osOK)
		{
			for (i=0; i<test_data.n_iter; i++)
			{
                result = UART_Test_Perform((uint8_t *)test_data.payload, test_data.p_len);
				if (result == TEST_FAILED)
					break;
			}

			// load out_msg
			out_msg.addr = test_data.addr;
			out_msg.port = test_data.port;
			out_msg.test_id = test_data.test_id;
			out_msg.test_result = result;

			LOG_INFO("UART test %s", (result == TEST_SUCCESS)? "success" : "failed");

			// send result to queue
			status = osMessageQueuePut(outMsgQueueHandle, &out_msg, 0, osWaitForever);
			if(status != osOK)
			{
				LOG_ERR("UART test couldn't put messages into result queue (err code: %d)", status);
			}
		}
		else
		{
			LOG_ERR("Couldn't get message from uart test queue (err code: %d)", status);
		}
	}


}

uint8_t UART_Test_Perform(uint8_t *msg, uint8_t msg_len)
{
	uint8_t uart4_rx[MAX_BUF];
	uint8_t uart5_rx[MAX_BUF];
	osStatus_t status;
	HAL_StatusTypeDef hstatus;

	osSemaphoreAcquire(uart4RxSem, 0);
	osSemaphoreAcquire(uart5RxSem, 0);

	HAL_UART_Abort(&huart4);
	HAL_UART_Abort(&huart5);

	// Send msg uart4 -> uart5
	hstatus = HAL_UART_Receive_DMA(&huart5, uart5_rx, msg_len);
	if (hstatus != HAL_OK)
	{
		LOG_ERR("uart4 -> uart5 RX failed (err code: %d)", hstatus);
		return TEST_FAILED;
	}
	hstatus = HAL_UART_Transmit_IT(&huart4, msg, msg_len);
	if (hstatus != HAL_OK)
	{
		LOG_ERR("uart4 -> uart5 TX failed (err code: %d)", hstatus);
		return TEST_FAILED;
	}
	status = osSemaphoreAcquire(uart5RxSem, 10);
	if (status != osOK) {
	    LOG_ERR("uart5 RX semaphore acquire error (err code: %d)", status);
	    return TEST_FAILED;
	}

	HAL_UART_Abort(&huart4);
	HAL_UART_Abort(&huart5);

	// Send msg uart5 -> uart4
	hstatus = HAL_UART_Receive_DMA(&huart4, uart4_rx, msg_len);
	if (hstatus != HAL_OK)
	{
		LOG_ERR("uart5 -> uart4 RX failed (err code: %d)", hstatus);
		return TEST_FAILED;
	}
	hstatus = HAL_UART_Transmit_IT(&huart5, uart5_rx, msg_len);
	if (hstatus != HAL_OK)
	{
		LOG_ERR("uart5 -> uart4 TX failed (err code: %d)", hstatus);
		return TEST_FAILED;
	}
    status = osSemaphoreAcquire(uart4RxSem, 10);
	if (status != osOK) {
	    LOG_ERR("uart4 RX semaphore timeout (err code: %d)", status);
	    return TEST_FAILED;
	}

	// compare crc
	int crc_result = Match_CRC(msg, msg_len, uart4_rx, msg_len);
	if (crc_result == CRC_MATCH_OK) return TEST_SUCCESS;

	return TEST_FAILED;
}

/****************************
 * CALLBACK IMPLEMENTATION  *
 ****************************/

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	osStatus_t status;

    if (huart == &huart4) {
    	status = osSemaphoreRelease(uart4RxSem);
		if(status != osOK)
		{
			LOG_ERR("UART callback couldn't release uart4 rx semaphore");
		}
    }
    if (huart == &huart5)
    {
    	status = osSemaphoreRelease(uart5RxSem);
		if(status != osOK)
		{
			LOG_ERR("UART callback couldn't release uart5 rx semaphore");
		}
    }
}

