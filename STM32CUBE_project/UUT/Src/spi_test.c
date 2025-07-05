/**
 * @file spi_test.c
 * @author leah
 * @date 28-06-2025
 *
 * @brief Implementation of SPI test
 *
 * SPI testing protocol:
 *  1. SPI1 (Master) sends a bit pattern to SPI4 (Slave).
 *  2. SPI4 (Slave) sends the bit pattern back to SPI1 (Master).
 *  3. The loopbacked bit pattern is compared to the original via CRC.
 *  4. The test succeeds if the CRC codes match.
 */

#include "hw_verif_crc.h"
#include "stm32f7xx_hal.h"
#include "main.h"
#include "hw_verif_sys.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/*************************
 * GLOBALS               *
 *************************/

extern SPI_HandleTypeDef hspi1;                   /** SPI1 (Master) handle */
extern SPI_HandleTypeDef hspi4;                   /** SPI4 (Slave) handle */

extern osMessageQueueId_t spiQueueHandle;         /** SPI tests to be done */
extern osMessageQueueId_t outMsgQueueHandle;      /** Result queue to responder */

osSemaphoreId_t spi1TxSem;                         /** TX done on spi1? */
osSemaphoreId_t spi1RxSem;                         /** RX done on spi1? */
osSemaphoreId_t spi4TxSem;                         /** TX done on spi4? */
osSemaphoreId_t spi4RxSem;                         /** RX done on spi4? */

/*************************
 * FUNCTION DECLARATIONS *
 *************************/

uint8_t SPI_Test_Perform(uint8_t *msg, uint8_t msg_len);

/****************************
 * FUNCTION IMPLEMENTATION  *
 ****************************/

void SpiTestTask(void)
{
	TestData_t test_data;
	OutMsg_t out_msg;
	uint8_t result;

	spi1TxSem = osSemaphoreNew(1, 0, NULL);
	spi1RxSem = osSemaphoreNew(1, 0, NULL);
	spi4TxSem = osSemaphoreNew(1, 0, NULL);
	spi4RxSem = osSemaphoreNew(1, 0, NULL);

	while (1)
	{
		if(osMessageQueueGet(spiQueueHandle, &test_data, 0, osWaitForever) == osOK)
		{
			for (uint8_t i=0; i<test_data.n_iter; i++)
			{
				result = SPI_Test_Perform((uint8_t *)test_data.payload, test_data.p_len);
				if (result == TEST_FAILED)
					break;
			}

			// load out_msg
			out_msg.addr = test_data.addr;
			out_msg.port = test_data.port;
			out_msg.test_id = test_data.test_id;
			out_msg.test_result = result;

#ifdef PRINT_TESTS_DEBUG
		    printf("SPI test %s\n", result? "success" : "failed");
#endif

			// send result to queue
			osMessageQueuePut(outMsgQueueHandle, &out_msg, 0, osWaitForever);
		}
		else osDelay(1);
	}
}

uint8_t SPI_Test_Perform(uint8_t *msg, uint8_t msg_len)
{
	HAL_StatusTypeDef status;

	uint8_t spi1_rx[MAX_BUF];
	uint8_t spi4_rx_tx[MAX_BUF];

	// send msg spi1 -> spi4
	status = HAL_SPI_Receive_DMA(&hspi4, spi4_rx_tx, msg_len);
	if (status != HAL_OK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("spi1 -> spi4 RX failed\n");
#endif
		return TEST_FAILED;
	}
	status = HAL_SPI_Transmit_DMA(&hspi1, msg, msg_len);
	if (status != HAL_OK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("spi1 -> spi4 TX failed\n");
#endif
		return TEST_FAILED;
	}
	if (osSemaphoreAcquire(spi1TxSem, 10) != osOK || osSemaphoreAcquire(spi4RxSem, 10) != osOK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("spi1 -> spi4 semaphore timeout\n");
#endif
		return TEST_FAILED;
	}

	// send msg spi4 -> spi1
	status = HAL_SPI_Transmit_DMA(&hspi4, spi4_rx_tx, msg_len);
	if (status != HAL_OK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("spi4 -> spi1 TX failed\n");
#endif
		return TEST_FAILED;
	}
	status = HAL_SPI_Receive_DMA(&hspi1, spi1_rx, msg_len);
	if (status != HAL_OK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("spi4 -> spi1 RX failed\n");
#endif
		return TEST_FAILED;
	}
	if (osSemaphoreAcquire(spi4TxSem, 10) != osOK || osSemaphoreAcquire(spi1RxSem, 10) != osOK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("spi4 -> spi1 semaphore timeout\n");
#endif
		return TEST_FAILED;
	}

	// Compare CRC
	int crc_result = Match_CRC(msg, msg_len, spi1_rx, msg_len);
	if (crc_result == CRC_MATCH_OK) return TEST_SUCCESS;

	return TEST_FAILED;
}

/****************************
 * CALLBACK IMPLEMENTATION  *
 ****************************/

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == &hspi1) osSemaphoreRelease(spi1TxSem);
	if (hspi == &hspi4) osSemaphoreRelease(spi4TxSem);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == &hspi1) osSemaphoreRelease(spi1RxSem);
	if (hspi == &hspi4) osSemaphoreRelease(spi4RxSem);
}


