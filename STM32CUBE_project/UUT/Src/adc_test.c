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
#define ERR_TOLERANCE_3V3 200                  /** Error tolerance for 3.3v reading */

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

		    LOG_INFO("ADC test %s", (result == TEST_SUCCESS)? "success" : "failed");

			// send result to queue
			status = osMessageQueuePut(outMsgQueueHandle, &out_msg, 0, osWaitForever);
			if (status != osOK)
			{
				LOG_ERR("ADC tester couldn't put result in msg queue (err code %d)", status);
			}
		}
		else
		{
			LOG_ERR("adc msg read error: %d", status);
		}
	}
}

uint8_t ADC_Test_Perform(void)
{
	HAL_StatusTypeDef hstatus;
	osStatus_t status;

	hstatus = HAL_ADC_Start_DMA(&hadc1,(uint32_t*)adc_buf,1);
	if (hstatus != HAL_OK)
	{
		LOG_ERR("ADC_Start_DMA failed (err code %d)", hstatus);
		return TEST_FAILED;
	}

	status = osSemaphoreAcquire(adcReadySem, 10);
	if (status != osOK) {
		LOG_ERR("ADC semaphore timeout (err code %d)", status);
		return TEST_FAILED;
	}

	LOG_INFO("adc value = %d", adc_buf[0]);

	if (adc_buf[0] >= EXPECTED_3V3 - ERR_TOLERANCE_3V3) return TEST_SUCCESS;

	else LOG_INFO("bad adc value = %d",adc_buf[0]);

	return TEST_FAILED;
}

/****************************
 * CALLBACK IMPLEMENTATION  *
 ****************************/

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	osStatus_t status = osSemaphoreRelease(adcReadySem);
	if (status != osOK)
	{
		LOG_ERR("osSemaphoreRelease failed with error code %d", status);
	}

	HAL_StatusTypeDef hstatus = HAL_ADC_Stop_DMA(&hadc1);
	if(hstatus != HAL_OK)
	{
		LOG_ERR("ADC_Stop_DMA failed with error code %d", hstatus);
		Error_Handler();
	}
}
