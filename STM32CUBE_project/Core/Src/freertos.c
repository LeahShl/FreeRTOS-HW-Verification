/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hw_verif_sys.h"
#include "hw_verif_udp.h"
#include "dispatcher.h"
#include "uart_test.h"
#include "i2c_test.h"
#include "spi_test.h"
#include "adc_test.h"
#include "timer_test.h"

#include "lwip/stats.h"

#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal3,
};
/* Definitions for UDPListenerTask */
osThreadId_t UDPListenerTaskHandle;
const osThreadAttr_t UDPListenerTask_attributes = {
  .name = "UDPListenerTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal2,
};
/* Definitions for DispatcherTask */
osThreadId_t DispatcherTaskHandle;
const osThreadAttr_t DispatcherTask_attributes = {
  .name = "DispatcherTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for uartTestTask */
osThreadId_t uartTestTaskHandle;
const osThreadAttr_t uartTestTask_attributes = {
  .name = "uartTestTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for i2cTestTask */
osThreadId_t i2cTestTaskHandle;
const osThreadAttr_t i2cTestTask_attributes = {
  .name = "i2cTestTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for spiTestTask */
osThreadId_t spiTestTaskHandle;
const osThreadAttr_t spiTestTask_attributes = {
  .name = "spiTestTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for adcTestTask */
osThreadId_t adcTestTaskHandle;
const osThreadAttr_t adcTestTask_attributes = {
  .name = "adcTestTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for timTestTask */
osThreadId_t timTestTaskHandle;
const osThreadAttr_t timTestTask_attributes = {
  .name = "timTestTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UDPResponderTas */
osThreadId_t UDPResponderTasHandle;
const osThreadAttr_t UDPResponderTas_attributes = {
  .name = "UDPResponderTas",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal2,
};
/* Definitions for inMsgQueue */
osMessageQueueId_t inMsgQueueHandle;
const osMessageQueueAttr_t inMsgQueue_attributes = {
  .name = "inMsgQueue"
};
/* Definitions for outMsgQueue */
osMessageQueueId_t outMsgQueueHandle;
const osMessageQueueAttr_t outMsgQueue_attributes = {
  .name = "outMsgQueue"
};
/* Definitions for uartQueue */
osMessageQueueId_t uartQueueHandle;
const osMessageQueueAttr_t uartQueue_attributes = {
  .name = "uartQueue"
};
/* Definitions for i2cQueue */
osMessageQueueId_t i2cQueueHandle;
const osMessageQueueAttr_t i2cQueue_attributes = {
  .name = "i2cQueue"
};
/* Definitions for spiQueue */
osMessageQueueId_t spiQueueHandle;
const osMessageQueueAttr_t spiQueue_attributes = {
  .name = "spiQueue"
};
/* Definitions for adcQueue */
osMessageQueueId_t adcQueueHandle;
const osMessageQueueAttr_t adcQueue_attributes = {
  .name = "adcQueue"
};
/* Definitions for timQueue */
osMessageQueueId_t timQueueHandle;
const osMessageQueueAttr_t timQueue_attributes = {
  .name = "timQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTaskUdpListener(void *argument);
void StartTaskDispatcher(void *argument);
void StartTaskUartTest(void *argument);
void StartTaskI2cTest(void *argument);
void StartTaskSpiTest(void *argument);
void StartTaskAdcTest(void *argument);
void StartTaskTimTest(void *argument);
void StartResponseTask(void *argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
#ifdef PRINT_TESTS_DEBUG
	printf("stack overflow from task '%s'\n", pcTaskName);
#endif
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
}
/* USER CODE END 5 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of inMsgQueue */
  inMsgQueueHandle = osMessageQueueNew (16, sizeof(InMsg_t), &inMsgQueue_attributes);

  /* creation of outMsgQueue */
  outMsgQueueHandle = osMessageQueueNew (16, sizeof(OutMsg_t), &outMsgQueue_attributes);

  /* creation of uartQueue */
  uartQueueHandle = osMessageQueueNew (8, sizeof(TestData_t), &uartQueue_attributes);

  /* creation of i2cQueue */
  i2cQueueHandle = osMessageQueueNew (8, sizeof(TestData_t), &i2cQueue_attributes);

  /* creation of spiQueue */
  spiQueueHandle = osMessageQueueNew (8, sizeof(TestData_t), &spiQueue_attributes);

  /* creation of adcQueue */
  adcQueueHandle = osMessageQueueNew (8, sizeof(TestData_t), &adcQueue_attributes);

  /* creation of timQueue */
  timQueueHandle = osMessageQueueNew (8, sizeof(TestData_t), &timQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of UDPListenerTask */
  UDPListenerTaskHandle = osThreadNew(StartTaskUdpListener, NULL, &UDPListenerTask_attributes);

  /* creation of DispatcherTask */
  DispatcherTaskHandle = osThreadNew(StartTaskDispatcher, NULL, &DispatcherTask_attributes);

  /* creation of uartTestTask */
  uartTestTaskHandle = osThreadNew(StartTaskUartTest, NULL, &uartTestTask_attributes);

  /* creation of i2cTestTask */
  i2cTestTaskHandle = osThreadNew(StartTaskI2cTest, NULL, &i2cTestTask_attributes);

  /* creation of spiTestTask */
  spiTestTaskHandle = osThreadNew(StartTaskSpiTest, NULL, &spiTestTask_attributes);

  /* creation of adcTestTask */
  adcTestTaskHandle = osThreadNew(StartTaskAdcTest, NULL, &adcTestTask_attributes);

  /* creation of timTestTask */
  timTestTaskHandle = osThreadNew(StartTaskTimTest, NULL, &timTestTask_attributes);

  /* creation of UDPResponderTas */
  UDPResponderTasHandle = osThreadNew(StartResponseTask, NULL, &UDPResponderTas_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTaskUdpListener */
/**
* @brief Function implementing the UDPListenerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskUdpListener */
void StartTaskUdpListener(void *argument)
{
  /* USER CODE BEGIN StartTaskUdpListener */
  UDP_Listen(); // loops here
  /* USER CODE END StartTaskUdpListener */
}

/* USER CODE BEGIN Header_StartTaskDispatcher */
/**
* @brief Function implementing the DispatcherTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskDispatcher */
void StartTaskDispatcher(void *argument)
{
  /* USER CODE BEGIN StartTaskDispatcher */
  TestDispatcherTask(); // loops here
  /* USER CODE END StartTaskDispatcher */
}

/* USER CODE BEGIN Header_StartTaskUartTest */
/**
* @brief Function implementing the uartTestTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskUartTest */
void StartTaskUartTest(void *argument)
{
  /* USER CODE BEGIN StartTaskUartTest */
  UartTestTask(); // loops here
  /* USER CODE END StartTaskUartTest */
}

/* USER CODE BEGIN Header_StartTaskI2cTest */
/**
* @brief Function implementing the i2cTestTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskI2cTest */
void StartTaskI2cTest(void *argument)
{
  /* USER CODE BEGIN StartTaskI2cTest */
	I2cTestTask(); // loops here
  /* USER CODE END StartTaskI2cTest */
}

/* USER CODE BEGIN Header_StartTaskSpiTest */
/**
* @brief Function implementing the spiTestTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskSpiTest */
void StartTaskSpiTest(void *argument)
{
  /* USER CODE BEGIN StartTaskSpiTest */
  SpiTestTask(); // loops here
  /* USER CODE END StartTaskSpiTest */
}

/* USER CODE BEGIN Header_StartTaskAdcTest */
/**
* @brief Function implementing the adcTestTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskAdcTest */
void StartTaskAdcTest(void *argument)
{
  /* USER CODE BEGIN StartTaskAdcTest */
  AdcTestTask(); // loops here
  /* USER CODE END StartTaskAdcTest */
}

/* USER CODE BEGIN Header_StartTaskTimTest */
/**
* @brief Function implementing the timTestTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskTimTest */
void StartTaskTimTest(void *argument)
{
  /* USER CODE BEGIN StartTaskTimTest */
  TimTestTask(); // loops here
  /* USER CODE END StartTaskTimTest */
}

/* USER CODE BEGIN Header_StartResponseTask */
/**
* @brief Function implementing the responseTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartResponseTask */
void StartResponseTask(void *argument)
{
  /* USER CODE BEGIN StartResponseTask */
  UDP_Response(); // Loops here
  /* USER CODE END StartResponseTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

