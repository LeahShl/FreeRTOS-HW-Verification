/**
 * @file dispatcher.h
 * @author leah
 * @date 27-06-2025
 *
 * @brief UUT Hardware verification system dispatcher source code
 *
 */
#include "cmsis_os.h"
#include "hw_verif_sys.h"
#include <stdio.h>
#include <string.h>

extern osMessageQueueId_t inMsgQueueHandle;
extern osMessageQueueId_t uartQueueHandle;
extern osMessageQueueId_t i2cQueueHandle;
extern osMessageQueueId_t spiQueueHandle;
extern osMessageQueueId_t adcQueueHandle;
extern osMessageQueueId_t timQueueHandle;

void TestDispatcher(void)
{
	while(1)
	{
		InMsg_t in_msg;
		TestData_t test_data;

		if(osMessageQueueGet(inMsgQueueHandle, &in_msg, 0, osWaitForever) == osOK)
		{
			// load test_data
			test_data.addr = in_msg.addr;
			test_data.port = in_msg.port;
			test_data.test_id = in_msg.test_id;
			test_data.n_iter = in_msg.n_iter;
			test_data.p_len = in_msg.p_len;
			memcpy(test_data.payload, in_msg.payload, in_msg.p_len);

			// send to relevant test queue
			if(in_msg.peripheral & TEST_UART)
			{
				osMessageQueuePut(uartQueueHandle, &test_data, 0, osWaitForever);
			}
			if(in_msg.peripheral & TEST_I2C)
			{
				osMessageQueuePut(i2cQueueHandle, &test_data, 0, osWaitForever);
			}
			if(in_msg.peripheral & TEST_SPI)
			{
				osMessageQueuePut(spiQueueHandle, &test_data, 0, osWaitForever);
			}
			if(in_msg.peripheral & TEST_ADC)
			{
				osMessageQueuePut(adcQueueHandle, &test_data, 0, osWaitForever);
			}
			if(in_msg.peripheral & TEST_TIM)
			{
				osMessageQueuePut(timQueueHandle, &test_data, 0, osWaitForever);
			}
		}
	}
}
