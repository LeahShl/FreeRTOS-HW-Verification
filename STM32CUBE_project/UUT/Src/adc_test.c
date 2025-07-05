/**
 * @file adc_test.c
 * @author leah
 * @date 19-06-2025
 *
 * @brief Implementation of ADC test
 *
 * ADC test protocol:
 *  1) ADC1/0 is connected to 3v3
 *  2) ADC reads value
 *  3) Test succeeds if value is within a specified tolerance
 */
#include "stm32f7xx_hal.h"
#include "main.h"
#include "hw_verif_sys.h"
#include <stdio.h>
#include <stdint.h>

/*************************
 * MACROS                *
 *************************/

#define EXPECTED_3V3 4095                      /** Expected reading from a 3.3v source */
#define ERR_TOLERANCE_3V3 95                   /** Error tolerance for 3.3v reading */

/*************************
 * GLOBALS               *
 *************************/

extern ADC_HandleTypeDef hadc1;                /** ADC1 handle */

extern osMessageQueueId_t adcQueueHandle;      /** ADC tests to be done */
extern osMessageQueueId_t outMsgQueueHandle;   /** Result queue to responder */

osSemaphoreId_t adcReadySem;                   /** is ADC ready? */

uint16_t adc_buf[MAX_BUF];                     /** Buffer for ADC samples */

/*************************
 * FUNCTION DECLARATIONS *
 *************************/

uint8_t ADC_Test_Perform(void);

/****************************
 * FUNCTION IMPLEMENTATION  *
 ****************************/

void AdcTestTask(void)
{
	TestData_t test_data;
	OutMsg_t out_msg;
	uint8_t result;
	osStatus_t status;

	adcReadySem = osSemaphoreNew(1, 0, NULL);

	while (1)
	{
		status = osMessageQueueGet(adcQueueHandle, &test_data, 0, osWaitForever);
		if(status == osOK)
		{
			for (uint8_t i=0; i<test_data.n_iter; i++)
			{
				result = ADC_Test_Perform();
				if (result == TEST_FAILED)
					break;
			}

			// load out_msg
			out_msg.addr = test_data.addr;
			out_msg.port = test_data.port;
			out_msg.test_id = test_data.test_id;
			out_msg.test_result = result;

#ifdef PRINT_TESTS_DEBUG
		    printf("ADC test %s\n", (result == TEST_SUCCESS)? "success" : "failed");
#endif

			// send result to queue
			osMessageQueuePut(outMsgQueueHandle, &out_msg, 0, osWaitForever);
		}
		else if (status == osErrorTimeout)
		{
			osDelay(1);
		}
		else
		{
			printf("adc msg read error: %d\n", status);
			osDelay(1);
		}
	}
}

uint8_t ADC_Test_Perform(void)
{
	HAL_StatusTypeDef status;

	status = HAL_ADC_Start_DMA(&hadc1,(uint32_t*)adc_buf,1);
	if (status != HAL_OK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("ADC_Start_DMA failed\n");
#endif
		return TEST_FAILED;
	}

	if (osSemaphoreAcquire(adcReadySem, 10) != osOK) {
#ifdef PRINT_TESTS_DEBUG
		printf("ADC semaphore timeout\n");
#endif
		return TEST_FAILED;
	}

#ifdef PRINT_TESTS_DEBUG2
	printf("adc value = %d\n",adc_buf[0]);
#endif

	if(adc_buf[0] >= EXPECTED_3V3 - ERR_TOLERANCE_3V3) return TEST_SUCCESS;

	return TEST_FAILED;
}

/****************************
 * CALLBACK IMPLEMENTATION  *
 ****************************/

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	osSemaphoreRelease(adcReadySem);
	if(HAL_ADC_Stop_DMA(&hadc1) != HAL_OK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("ADC_Stop_DMA failed\n");
#endif
		Error_Handler();
	}
}
