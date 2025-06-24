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
osThreadId defaultTaskHandle;
osThreadId UDPListenerTaskHandle;
osThreadId hwVerifTaskHandle;
osThreadId uartTestTaskHandle;
osThreadId i2cTestTaskHandle;
osThreadId spiTestTaskHandle;
osThreadId adcTestTaskHandle;
osThreadId timTestTaskHandle;
osThreadId UDPResponderTasHandle;
osThreadId loggerTaskHandle;
osMessageQId inMsgQueueHandle;
osMessageQId outMsgQueueHandle;
osMessageQId uartQueueHandle;
osMessageQId i2cQueueHandle;
osMessageQId spiQueueHandle;
osMessageQId adcQueueHandle;
osMessageQId timQueueHandle;
osMutexId netconnMutexHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTaskListen(void const * argument);
void StartTaskHwVerif(void const * argument);
void StartTaskUartTest(void const * argument);
void StartTaskI2cTest(void const * argument);
void StartTaskSpiTest(void const * argument);
void StartTaskAdcTest(void const * argument);
void StartTaskTimTest(void const * argument);
void StartResponseTask(void const * argument);
void StartLoggerTask(void const * argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  printf("Starting FreeRTOS...\n");
  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of netconnMutex */
  osMutexDef(netconnMutex);
  netconnMutexHandle = osMutexCreate(osMutex(netconnMutex));

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
  /* definition and creation of inMsgQueue */
  osMessageQDef(inMsgQueue, 16, InMsg_t);
  inMsgQueueHandle = osMessageCreate(osMessageQ(inMsgQueue), NULL);

  /* definition and creation of outMsgQueue */
  osMessageQDef(outMsgQueue, 16, OutMsg_t);
  outMsgQueueHandle = osMessageCreate(osMessageQ(outMsgQueue), NULL);

  /* definition and creation of uartQueue */
  osMessageQDef(uartQueue, 8, TestData_t);
  uartQueueHandle = osMessageCreate(osMessageQ(uartQueue), NULL);

  /* definition and creation of i2cQueue */
  osMessageQDef(i2cQueue, 8, TestData_t);
  i2cQueueHandle = osMessageCreate(osMessageQ(i2cQueue), NULL);

  /* definition and creation of spiQueue */
  osMessageQDef(spiQueue, 8, TestData_t);
  spiQueueHandle = osMessageCreate(osMessageQ(spiQueue), NULL);

  /* definition and creation of adcQueue */
  osMessageQDef(adcQueue, 8, TestData_t);
  adcQueueHandle = osMessageCreate(osMessageQ(adcQueue), NULL);

  /* definition and creation of timQueue */
  osMessageQDef(timQueue, 8, TestData_t);
  timQueueHandle = osMessageCreate(osMessageQ(timQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of UDPListenerTask */
  osThreadDef(UDPListenerTask, StartTaskListen, osPriorityAboveNormal, 0, 512);
  UDPListenerTaskHandle = osThreadCreate(osThread(UDPListenerTask), NULL);

  /* definition and creation of hwVerifTask */
  osThreadDef(hwVerifTask, StartTaskHwVerif, osPriorityNormal, 0, 256);
  hwVerifTaskHandle = osThreadCreate(osThread(hwVerifTask), NULL);

  /* definition and creation of uartTestTask */
  osThreadDef(uartTestTask, StartTaskUartTest, osPriorityNormal, 0, 256);
  uartTestTaskHandle = osThreadCreate(osThread(uartTestTask), NULL);

  /* definition and creation of i2cTestTask */
  osThreadDef(i2cTestTask, StartTaskI2cTest, osPriorityNormal, 0, 256);
  i2cTestTaskHandle = osThreadCreate(osThread(i2cTestTask), NULL);

  /* definition and creation of spiTestTask */
  osThreadDef(spiTestTask, StartTaskSpiTest, osPriorityNormal, 0, 256);
  spiTestTaskHandle = osThreadCreate(osThread(spiTestTask), NULL);

  /* definition and creation of adcTestTask */
  osThreadDef(adcTestTask, StartTaskAdcTest, osPriorityNormal, 0, 256);
  adcTestTaskHandle = osThreadCreate(osThread(adcTestTask), NULL);

  /* definition and creation of timTestTask */
  osThreadDef(timTestTask, StartTaskTimTest, osPriorityNormal, 0, 256);
  timTestTaskHandle = osThreadCreate(osThread(timTestTask), NULL);

  /* definition and creation of UDPResponderTas */
  osThreadDef(UDPResponderTas, StartResponseTask, osPriorityAboveNormal, 0, 512);
  UDPResponderTasHandle = osThreadCreate(osThread(UDPResponderTas), NULL);

  /* definition and creation of loggerTask */
  osThreadDef(loggerTask, StartLoggerTask, osPriorityLow, 0, 128);
  loggerTaskHandle = osThreadCreate(osThread(loggerTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
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

/* USER CODE BEGIN Header_StartTaskListen */
/**
* @brief Function implementing the ListenTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskListen */
void StartTaskListen(void const * argument)
{
  /* USER CODE BEGIN StartTaskListen */

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTaskListen */
}

/* USER CODE BEGIN Header_StartTaskHwVerif */
/**
* @brief Function implementing the hwVerifTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskHwVerif */
void StartTaskHwVerif(void const * argument)
{
  /* USER CODE BEGIN StartTaskHwVerif */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTaskHwVerif */
}

/* USER CODE BEGIN Header_StartTaskUartTest */
/**
* @brief Function implementing the uartTestTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskUartTest */
void StartTaskUartTest(void const * argument)
{
  /* USER CODE BEGIN StartTaskUartTest */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTaskUartTest */
}

/* USER CODE BEGIN Header_StartTaskI2cTest */
/**
* @brief Function implementing the i2cTestTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskI2cTest */
void StartTaskI2cTest(void const * argument)
{
  /* USER CODE BEGIN StartTaskI2cTest */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTaskI2cTest */
}

/* USER CODE BEGIN Header_StartTaskSpiTest */
/**
* @brief Function implementing the spiTestTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskSpiTest */
void StartTaskSpiTest(void const * argument)
{
  /* USER CODE BEGIN StartTaskSpiTest */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTaskSpiTest */
}

/* USER CODE BEGIN Header_StartTaskAdcTest */
/**
* @brief Function implementing the adcTestTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskAdcTest */
void StartTaskAdcTest(void const * argument)
{
  /* USER CODE BEGIN StartTaskAdcTest */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTaskAdcTest */
}

/* USER CODE BEGIN Header_StartTaskTimTest */
/**
* @brief Function implementing the timTestTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskTimTest */
void StartTaskTimTest(void const * argument)
{
  /* USER CODE BEGIN StartTaskTimTest */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTaskTimTest */
}

/* USER CODE BEGIN Header_StartResponseTask */
/**
* @brief Function implementing the responseTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartResponseTask */
void StartResponseTask(void const * argument)
{
  /* USER CODE BEGIN StartResponseTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartResponseTask */
}

/* USER CODE BEGIN Header_StartLoggerTask */
/**
* @brief Function implementing the loggerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLoggerTask */
void StartLoggerTask(void const * argument)
{
  /* USER CODE BEGIN StartLoggerTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartLoggerTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

