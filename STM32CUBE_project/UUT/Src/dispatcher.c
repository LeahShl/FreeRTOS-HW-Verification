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

void TestDispatcherTask(void)
{
	while(1)
	{
		InMsg_t in_msg;
		TestData_t test_data;
		osStatus_t status;

		status = osMessageQueueGet(inMsgQueueHandle, &in_msg, 0, osWaitForever);
		if(status == osOK)
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
				status = osMessageQueuePut(uartQueueHandle, &test_data, 0, osWaitForever);
				if (status != osOK)
				{
					LOG_ERR("Dispatcher couldn't put message in uart queue (err code %d)", status);
				}
			}
			if(in_msg.peripheral & TEST_I2C)
			{
				status = osMessageQueuePut(i2cQueueHandle, &test_data, 0, osWaitForever);
				if (status != osOK)
				{
					LOG_ERR("Dispatcher couldn't put message in i2c queue (err code %d)", status);
				}
			}
			if(in_msg.peripheral & TEST_SPI)
			{
				status = osMessageQueuePut(spiQueueHandle, &test_data, 0, osWaitForever);
				if (status != osOK)
				{
					LOG_ERR("Dispatcher couldn't put message in spi queue (err code %d)", status);
				}
			}
			if(in_msg.peripheral & TEST_ADC)
			{
				status = osMessageQueuePut(adcQueueHandle, &test_data, 0, osWaitForever);
				if (status != osOK)
				{
					LOG_ERR("Dispatcher couldn't put message in adc queue (err code %d)", status);
				}
			}
			if(in_msg.peripheral & TEST_TIM)
			{
				status = osMessageQueuePut(timQueueHandle, &test_data, 0, osWaitForever);
				if (status != osOK)
				{
					LOG_ERR("Dispatcher couldn't put message in timer queue (err code %d)", status);
				}
			}
		}
		else
		{
			LOG_ERR("Dispatcher couldn't get message from queue (err code %d)", status);
		}
	}
}
