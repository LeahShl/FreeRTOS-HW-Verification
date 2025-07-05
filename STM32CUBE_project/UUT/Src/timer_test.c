/**
 * @file timer_test.c
 * @author leah
 * @date 28-06-2025
 *
 * @brief Implementation of timer test
 *
 * @attention N_SAMPLES will influence test duration significantly.
 *
 * Timer test protocol:
 *  1. TIM2 (advanced timer) is set to count up.
 *  2. TIM6 (basic timer) is set to restart every 10ms.
 *  3. TIM2 count is sampled every time TIM6 restarts, until N_SAMPLES(=10) are retrieved.
 *  4. A single test passes if all samples are spaced 54,000 counts from each other.
 */

#include "stm32f7xx_hal.h"
#include "main.h"
#include "hw_verif_sys.h"
#include <stdio.h>
#include <stdint.h>

/*************************
 * MACROS                *
 *************************/

#define N_SAMPLES 10                          /** Number of samples */
#define EXPECTED_INTERVAL 54000               /** Expected interval between samples */
#define TIM_ERR_TOLERANCE 4000                /** Error tolerance for sample readings */

/*************************
 * GLOBALS               *
 *************************/

extern TIM_HandleTypeDef htim2;               /** Advanced timer handle */
extern TIM_HandleTypeDef htim6;               /** Basic timer handle */
extern DMA_HandleTypeDef hdma_tim6_up;        /** DMA handle */
extern osMessageQueueId_t timQueueHandle;     /** Timer tests to be done */
extern osMessageQueueId_t outMsgQueueHandle;  /** Result queue to responder */

osSemaphoreId_t testDoneSem;                  /** Is the sampling done yet? */

uint32_t tim2_samples[N_SAMPLES];             /** Buffer for advanced timer samples */

/*************************
 * FUNCTION DECLARATIONS *
 *************************/

uint8_t TIM_Test_Perform(void);

void TIM6_UP_DMA_XferCplt(DMA_HandleTypeDef *hdma);

/****************************
 * FUNCTION IMPLEMENTATION  *
 ****************************/

void TimTestTask(void)
{
	TestData_t test_data;
	OutMsg_t out_msg;
	uint8_t result;

	testDoneSem = osSemaphoreNew(1, 0, NULL);

	while (1)
	{
		if(osMessageQueueGet(timQueueHandle, &test_data, 0, osWaitForever) == osOK)
		{
			for (uint8_t i=0; i<test_data.n_iter; i++)
			{
				result = TIM_Test_Perform();
				if (result == TEST_FAILED)
					break;
			}

			// load out_msg
			out_msg.addr = test_data.addr;
			out_msg.port = test_data.port;
			out_msg.test_id = test_data.test_id;
			out_msg.test_result = result;

#ifdef PRINT_TESTS_DEBUG
		    printf("Timer test %s\n", result? "success" : "failed");
#endif

			// send result to queue
			osMessageQueuePut(outMsgQueueHandle, &out_msg, 0, osWaitForever);
		}
		else osDelay(1);
	}
}

uint8_t TIM_Test_Perform(void)
{
	static uint8_t callback_registered = 0;

	if (!callback_registered)
	{
		HAL_DMA_RegisterCallback(&hdma_tim6_up, HAL_DMA_XFER_CPLT_CB_ID, TIM6_UP_DMA_XferCplt);
		callback_registered = 1;
	}

	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim6);

	if(HAL_DMA_Start_IT(&hdma_tim6_up, (uint32_t)&htim2.Instance->CNT,
			            (uint32_t)tim2_samples, N_SAMPLES) != HAL_OK)
	{
#ifdef PRINT_TESTS_DEBUG
	printf("Timer test: DMA init error\n");
#endif
		return TEST_FAILED;
	}
    __HAL_TIM_ENABLE_DMA(&htim6, TIM_DMA_UPDATE);

    if (osSemaphoreAcquire(testDoneSem, osWaitForever) != osOK)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("Timer test: timeout waiting for DMA complete\n");
#endif
		HAL_TIM_Base_Stop(&htim6);
		HAL_TIM_Base_Stop(&htim2);
		return TEST_FAILED;
	}

	HAL_TIM_Base_Stop(&htim6);
	HAL_TIM_Base_Stop(&htim2);

#ifdef PRINT_TESTS_DEBUG
	printf("Sampling complete\n");
#endif
	for (int i = 1; i < N_SAMPLES; i++)
	{
#ifdef PRINT_TESTS_DEBUG
		printf("Sample[%d] - sample[%d] = %lu\n", i, i-1,
			   tim2_samples[i] - tim2_samples[i-1]);
#endif

		if (tim2_samples[i] - tim2_samples[i-1] < EXPECTED_INTERVAL - TIM_ERR_TOLERANCE ||
			tim2_samples[i] - tim2_samples[i-1] > EXPECTED_INTERVAL + TIM_ERR_TOLERANCE)
			return TEST_FAILED;
	}

	return TEST_SUCCESS;
}

/****************************
 * CALLBACK IMPLEMENTATION  *
 ****************************
 *
 * The original non freeRTOS code contained the following callback
 * implementation:
 *
 * void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
 * {
 * 	   if (htim == &htim6) tim6_count++;
 * }
 *
 * Using the same strategy with FreeRTOS will break the code, so I implemented
 * here a different strategy.
 */

void TIM6_UP_DMA_XferCplt(DMA_HandleTypeDef *hdma)
{
    if (hdma == &hdma_tim6_up)
    {
    	osSemaphoreRelease(testDoneSem);
    }
}
