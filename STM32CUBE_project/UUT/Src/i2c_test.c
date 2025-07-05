/**
 * @file i2c_test.c
 * @author leah
 * @date 27-06-2025
 *
 * @brief Implementation of I2C test
 *
 * I2C testing protocol:
 *  1. I2C1 (Master) sends a bit pattern to I2C2 (Slave).
 *  2. I2C2 (Slave) sends the bit pattern back to I2C1 (Master).
 *  3. The loopbacked bit pattern is compared to the original via CRC.
 *  4. The test succeeds if the CRC codes match.
 */

#include "hw_verif_crc.h"
#include "stm32f7xx_hal.h"
#include "main.h"
#include "hw_verif_sys.h"
#include <stdio.h>
#include <stdint.h>

/*************************
 * GLOBALS               *
 *************************/

extern I2C_HandleTypeDef hi2c1;                   /** I2C1 (Master) handle */
extern I2C_HandleTypeDef hi2c2;                   /** I2C2 (Slave) handle */

extern osMessageQueueId_t i2cQueueHandle;         /** I2C tests to be done */
extern osMessageQueueId_t outMsgQueueHandle;      /** Result queue to responder */

osSemaphoreId_t i2c1TxSem;                         /** TX done on i2c1? */
osSemaphoreId_t i2c1RxSem;                         /** RX done on i2c1? */
osSemaphoreId_t i2c2TxSem;                         /** TX done on i2c2? */
osSemaphoreId_t i2c2RxSem;                         /** RX done on i2c2? */

/*************************
 * FUNCTION DECLARATIONS *
 *************************/

uint8_t I2C_Test_Perform(uint8_t *msg, uint8_t msg_len);

/****************************
 * FUNCTION IMPLEMENTATION  *
 ****************************/

void I2cTestTask(void)
{
	TestData_t test_data;
	OutMsg_t out_msg;
	uint8_t result;

	i2c1TxSem = osSemaphoreNew(1, 0, NULL);
	i2c1RxSem = osSemaphoreNew(1, 0, NULL);
	i2c2TxSem = osSemaphoreNew(1, 0, NULL);
	i2c2RxSem = osSemaphoreNew(1, 0, NULL);

	while (1)
	{
		if(osMessageQueueGet(i2cQueueHandle, &test_data, 0, osWaitForever) == osOK)
		{
			for (uint8_t i=0; i<test_data.n_iter; i++)
			{
				result = I2C_Test_Perform((uint8_t *)test_data.payload, test_data.p_len);
				if (result == TEST_FAILED)
					break;
			}

			// load out_msg
			out_msg.addr = test_data.addr;
			out_msg.port = test_data.port;
			out_msg.test_id = test_data.test_id;
			out_msg.test_result = result;

#ifdef PRINT_TESTS_DEBUG
		    printf("I2C test %s\n", (result == TEST_SUCCESS)? "success" : "failed");
#endif

			// send result to queue
			osMessageQueuePut(outMsgQueueHandle, &out_msg, 0, osWaitForever);
		}
		else osDelay(1);
	}
}

uint8_t I2C_Test_Perform(uint8_t *msg, uint8_t msg_len)
{
	HAL_StatusTypeDef status;

	uint8_t i2c1_rx[MAX_BUF];
	uint8_t i2c2_rx[MAX_BUF];

	osSemaphoreAcquire(i2c1TxSem, 0);
	osSemaphoreAcquire(i2c1RxSem, 0);
	osSemaphoreAcquire(i2c2TxSem, 0);
	osSemaphoreAcquire(i2c2RxSem, 0);

	// Send msg i2c1 -> i2c2
	status = HAL_I2C_Slave_Receive_DMA(&hi2c2, i2c2_rx, msg_len);
	if (status != HAL_OK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("i2c1 -> i2c2 RX failed\n");
#endif
		return TEST_FAILED;
	}

	status = HAL_I2C_Master_Transmit_DMA(&hi2c1, 10<<1, msg, msg_len);
	if (status != HAL_OK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("i2c1 -> i2c2 TX failed\n");
#endif
		return TEST_FAILED;
	}
	if (osSemaphoreAcquire(i2c1TxSem, 10) != osOK || osSemaphoreAcquire(i2c2RxSem, 10) != osOK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("i2c1 -> i2c2 semaphore timeout\n");
#endif
		return TEST_FAILED;
	}

	// Send msg i2c2 -> i2c1
	status = HAL_I2C_Master_Receive_DMA(&hi2c1, 10<<1, i2c1_rx, msg_len);
	if (status != HAL_OK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("i2c2 -> i2c1 RX failed\n");
#endif
		return TEST_FAILED;
	}

	status = HAL_I2C_Slave_Transmit_DMA(&hi2c2, i2c2_rx, msg_len);
	if (status != HAL_OK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("i2c2 -> i2c1 TX failed\n");
#endif
		return TEST_FAILED;
	}
	if (osSemaphoreAcquire(i2c2TxSem, 10) != osOK || osSemaphoreAcquire(i2c1RxSem, 10) != osOK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("i2c2 -> i2c1 semaphore timeout\n");
#endif
		return TEST_FAILED;
	}

	// compare crc
	int crc_result = Match_CRC(msg, msg_len, i2c1_rx, msg_len);
	if (crc_result == CRC_MATCH_OK) return TEST_SUCCESS;

	return TEST_FAILED;
}

/****************************
 * CALLBACK IMPLEMENTATION  *
 ****************************/

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c == &hi2c1) osSemaphoreRelease(i2c1TxSem);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c == &hi2c1) osSemaphoreRelease(i2c1RxSem);
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c == &hi2c2) osSemaphoreRelease(i2c2TxSem);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c == &hi2c2) osSemaphoreRelease(i2c2RxSem);
}
