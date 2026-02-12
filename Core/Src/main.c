/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * 16.02.25 by Kurein M.N.
  *
  * v.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "stm32f1xx_hal.h"
#include "DataFile.h"
#include "uart.h"
#include "crc16.h"
#include "fram.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
volatile uint32_t last_rx_time;                   // Time of last received byte
extern volatile char uart1_rx_buf[UART_BUF_SIZE];
fram_t fram;
fram_cfg_t cfgFRAM;
QueueHandle_t uartQueue;

osThreadId_t LedTaskHandle, Uart1TaskHandle;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;

const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityNormal,
};

/* USER CODE BEGIN PV */
const osThreadAttr_t LedTask_attributes = {
  .name = "LedTask",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t Uart1Task_attributes = {
  .name = "Uart1Task",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_SPI1_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
void LedTask(void *argument);
void Uart1Task(void *argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LedTask(void *pvParameters) {
	//int c = 'A'; // 0x41

    while (1) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        //uart1_put_ch(c);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void Uart1Task(void *pvParameters) {
    uint8_t command[UART_BUF_SIZE];
    uint8_t ok[2] = {ID_BU, 0x00}; 
    uint8_t *ok_ptr;
    uint8_t index = 0;
    uint16_t crc = 0;
    char receivedChar;
    char wr_data[7] = {'K', 'U', 'R', 'E', 'I', 'N'};
    char rd_data[7] = {0};
    uart1_put_ch('S');  // Сигнал о запуске задачи
    while (1) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);

        // Waiting for a byte from the queue(очереди)
        if (xQueueReceive(uartQueue, &receivedChar, pdMS_TO_TICKS(2)) == pdPASS) {
            if (index < UART_BUF_SIZE) {
                command[index++] = receivedChar;
               // uart1_put_ch(receivedChar);
            }
        }

        uint32_t current_time = xTaskGetTickCount();
        uint32_t diff = current_time - last_rx_time;

        if (index > 0 && diff > pdMS_TO_TICKS(UART_TIMEOUT_MS)) {
          //  uart1_put_u32(diff);  //  для отладки таймаута
            crc = process_crc(command, index, true);
            // uart1_put_ch(crc);
            if (crc == 1) {
                switch (command[0]) {
                    case 0x70:
                        if (command[1] == 0) {
                            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
                        }
                        if (command[1] == 1) {
                            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
                        }
                        break;

                    case 2:
                        if (command[1] == 0) {
                            fram_write(&fram, 0x0150, (uint8_t *)wr_data, sizeof(wr_data));
                            HAL_Delay(1000);
                            fram_read(&fram, 0x0150, (uint8_t *)rd_data, sizeof(wr_data));
                        }
                        break;
                }                
            }
            ok_ptr = calculate_crc_for_2_bytes(ok);
            for (int i = 0; i < 4; i++) {
                uart1_put_ch(ok_ptr[i]);
            }

            //HAL_UART_Transmit(&huart1, ok_ptr, 4, 100); // 100 мс таймаут
            //uart1_put_ch(0xEE);  // Добавляем маркер конца посылки
            //uart1_put_ch(0xFF);

            while (uxQueueMessagesWaiting(uartQueue) > 0) {
                char discardChar;
                xQueueReceive(uartQueue, &discardChar, 0);
            }
            index = 0;
        } 
        vTaskDelay(pdMS_TO_TICKS(3));  // Adding a delay
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  last_rx_time = 0; 

  uint32_t reset_cause = RCC->CSR;
  RCC->CSR |= RCC_CSR_RMVF;          // Сброс флагов сброса
  //__HAL_RCC_CLEAR_RESET_FLAGS();   // Очистить флаги сброса

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_SPI1_Init();

  /* USER CODE BEGIN 2 */
  RCC->CSR |= RCC_CSR_RMVF;

  log_printf("Rst:0x%08lX\r\n", reset_cause);
  HAL_Delay(1000);
  log_printf("Main started, time: %lu ms\r\n", HAL_GetTick());

  fram_cfg_setup(&cfgFRAM);
  fram_init(&fram,&cfgFRAM);
  //log_printf("FRAM setup done\r\n");

  //fram_erase_all(&fram);
  //HAL_Delay(1000);
  //log_printf("---- Application Init ----\r\n");
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  uartQueue = xQueueCreate(40, sizeof(char));  // Очередь на 40 элементов
  if (uartQueue == NULL) {
       Error_Handler();// Ошибка создания очереди
  }
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  LedTaskHandle = osThreadNew(LedTask, NULL, &LedTask_attributes);

  Uart1TaskHandle = osThreadNew(Uart1Task, NULL, &Uart1Task_attributes);
  if (Uart1TaskHandle == NULL) {
	uart1_put_ch('E');
    Error_Handler();
  }
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
	printf("ERROR 1 DETECTED! MCU WILL RESET!\r\n");
	HAL_Delay(1000);
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
	printf("ERROR 2 DETECTED! MCU WILL RESET!\r\n");
	HAL_Delay(1000);
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    printf("ERROR SPI DETECTED! MCU WILL RESET!\r\n");
	HAL_Delay(1000);
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
	printf("ERROR USART1 DETECTED! MCU WILL RESET!\r\n");
	HAL_Delay(1000);
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  // запускает прерывание для приёма 1 байта
  HAL_UART_Receive_IT(&huart1, (uint8_t *)&uart1_rx_buf[0], 1);
  /* USER CODE END USART1_Init 2 */

}
static void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    printf("ERROR USART3 DETECTED! MCU WILL RESET!\r\n");
    HAL_Delay(1000);
    Error_Handler();
  }
}
/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	printf_uart3("Default Task running...\r\n");
  uint32_t diag_divider = 0;
  for (;;) {
	vTaskDelay(pdMS_TO_TICKS(1000));
	printf_uart3("Tick:%lu\r\n", HAL_GetTick());

	/* Lightweight runtime diagnostics every 5 seconds. */
	if (++diag_divider >= 5) {
		diag_divider = 0;

		size_t free_heap = xPortGetFreeHeapSize();
		size_t min_heap = xPortGetMinimumEverFreeHeapSize();
		UBaseType_t dflt_stack = uxTaskGetStackHighWaterMark((TaskHandle_t)defaultTaskHandle);
		UBaseType_t led_stack  = uxTaskGetStackHighWaterMark((TaskHandle_t)LedTaskHandle);
		UBaseType_t uart_stack = uxTaskGetStackHighWaterMark((TaskHandle_t)Uart1TaskHandle);
		size_t dflt_stack_b = ((size_t)dflt_stack) * sizeof(StackType_t);
		size_t led_stack_b  = ((size_t)led_stack) * sizeof(StackType_t);
		size_t uart_stack_b = ((size_t)uart_stack) * sizeof(StackType_t);

		printf_uart3("Heap:%u/%u\r\n", (unsigned int)free_heap, (unsigned int)min_heap);
		printf_uart3("StkB:%u,%u,%u\r\n", (unsigned int)dflt_stack_b, (unsigned int)led_stack_b, (unsigned int)uart_stack_b);
	}
  }
  /*for(;;)
  {
    osDelay(1000);
  }*/
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();

  printf("ERROR DETECTED! \r\n");
  HAL_Delay(1000);

  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
