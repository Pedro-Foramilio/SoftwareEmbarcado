/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//             --- veja includes, macro, e constantes no main.h ---
#include <funcoes_display.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// ---- ver vários #defines (kts, delays, etc) no main.h ----
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//  --- ver macros definidas no main.h ---
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

UART_HandleTypeDef huart1;

osThreadId defaultTaskHandle;
osThreadId Task_MngLEDHandle;
osThreadId TaskDisplayHandle;
osThreadId Task_VarrerHandle;
osThreadId Task_MngComnsHandle;
osMessageQId Q_ReqsHandle;
/* USER CODE BEGIN PV */
// variáveis que todos vamos usar: buffers de entrada/saída na comunicação
// buffers para entrada e saida de dados via USART
uint8_t BufOUT[] = {'0','0','0','0','0'};  // inicia buffer OUT com cars "0"
uint8_t BufIN[]  = {'0','0','0','0','0'};  // inicia buffer IN com cars "0"
int8_t DspHex[]  = {8,8,8,8};      // vetor val display (se=16 display off)
size_t sizeBuffs = sizeof(BufOUT);     // tamanho dos buffers - usa geral

// os vetores abaixo tem idx[0] = digito menos significativo no display
int8_t Crono[] = {0,0,0,0};            // vetor com vals decimais do cronometro
int8_t ValAdc[] = {0,0,0,0};           // vetor com vals decimais do ADC
int8_t CronoExt[] = {0,0,0,0};
int8_t ValAdcExt[] = {0,0,0,0};

uint8_t ptoDec = 0;  // qual dig liga pto? (ex: 0xA=>1010=> 1000=MSD + 0010=DG2)
uint32_t tinCrono = 0;                 // último tempo de entrada no cronômetro

volatile uint8_t modoLed = STARTUP;
volatile uint8_t modoDisplay = DISPLAY_INTRN;
uint8_t recebendoDados = 0;

int8_t RA[] = {0, 3, 3, 9};

uint32_t tADC = 0;

uint32_t startTime;
uint8_t startupComplete = 0;

int A1_foi_apertado = 0;

BaseType_t xHigherPriorityTaskWoken = pdFALSE;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
void StartDefaultTask(void const * argument);
void fnTask_MngLED(void const * argument);
void fn_TaskDisplay(void const * argument);
void fn_Task_Varrer(void const * argument);
void fn_Task_MngComns(void const * argument);

static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
void put_Q_ISR(uint16_t code);
void put_Q(uint16_t code);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void put_Q_ISR(uint16_t code)
{
	xQueueSendFromISR(Q_ReqsHandle, &code,  &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}
void put_Q(uint16_t code)
{
	xQueueSend(Q_ReqsHandle, &code, ( TickType_t ) 0 );
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_ADC1_Init();
  MX_USART1_UART_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

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
  /* definition and creation of Q_Reqs */
  osMessageQDef(Q_Reqs, 16, uint16_t);
  Q_ReqsHandle = osMessageCreate(osMessageQ(Q_Reqs), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 64);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of Task_MngLED */
  osThreadDef(Task_MngLED, fnTask_MngLED, osPriorityIdle, 0, 64);
  Task_MngLEDHandle = osThreadCreate(osThread(Task_MngLED), NULL);

  /* definition and creation of TaskDisplay */
  osThreadDef(TaskDisplay, fn_TaskDisplay, osPriorityIdle, 0, 64);
  TaskDisplayHandle = osThreadCreate(osThread(TaskDisplay), NULL);

  /* definition and creation of Task_Varrer */
  osThreadDef(Task_Varrer, fn_Task_Varrer, osPriorityIdle, 0, 64);
  Task_VarrerHandle = osThreadCreate(osThread(Task_Varrer), NULL);

  /* definition and creation of Task_MngComns */
  osThreadDef(Task_MngComns, fn_Task_MngComns, osPriorityIdle, 0, 64);
  Task_MngComnsHandle = osThreadCreate(osThread(Task_MngComns), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
        // ATENÇÃO: como aqui nunca vai chegar mesmo!
        // exemplo de comandO para ENVIAR o buffer BufOUT pela UART1
        HAL_UART_Transmit_IT(&huart1, BufOUT, sizeBuffs);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* EXTI3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  /* EXTI2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
  /* EXTI1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  /* ADC1_2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(ADC1_2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_6|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_SET);

  /*Configure GPIO pins : PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB12 PB13 PB14
                           PB15 PB6 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_6|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

// fn que atende ao callback da ISR do conversor ADC1
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  uint16_t val_adc = 0;                // define var para ler ADC
  if(hadc->Instance == ADC1) {         // se veio ADC1
    val_adc = HAL_ADC_GetValue(&hadc1);// capta valor adc
    // converter o valor lido em valores hexa p/ display
    int miliVolt = val_adc*3300/4095;
    int uniADC = miliVolt/1000;
    int decADC = (miliVolt-(uniADC*1000))/100;
    int cnsADC = (miliVolt-(uniADC*1000)-(decADC*100))/10;
    int mlsADC = miliVolt-(uniADC*1000)-(decADC*100)-(cnsADC*10);
    ValAdc[3] = uniADC;         // dig mais significativo
    ValAdc[2] = decADC;
    ValAdc[1] = cnsADC;
    ValAdc[0] = mlsADC;
  }
}


// fn que atende ao callback da ISR quando RECEBE dado pela UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  // o que veio na UART? aqui vc vai fazer testes p/ identificar mensagens

	__disable_irq();                   // desabilita IRQs
  // exemplo: se veio um valor iniciado com 'aXXXX", veio o valor do ADC
	if (BufIN[0] == 's')
	{
		HAL_GPIO_WritePin(GPIOB, LED4 | LED3 | LED2 | LED1, GPIO_PIN_RESET);
		modoDisplay = DISPLAY_EXTRN;
		recebendoDados = 1;
		if (A1_foi_apertado == 0)
		{
			modoLed = WFI;
		}
		else
		{
			//setup modo 2s 4 valores
			modoLed = LED_CRON_EXT;
		}
	}

	if (BufIN[0] == 'n')
	{
		modoDisplay = DISPLAY_INTRN;
		HAL_GPIO_WritePin(GPIOB, LED4 | LED3 | LED2 | LED1, GPIO_PIN_RESET);
		recebendoDados = 0;
		if (A1_foi_apertado == 0)
		{
			modoLed = WFI;
		}
		else
		{
			modoLed = LED_CRON;
		}

	}

	if (BufIN[0]=='A')
	{

		ValAdcExt[0] = conv_ASC_num(BufIN[4]);
		ValAdcExt[1] = conv_ASC_num(BufIN[3]);
		ValAdcExt[2] = conv_ASC_num(BufIN[2]);
		ValAdcExt[3] = conv_ASC_num(BufIN[1]);


	}

	if (BufIN[0]=='T')
	{
		CronoExt[0] = conv_ASC_num(BufIN[4]);
		CronoExt[1] = conv_ASC_num(BufIN[3]);
		CronoExt[2] = conv_ASC_num(BufIN[2]);
		CronoExt[3] = conv_ASC_num(BufIN[1]);

	}

	if (BufIN[0] == 't')
	{
		put_Q_ISR((uint16_t) Q_SND_CRN);
		//BufOUT[0] = 'T';
		//BufOUT[1] = conv_num_ASC(Crono[0]);
		//BufOUT[2] = conv_num_ASC(Crono[1]);
		//BufOUT[3] = conv_num_ASC(Crono[2]);
		//BufOUT[4] = conv_num_ASC(Crono[3]);
		//HAL_UART_Transmit_IT(&huart1, BufOUT, sizeBuffs);
	}

	if (BufIN[0] == 'a')
	{
		put_Q_ISR((uint16_t) Q_SND_ADC);
		//BufOUT[0] = 'A';
		//BufOUT[1] = conv_num_ASC(ValAdc[0]);
		//BufOUT[2] = conv_num_ASC(ValAdc[1]);
		//BufOUT[3] = conv_num_ASC(ValAdc[2]);
		//BufOUT[4] = conv_num_ASC(ValAdc[3]);
		//HAL_UART_Transmit_IT(&huart1, BufOUT, sizeBuffs);
	}



  	  __enable_irq();                      // volta habilitar IRQs

  // como precisa escutar a UART continuamente, reativar RECEPÇÃO via irq
  //TODO: disprar pela primeira vez em algum lugar
  	  HAL_UART_Receive_IT(&huart1, BufIN, sizeBuffs);
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
	startTime = HAL_GetTick();
	HAL_GPIO_WritePin(GPIOB, LED1 | LED2 | LED3 | LED4 , GPIO_PIN_RESET);

	HAL_UART_Receive_IT(&huart1, BufIN, sizeBuffs);
	/* Infinite loop */
  for(;;)
  {
	  // essa é a task default - vai colocar algo nela?
	  osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_fnTask_MngLED */
/**
* @brief Function implementing the Task_MngLED thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_fnTask_MngLED */
void fnTask_MngLED(void const * argument)
{
  /* USER CODE BEGIN fnTask_MngLED */
  /* Infinite loop */
 //HAL_GPIO_WritePin(GPIOB, LED4 | LED3 | LED2 | LED1, GPIO_PIN_SET);

  for(;;)
  {
    switch (modoLed)
    {
      case STARTUP:
    	HAL_GPIO_WritePin(GPIOB, LED4 | LED3 | LED2 | LED1, GPIO_PIN_RESET);
    	break;
      case WFI:
    	HAL_GPIO_WritePin(GPIOB, LED4 | LED3 | LED2 | LED1, GPIO_PIN_SET);
    	break;
      case LED_CRON:
        HAL_GPIO_TogglePin(GPIOB, LED1);
        HAL_GPIO_WritePin(GPIOB, LED4 | LED3 | LED2, GPIO_PIN_SET);
        break;
      case LED_ADC:
        HAL_GPIO_TogglePin(GPIOB, LED2);
        HAL_GPIO_WritePin(GPIOB, LED4 | LED3 | LED1, GPIO_PIN_SET);
        break;
      case LED_CRON_EXT:
        HAL_GPIO_TogglePin(GPIOB, LED3);
        HAL_GPIO_WritePin(GPIOB, LED4 | LED2 | LED1, GPIO_PIN_SET);
        break;
      case LED_ADC_EXT:
        HAL_GPIO_TogglePin(GPIOB, LED4);
        HAL_GPIO_WritePin(GPIOB, LED3 | LED2 | LED1, GPIO_PIN_SET);
        break;
    }

    osDelay(DT_LEDS);
  }
  /* USER CODE END fnTask_MngLED */
}

/* USER CODE BEGIN Header_fn_TaskDisplay */
/**
* @brief Function implementing the TaskDisplay thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_fn_TaskDisplay */
void fn_TaskDisplay(void const * argument)
{
  /* USER CODE BEGIN fn_TaskDisplay */
  /* Infinite loop */
  for(;;)
  {

	  switch (modoDisplay)
	  {
	  	  case WFI:
	  		  break;

	  	  case DISPLAY_INTRN:
	  		//trocar para ADC
	  		  if (modoLed == LED_CRON)
	  		  {
	  			  modoLed = LED_ADC;

	  		  }

	  		  //trocar para CRON
	  		  else if (modoLed == LED_ADC)
	  		  {
	  			  modoLed = LED_CRON;
	  		  }

	  		  break;

	  	  case DISPLAY_EXTRN:
	  		if (modoLed == WFI)
	  		{
	  			modoLed = LED_CRON_EXT;
	  		}
	  		else if (modoLed == LED_CRON)
	  		{
	  			modoLed = LED_ADC;
	  		}
	  		else if (modoLed == LED_ADC)
	  		{
	  			modoLed = LED_CRON_EXT;
	  		}
	  		else if (modoLed == LED_CRON_EXT)
	  		{
	  			modoLed = LED_ADC_EXT;
	  		}
	  		else if (modoLed == LED_ADC_EXT)
	  		{
	  			if (A1_foi_apertado == 0)
	  			{
	  				modoLed = WFI;
	  			}
	  			else
	  			{
	  				modoLed = LED_CRON;
	  			}
	  		}
	  		break; //break do case DISPLAY EXTRN
	  }


	  if (modoDisplay == DISPLAY_INTRN)
	  {
		  osDelay(DT_DISPLAY_MD1);
	  }
	  else
	  {
		  osDelay(DT_DISPLAY_MD2);
	  }
  }
  /* USER CODE END fn_TaskDisplay */
}

/* USER CODE BEGIN Header_fn_Task_Varrer */
/**
* @brief Function implementing the Task_Varrer thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_fn_Task_Varrer */
void fn_Task_Varrer(void const * argument)
{
  /* USER CODE BEGIN fn_Task_Varrer */
  /* Infinite loop */
  for(;;)
  {
	  if (HAL_GetTick() - startTime > 2000  && startupComplete != 1)
	  {
		  modoLed = WFI;
		  startupComplete = 1;
	  }
	  switch (modoLed)
	  		  {
	  	  	  	  case STARTUP:
	  	  	  		  DspHex[0] = 8;
	  	  	  		  DspHex[1] = 8;
	  	  	  		  DspHex[2] = 8;
	  	  	  		  DspHex[3] = 8;
	  	  	  		  ptoDec = 15;
	  	  	  		  break;
	  		  	  case WFI:
	  		  		  DspHex[3] = RA[0];
	  		  		  DspHex[2] = RA[1];
	  		  		  DspHex[1] = RA[2];
	  		  		  DspHex[0] = RA[3];
	  		  		  break;
	  		  	  case LED_CRON:
	  		  		  DspHex[0] = Crono[0];
	  		  		  DspHex[1] = Crono[1];
	  		  		  DspHex[2] = Crono[2];
	  		  		  DspHex[3] = Crono[3];
	  		  		  ptoDec = 10;
	  		  		  break;
	  		  	  case LED_ADC:
	  		  		  DspHex[0] = ValAdc[0];
	  		  		  DspHex[1] = ValAdc[1];
	  		  		  DspHex[2] = ValAdc[2];
	  		  		  DspHex[3] = ValAdc[3];
	  		  		  ptoDec = 8;
	  		  		  break;
	  		  	  case LED_CRON_EXT:

	  		  		  //STR_BUFF(REQCRN);
	  		  		  //HAL_UART_Transmit_IT(&huart1, BufOUT, sizeBuffs);
	  		  		  put_Q((uint16_t) Q_REQ_CRN);

	  		  		  DspHex[0] = CronoExt[0];
	  		  		  DspHex[1] = CronoExt[1];
	  		  		  DspHex[2] = CronoExt[2];
	  		  		  DspHex[3] = CronoExt[3];
	  		  		  ptoDec = 10;
	  		  		  break;
	  		  	  case LED_ADC_EXT:
	  		  		  //STR_BUFF(REQADC);
	  		  		  //HAL_UART_Transmit_IT(&huart1, BufOUT, sizeBuffs);
	  		  		  put_Q((uint16_t) Q_REQ_ADC);

	  		  		  DspHex[0] = ValAdcExt[0];
	  		  		  DspHex[1] = ValAdcExt[1];
	  		  		  DspHex[2] = ValAdcExt[2];
	  		  		  DspHex[3] = ValAdcExt[3];
	  		  		  ptoDec = 8;
	  		  		  break;
	  		  }
	  mostrar_no_display(DspHex, ptoDec);
	  osDelay(DT_VARRE_DISPLAY);
  }
  /* USER CODE END fn_Task_Varrer */
}

/* USER CODE BEGIN Header_fn_Task_MngComns */
/**
* @brief Function implementing the Task_MngComns thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_fn_Task_MngComns */
void fn_Task_MngComns(void const * argument)
{
  /* USER CODE BEGIN fn_Task_MngComns */
  /* Infinite loop */
	uint16_t msg = 0;
	BaseType_t statusReturn = pdFALSE;
  for(;;)
  {
	  statusReturn = pdFALSE;
	  if (HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX)
	  {
		  //busy -> nao enviar nada agora
	  }

	  else if (uxQueueMessagesWaiting(Q_ReqsHandle) > 0)
	  {
		  statusReturn = xQueueReceive(Q_ReqsHandle, &msg, ( TickType_t ) 10);

		  if (statusReturn == pdTRUE)
		  {
			  switch (msg)
			  {
				  case Q_SND_CRN:
					  BufOUT[0] = 'T';
					  BufOUT[1] = conv_num_ASC(Crono[3]);
					  BufOUT[2] = conv_num_ASC(Crono[2]);
					  BufOUT[3] = conv_num_ASC(Crono[1]);
					  BufOUT[4] = conv_num_ASC(Crono[0]);
					  break;
				  case Q_SND_ADC:
					  BufOUT[0] = 'A';
					  BufOUT[1] = conv_num_ASC(ValAdc[3]);
					  BufOUT[2] = conv_num_ASC(ValAdc[2]);
					  BufOUT[3] = conv_num_ASC(ValAdc[1]);
					  BufOUT[4] = conv_num_ASC(ValAdc[0]);
					  break;
				  case Q_REQ_CRN:
					  STR_BUFF(REQCRN);
					  break;
				  case Q_REQ_ADC:
					  STR_BUFF(REQADC);
					  break;
				  case Q_REQ_SRV:
					  STR_BUFF(REQSRV);
					  break;
				  case Q_REQ_OFF:
					  STR_BUFF(REQOFF);
					  break;
			  }
			  HAL_UART_Transmit_IT(&huart1, BufOUT, sizeBuffs);
		  }

	  }
	  else
	  {
		  //tx livre -> fila vazia -> sem reqs
	  }
	  osDelay(1);
  }
  /* USER CODE END fn_Task_MngComns */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  //TODO:chamar att ADC no DT certo
  if (HAL_GetTick()- tADC > DT_ADC)
  {
	  HAL_ADC_Start_IT(&hadc1);
	  tADC = HAL_GetTick();
  }

  // CRONOMETRO UP/DOWN: teste se já passou DT_CRONO ms
    if ((HAL_GetTick() - tinCrono) > DT_CRONO)
    {
      tinCrono = HAL_GetTick();          // atualiza o tempo de entrada aqui
      if(MD_CRONO == 0){                 // MD_CRONO = 0 incrementa o cronômetro
        ++ Crono[0];                     // inc decimo de segundos
        if (Crono[0] > 9){               // se > 9
          Crono[0] = 0;                  // volta p/ zero
          ++ Crono[1];                   // inc unidade de segundo
          if (Crono[1] > 9){             // se > 9
            Crono[1] = 0;                // volta p/ zero
            ++ Crono[2];                 // inc dezena de segundos
            if (Crono[2] > 5){           // se > 5
              Crono[2] = 0;              // volta p/ zero
              ++ Crono[3];               // inc minutos
              if (Crono[3] > 9){         // se > 9
                Crono[3] = 0;            // volta p/ zero
        }  }  }  }
      } else {                           // MD_CRONO = 1 decrementa o cronômetro
        -- Crono[0];                     // dec decimo de segundos
        if (Crono[0] < 0){               // se < 0
          Crono[0] = 9;                  // volta p/ 9
          -- Crono[1];                   // dec unidade de segundo
          if (Crono[1] < 0){             // se < 0
            Crono[1] = 9;                // volta p/ 9
            -- Crono[2];                 // dec dezena de segundos
            if (Crono[2] < 0){           // se < 0
              Crono[2] = 5;              // volta p/ 5
              -- Crono[3];               // dec minutos
              if (Crono[3] < 0 ){        // se < 0
                Crono[3] = 9;            // volta p/ 9
        }  }  }  }
      }
    }

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
