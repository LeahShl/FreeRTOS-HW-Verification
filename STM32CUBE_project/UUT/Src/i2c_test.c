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

/**
 * @brief DMA syncronization
 */
volatile uint8_t i2c1_tx_done;                    /** I2C1 (Master) transmit completed */
volatile uint8_t i2c1_rx_done;                    /** I2C1 (Master) receive completed */
volatile uint8_t i2c2_tx_done;                    /** I2C2 (Slave) transmit completed */
volatile uint8_t i2c2_rx_done;                    /** I2C2 (Slave) receive completed */

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

	while (1)
	{
		printf("i2c waiting for messages\n");
		if(osMessageQueueGet(i2cQueueHandle, &test_data, 0, osWaitForever) == osOK)
		{
			printf("i2c received test ID: %lu\n", test_data.test_id);

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

			// send result to queue
			osMessageQueuePut(outMsgQueueHandle, &out_msg, 0, osWaitForever);
		}
		else osDelay(1);
	}
}

uint8_t I2C_Test_Perform(uint8_t *msg, uint8_t msg_len)
{
#ifdef PRINT_TESTS_DEBUG
	printf("Performing i2c test\n");
#endif

	HAL_StatusTypeDef status;

	i2c1_tx_done = 0;
	i2c1_rx_done = 0;
	i2c2_tx_done = 0;
	i2c2_rx_done = 0;

	uint8_t i2c1_rx[MAX_BUF];
	uint8_t i2c2_rx[MAX_BUF];

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
	while (!i2c2_rx_done || !i2c1_tx_done);

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
	while (!i2c1_rx_done || !i2c2_tx_done);

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
	if (hi2c == &hi2c1) i2c1_tx_done = 1;
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c == &hi2c1) i2c1_rx_done = 1;
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c == &hi2c2) i2c2_tx_done = 1;
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c == &hi2c2) i2c2_rx_done = 1;
}
