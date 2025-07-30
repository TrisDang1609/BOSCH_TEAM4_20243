/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dcm.h"
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
CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
uint8_t uart3_receive;

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;
CAN_TxHeaderTypeDef CAN1_pHeader;
CAN_RxHeaderTypeDef CAN1_pHeaderRx;
CAN_FilterTypeDef CAN1_sFilterConfig;
CAN_TxHeaderTypeDef CAN2_pHeader;
CAN_RxHeaderTypeDef CAN2_pHeaderRx;
CAN_FilterTypeDef CAN2_sFilterConfig;
uint32_t CAN1_pTxMailbox;
uint32_t CAN2_pTxMailbox;

uint16_t NumBytesReq = 0;
uint8_t  REQ_BUFFER  [4096];
uint8_t  REQ_1BYTE_DATA;

uint8_t CAN1_DATA_TX[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t CAN1_DATA_RX[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t CAN2_DATA_TX[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t CAN2_DATA_RX[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

uint16_t Num_Consecutive_Tester;
uint8_t  Flg_Consecutive = 0;

unsigned int TimeStamp;
// maximum characters send out via UART is 30
char bufsend[30]="XXX: D1 D2 D3 D4 D5 D6 D7 D8  ";

// Test message definitions for practices
// Practice 1 - ReadDataByIdentifier (SID 0x22)
uint8_t TEST_RDBI_MSG[8] = {0x03, 0x22, 0x01, 0x23, 0x55, 0x55, 0x55, 0x55};

// Practice 2 - SecurityAccess (SID 0x27)
// Step 1: Request SEED
uint8_t TEST_SECA_SEED_REQ[8] = {0x02, 0x27, 0x01, 0x55, 0x55, 0x55, 0x55, 0x55};

// Step 2: Send KEY (calculated from SEED: A3,5C,91,2F,84,67)
// KEY[0]=0xFF, KEY[1]=0xED, KEY[2]=0xBE, KEY[3]=0xD2, KEY[4]=0x80, KEY[5]=0x07
uint8_t TEST_SECA_KEY_SEND[8] = {0x08, 0x27, 0x02, 0xFF, 0xED, 0xBE, 0xD2, 0x80};

// Practice 3 - WriteDataByIdentifier (SID 0x2E) - requires security unlock first
uint8_t TEST_WDBI_MSG[8] = {0x05, 0x2E, 0x01, 0x23, 0x04, 0x56, 0x55, 0x55};

// Predefined SEED values for testing (will be used in dcm_seca.c)
uint8_t TEST_SEED_VALUES[6] = {0xA3, 0x5C, 0x91, 0x2F, 0x84, 0x67};

// Manual test control variables
uint8_t MANUAL_TEST_MODE = 1;  // 1 = Manual, 0 = Auto
uint8_t AUTO_TEST_ENABLED = 0; // Controls automatic testing

// Hex input buffer for manual hex entry
uint8_t HEX_INPUT_BUFFER[24] = {0}; // Buffer for hex string input
uint8_t HEX_INPUT_INDEX = 0;
uint8_t HEX_MESSAGE_BUFFER[8] = {0}; // Parsed hex message
uint8_t HEX_INPUT_MODE = 0; // 0 = Command mode, 1 = Hex input mode

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_CAN2_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
void MX_CAN1_Setup();
void MX_CAN2_Setup();
void USART3_SendString(uint8_t *ch);
void PrintCANLog(uint16_t CANID, uint8_t * CAN_Frame);
void SID_22_Practice();
void SID_2E_Practice();
void SID_27_Practice();
void delay(uint16_t delay);
void TestPracticeMessages();
void SendTestMessage(uint8_t* message, uint16_t canid, char* description);
void ProcessManualCommand(uint8_t command);
void PrintManualTestMenu();
void ManualTest_Practice1();
void ManualTest_Practice2_Seed();
void ManualTest_Practice2_Key();
void ManualTest_Practice3();
void ManualTest_ToggleMode();
uint8_t ParseHexChar(char c);
void ProcessHexInput(uint8_t input_char);
void ExecuteHexMessage();
void PrintHexInputMode();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	// Removed unused variables: i, j, Consecutive_Cntr
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
  MX_CAN1_Init();
  MX_CAN2_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  MX_CAN1_Setup();
  MX_CAN2_Setup();
  __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
  
  // Start UART receive interrupt
  HAL_UART_Receive_IT(&huart3, &REQ_1BYTE_DATA, 1);
  
  // Initialize CAN headers for testing
  CAN1_pHeader.StdId = 0x123;  // Test CAN ID
  CAN1_pHeader.DLC = 8;
  CAN1_DATA_TX[0] = 0x01; CAN1_DATA_TX[1] = 0x02; CAN1_DATA_TX[2] = 0x03; CAN1_DATA_TX[3] = 0x04;
  CAN1_DATA_TX[4] = 0x05; CAN1_DATA_TX[5] = 0x06; CAN1_DATA_TX[6] = 0x07; CAN1_DATA_TX[7] = 0x08;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // Send startup message
  USART3_SendString((uint8_t *)"STM32 CAN Diagnostic Demo Started\n");
  USART3_SendString((uint8_t *)"=================================\n");
  
  // Print initial menu
  PrintManualTestMenu();
  
  // Example Function to print can message via uart
  PrintCANLog(CAN1_pHeader.StdId, &CAN1_DATA_TX[0]);
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    // Update timestamp
    TimeStamp = HAL_GetTick();
    
    // Manage DCM security timeout
    DCM_ManageSecurityTimeout();

    // Process manual commands received via UART
    if (REQ_1BYTE_DATA != 0) {
        ProcessManualCommand(REQ_1BYTE_DATA);
        REQ_1BYTE_DATA = 0; // Clear command
    }

    if(!BtnU) /*IG OFF->ON stimulation*/
    {
      delay(20);
      USART3_SendString((uint8_t *)"IG OFF ");
      while(!BtnU);
      
      // Apply new CAN ID if it was written via WriteDataByIdentifier
      extern uint16_t DCM_StoredCANID;
      if (DCM_StoredCANID != DCM_REQUEST_CAN_ID) {
          // Update CAN filter to new CAN ID
          CAN1_sFilterConfig.FilterIdHigh = DCM_StoredCANID << 5;
          USART3_SendString((uint8_t *)"New CAN ID Applied ");
      }
      
      MX_CAN1_Setup();
      MX_CAN2_Setup();
      USART3_SendString((uint8_t *)"-> IG ON \n");
      delay(20);
    }
    
    // Send periodic heartbeat message every 10 seconds (only in auto mode)
    static uint32_t lastHeartbeat = 0;
    if (AUTO_TEST_ENABLED && (HAL_GetTick() - lastHeartbeat > 10000)) {
        lastHeartbeat = HAL_GetTick();
        USART3_SendString((uint8_t *)"Heartbeat: Auto mode running\n");
        
        // Update test data and send
        CAN1_DATA_TX[0]++;
        if (CAN1_DATA_TX[0] > 0xFF) CAN1_DATA_TX[0] = 0;
        PrintCANLog(CAN1_pHeader.StdId, &CAN1_DATA_TX[0]);
    }
    
    // Auto test practice messages every 15 seconds (only if enabled)
    static uint32_t lastPracticeTest = 0;
    if (AUTO_TEST_ENABLED && (HAL_GetTick() - lastPracticeTest > 15000)) {
        lastPracticeTest = HAL_GetTick();
        USART3_SendString((uint8_t *)"\n>>> AUTO TEST SEQUENCE <<<\n");
        TestPracticeMessages();
    }
    
    delay(50); // Small delay to prevent CPU overload
  }

  memset(&REQ_BUFFER,0x00,4096);
  NumBytesReq = 0;

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 21;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_12TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief CAN2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 21;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_12TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

  /* USER CODE END CAN2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
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
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PC13 PC4 PC5 PC6
                           PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void MX_CAN1_Setup()
{
	// Configure CAN1 filter for diagnostic request (0x712)
	CAN1_sFilterConfig.FilterIdHigh = DCM_REQUEST_CAN_ID << 5;    // 0x712 << 5
	CAN1_sFilterConfig.FilterIdLow = 0x0000;
	CAN1_sFilterConfig.FilterMaskIdHigh = 0xFFFF;                 // Accept only exact ID
	CAN1_sFilterConfig.FilterMaskIdLow = 0x0000;
	CAN1_sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	CAN1_sFilterConfig.FilterBank = 0;
	CAN1_sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	CAN1_sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	CAN1_sFilterConfig.FilterActivation = ENABLE;
	CAN1_sFilterConfig.SlaveStartFilterBank = 14;
	
	HAL_CAN_ConfigFilter(&hcan1, &CAN1_sFilterConfig);
	HAL_CAN_Start(&hcan1);
	HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}
void MX_CAN2_Setup()
{
	// Configure CAN2 filter (for response transmission)
	CAN2_sFilterConfig.FilterIdHigh = 0x0000;
	CAN2_sFilterConfig.FilterIdLow = 0x0000;
	CAN2_sFilterConfig.FilterMaskIdHigh = 0x0000;
	CAN2_sFilterConfig.FilterMaskIdLow = 0x0000;
	CAN2_sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	CAN2_sFilterConfig.FilterBank = 14;
	CAN2_sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	CAN2_sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	CAN2_sFilterConfig.FilterActivation = ENABLE;
	
	HAL_CAN_ConfigFilter(&hcan2, &CAN2_sFilterConfig);
	HAL_CAN_Start(&hcan2);
	HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void USART3_SendString(uint8_t *ch)
{
   while(*ch!=0)
   {
      HAL_UART_Transmit(&huart3, ch, 1,HAL_MAX_DELAY);
      ch++;
   }
}
void PrintCANLog(uint16_t CANID, uint8_t * CAN_Frame)
{
	uint16_t loopIndx = 0;
	char bufID[6] = "     ";  // Increased size to handle up to 4 hex digits + null terminator
	char bufDat[4] = "   ";   // Increased size to handle 2 hex digits + null terminator
	char bufTime [8]="        ";

	sprintf(bufTime,"%d",TimeStamp);
	USART3_SendString((uint8_t*)bufTime);
	USART3_SendString((uint8_t*)" ");

	sprintf(bufID,"%X",CANID);
	for(loopIndx = 0; loopIndx < 3; loopIndx ++)
	{
		bufsend[loopIndx]  = bufID[loopIndx];
	}
	bufsend[3] = ':';
	bufsend[4] = ' ';


	for(loopIndx = 0; loopIndx < 8; loopIndx ++ )
	{
		sprintf(bufDat,"%02X",CAN_Frame[loopIndx]);
		bufsend[loopIndx*3 + 5] = bufDat[0];
		bufsend[loopIndx*3 + 6] = bufDat[1];
		bufsend[loopIndx*3 + 7] = ' ';
	}
	bufsend[29] = '\n';
	USART3_SendString((unsigned char*)bufsend);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	REQ_BUFFER[NumBytesReq] = REQ_1BYTE_DATA;
	NumBytesReq++;
	
	// Restart UART receive interrupt for next byte
	HAL_UART_Receive_IT(&huart3, &REQ_1BYTE_DATA, 1);
	
	//REQ_BUFFER[7] = NumBytesReq;
}
void delay(uint16_t delay)
{
	HAL_Delay(delay);
}

/**
 * @brief Test all practice messages with predefined values
 */
void TestPracticeMessages()
{
    USART3_SendString((uint8_t *)"\n=== Testing Practice Messages ===\n");
    
    // Practice 1: ReadDataByIdentifier
    SendTestMessage(TEST_RDBI_MSG, DCM_REQUEST_CAN_ID, "Practice 1: Read DID 0x0123");
    delay(1000);
    
    // Practice 2: SecurityAccess - Request SEED
    SendTestMessage(TEST_SECA_SEED_REQ, DCM_REQUEST_CAN_ID, "Practice 2: Request SEED Level 1");
    delay(1000);
    
    // Practice 2: SecurityAccess - Send KEY  
    SendTestMessage(TEST_SECA_KEY_SEND, DCM_REQUEST_CAN_ID, "Practice 2: Send KEY Level 2");
    delay(1000);
    
    // Practice 3: WriteDataByIdentifier (requires security unlock)
    SendTestMessage(TEST_WDBI_MSG, DCM_REQUEST_CAN_ID, "Practice 3: Write DID 0x0123 = 0x0456");
    delay(1000);
    
    USART3_SendString((uint8_t *)"=== Test Complete ===\n\n");
}

/**
 * @brief Send a test message via CAN and log to UART
 */
void SendTestMessage(uint8_t* message, uint16_t canid, char* description)
{
    USART3_SendString((uint8_t *)description);
    USART3_SendString((uint8_t *)"\n");
    
    // Print the message being sent
    PrintCANLog(canid, message);
    
    // Actually process the message locally for testing
    DCM_ProcessRequest(message, message[0] + 1); // Length is first byte + 1
    
    USART3_SendString((uint8_t *)"\n");
}

/**
 * @brief Print manual test menu to UART
 */
void PrintManualTestMenu()
{
    USART3_SendString((uint8_t *)"\n=========== MANUAL TEST MENU ===========\n");
    USART3_SendString((uint8_t *)"Mode: ");
    if (MANUAL_TEST_MODE) {
        USART3_SendString((uint8_t *)"MANUAL\n");
    } else {
        USART3_SendString((uint8_t *)"AUTO\n");
    }
    USART3_SendString((uint8_t *)"----------------------------------------\n");
    USART3_SendString((uint8_t *)"Commands:\n");
    USART3_SendString((uint8_t *)"  1 - Test Practice 1 (ReadDataByIdentifier)\n");
    USART3_SendString((uint8_t *)"  2 - Test Practice 2a (SecurityAccess SEED)\n");
    USART3_SendString((uint8_t *)"  3 - Test Practice 2b (SecurityAccess KEY)\n");
    USART3_SendString((uint8_t *)"  4 - Test Practice 3 (WriteDataByIdentifier)\n");
    USART3_SendString((uint8_t *)"  A - Run All Practices (Auto sequence)\n");
    USART3_SendString((uint8_t *)"  X - Enter HEX Input Mode (like Hercules)\n");
    USART3_SendString((uint8_t *)"  M - Toggle Manual/Auto Mode\n");
    USART3_SendString((uint8_t *)"  H - Show this Help menu\n");
    USART3_SendString((uint8_t *)"  S - Show Security Status\n");
    USART3_SendString((uint8_t *)"========================================\n");
    USART3_SendString((uint8_t *)"Enter command: ");
}

/**
 * @brief Process manual command received via UART
 */
void ProcessManualCommand(uint8_t command)
{
    // If in HEX input mode, process as hex input
    if (HEX_INPUT_MODE) {
        ProcessHexInput(command);
        return;
    }
    
    // Normal command processing
    switch (command) {
        case '1':
            USART3_SendString((uint8_t *)"\n>>> MANUAL TEST: Practice 1 <<<\n");
            ManualTest_Practice1();
            break;
        case '2':
            USART3_SendString((uint8_t *)"\n>>> MANUAL TEST: Practice 2a (SEED) <<<\n");
            ManualTest_Practice2_Seed();
            break;
        case '3':
            USART3_SendString((uint8_t *)"\n>>> MANUAL TEST: Practice 2b (KEY) <<<\n");
            ManualTest_Practice2_Key();
            break;
        case '4':
            USART3_SendString((uint8_t *)"\n>>> MANUAL TEST: Practice 3 <<<\n");
            ManualTest_Practice3();
            break;
        case 'A':
        case 'a':
            USART3_SendString((uint8_t *)"\n>>> MANUAL TEST: All Practices <<<\n");
            TestPracticeMessages();
            break;
        case 'X':
        case 'x':
            USART3_SendString((uint8_t *)"\n>>> ENTERING HEX INPUT MODE <<<\n");
            HEX_INPUT_MODE = 1;
            HEX_INPUT_INDEX = 0;
            memset(HEX_INPUT_BUFFER, 0, sizeof(HEX_INPUT_BUFFER));
            PrintHexInputMode();
            break;
        case 'M':
        case 'm':
            ManualTest_ToggleMode();
            break;
        case 'H':
        case 'h':
            PrintManualTestMenu();
            break;
        case 'S':
        case 's':
            USART3_SendString((uint8_t *)"\n>>> SECURITY STATUS <<<\n");
            if (DCM_IsSecurityLevel1Unlocked()) {
                USART3_SendString((uint8_t *)"Security Level 1: UNLOCKED (LED ON)\n");
            } else {
                USART3_SendString((uint8_t *)"Security Level 1: LOCKED (LED OFF)\n");
            }
            USART3_SendString((uint8_t *)"Enter command: ");
            break;
        case '\r':
        case '\n':
            // Ignore newline characters
            break;
        default:
            USART3_SendString((uint8_t *)"\nInvalid command! Press 'H' for help.\n");
            USART3_SendString((uint8_t *)"Enter command: ");
            break;
    }
}

/**
 * @brief Manual test for Practice 1 - ReadDataByIdentifier
 */
void ManualTest_Practice1()
{
    USART3_SendString((uint8_t *)"Testing ReadDataByIdentifier (SID 0x22, DID 0x0123)\n");
    USART3_SendString((uint8_t *)"Expected: Returns current CAN ID from tester\n");
    USART3_SendString((uint8_t *)"--------------------------------------------\n");
    
    SendTestMessage(TEST_RDBI_MSG, DCM_REQUEST_CAN_ID, "Sending RDBI request");
    
    USART3_SendString((uint8_t *)"✅ Practice 1 complete.\n");
    USART3_SendString((uint8_t *)"Enter command: ");
}

/**
 * @brief Manual test for Practice 2a - SecurityAccess SEED Request
 */
void ManualTest_Practice2_Seed()
{
    USART3_SendString((uint8_t *)"Testing SecurityAccess SEED Request (SID 0x27, Level 0x01)\n");
    USART3_SendString((uint8_t *)"Expected: Returns 6-byte random SEED\n");
    USART3_SendString((uint8_t *)"------------------------------------------------------\n");
    
    SendTestMessage(TEST_SECA_SEED_REQ, DCM_REQUEST_CAN_ID, "Sending SECA SEED request");
    
    USART3_SendString((uint8_t *)"ℹ️ Note: Use received SEED to calculate KEY for Practice 2b\n");
    USART3_SendString((uint8_t *)"✅ Practice 2a complete.\n");
    USART3_SendString((uint8_t *)"Enter command: ");
}

/**
 * @brief Manual test for Practice 2b - SecurityAccess KEY Send
 */
void ManualTest_Practice2_Key()
{
    USART3_SendString((uint8_t *)"Testing SecurityAccess KEY Send (SID 0x27, Level 0x02)\n");
    USART3_SendString((uint8_t *)"Expected: LED turns ON for 5 seconds if KEY is correct\n");
    USART3_SendString((uint8_t *)"--------------------------------------------------------\n");
    
    // Show SEED-KEY calculation for reference
    USART3_SendString((uint8_t *)"SEED-KEY Algorithm:\n");
    USART3_SendString((uint8_t *)"KEY[0] = SEED[0] XOR SEED[1]\n");
    USART3_SendString((uint8_t *)"KEY[1] = SEED[1] + SEED[2]\n");
    USART3_SendString((uint8_t *)"KEY[2] = SEED[2] XOR SEED[3]\n");
    USART3_SendString((uint8_t *)"KEY[3] = SEED[3] + SEED[0]\n");
    USART3_SendString((uint8_t *)"KEY[4] = SEED[4] AND 0xF0\n");
    USART3_SendString((uint8_t *)"KEY[5] = SEED[5] AND 0x0F\n");
    USART3_SendString((uint8_t *)"------------------------\n");
    
    SendTestMessage(TEST_SECA_KEY_SEND, DCM_REQUEST_CAN_ID, "Sending SECA KEY");
    
    USART3_SendString((uint8_t *)"🔍 Check LED status: Should be ON if KEY is correct\n");
    USART3_SendString((uint8_t *)"✅ Practice 2b complete.\n");
    USART3_SendString((uint8_t *)"Enter command: ");
}

/**
 * @brief Manual test for Practice 3 - WriteDataByIdentifier
 */
void ManualTest_Practice3()
{
    USART3_SendString((uint8_t *)"Testing WriteDataByIdentifier (SID 0x2E, DID 0x0123)\n");
    USART3_SendString((uint8_t *)"Expected: Writes new CAN ID (0x0456) - requires security unlock\n");
    USART3_SendString((uint8_t *)"------------------------------------------------------------------\n");
    
    // Check security status first
    if (!DCM_IsSecurityLevel1Unlocked()) {
        USART3_SendString((uint8_t *)"⚠️ WARNING: Security not unlocked! Run Practice 2 first.\n");
        USART3_SendString((uint8_t *)"Expected Response: NRC 0x33 (Security Access Denied)\n");
    } else {
        USART3_SendString((uint8_t *)"✅ Security unlocked. Should write successfully.\n");
    }
    
    SendTestMessage(TEST_WDBI_MSG, DCM_REQUEST_CAN_ID, "Sending WDBI request");
    
    USART3_SendString((uint8_t *)"ℹ️ Note: New CAN ID applied after User Button press (Ignition Cycle)\n");
    USART3_SendString((uint8_t *)"✅ Practice 3 complete.\n");
    USART3_SendString((uint8_t *)"Enter command: ");
}

/**
 * @brief Toggle between Manual and Auto test modes
 */
void ManualTest_ToggleMode()
{
    MANUAL_TEST_MODE = !MANUAL_TEST_MODE;
    AUTO_TEST_ENABLED = !AUTO_TEST_ENABLED;
    
    USART3_SendString((uint8_t *)"\n>>> MODE CHANGED <<<\n");
    if (MANUAL_TEST_MODE) {
        USART3_SendString((uint8_t *)"Switched to: MANUAL MODE\n");
        USART3_SendString((uint8_t *)"- Auto testing disabled\n");
        USART3_SendString((uint8_t *)"- Use commands to test individually\n");
    } else {
        USART3_SendString((uint8_t *)"Switched to: AUTO MODE\n");
        USART3_SendString((uint8_t *)"- Auto testing every 15 seconds\n");
        USART3_SendString((uint8_t *)"- Manual commands still available\n");
    }
    PrintManualTestMenu();
}

/**
 * @brief Parse single hex character to value
 */
uint8_t ParseHexChar(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return 0xFF; // Invalid character
}

/**
 * @brief Print hex input mode instructions
 */
void PrintHexInputMode()
{
    USART3_SendString((uint8_t *)"========== HEX INPUT MODE ==========\n");
    USART3_SendString((uint8_t *)"Enter CAN message in hex format:\n");
    USART3_SendString((uint8_t *)"Format: XX XX XX XX XX XX XX XX\n");
    USART3_SendString((uint8_t *)"Examples:\n");
    USART3_SendString((uint8_t *)"  03 22 01 23 55 55 55 55  (Practice 1)\n");
    USART3_SendString((uint8_t *)"  02 27 01 55 55 55 55 55  (Practice 2a)\n");
    USART3_SendString((uint8_t *)"  08 27 02 XX XX XX XX XX  (Practice 2b)\n");
    USART3_SendString((uint8_t *)"  05 2E 01 23 04 56 55 55  (Practice 3)\n");
    USART3_SendString((uint8_t *)"Commands:\n");
    USART3_SendString((uint8_t *)"  ENTER - Execute message\n");
    USART3_SendString((uint8_t *)"  ESC   - Exit hex mode\n");
    USART3_SendString((uint8_t *)"  SPACE - Separator (optional)\n");
    USART3_SendString((uint8_t *)"===================================\n");
    USART3_SendString((uint8_t *)"HEX> ");
}

/**
 * @brief Process hex input character by character
 */
void ProcessHexInput(uint8_t input_char)
{
    // Handle special commands
    if (input_char == 27) { // ESC key
        USART3_SendString((uint8_t *)"\n>>> EXITING HEX INPUT MODE <<<\n");
        HEX_INPUT_MODE = 0;
        HEX_INPUT_INDEX = 0;
        memset(HEX_INPUT_BUFFER, 0, sizeof(HEX_INPUT_BUFFER));
        PrintManualTestMenu();
        return;
    }
    
    if (input_char == '\r' || input_char == '\n') { // ENTER key
        USART3_SendString((uint8_t *)"\n");
        ExecuteHexMessage();
        return;
    }
    
    if (input_char == ' ') { // Space separator - ignore
        USART3_SendString((uint8_t *)" ");
        return;
    }
    
    if (input_char == 8 || input_char == 127) { // Backspace
        if (HEX_INPUT_INDEX > 0) {
            HEX_INPUT_INDEX--;
            HEX_INPUT_BUFFER[HEX_INPUT_INDEX] = 0;
            USART3_SendString((uint8_t *)"\b \b"); // Backspace, space, backspace
        }
        return;
    }
    
    // Check if it's a valid hex character
    uint8_t hex_val = ParseHexChar(input_char);
    if (hex_val == 0xFF) {
        USART3_SendString((uint8_t *)"\nInvalid hex character! Use 0-9, A-F\n");
        USART3_SendString((uint8_t *)"HEX> ");
        return;
    }
    
    // Add to buffer if not full
    if (HEX_INPUT_INDEX < sizeof(HEX_INPUT_BUFFER) - 1) {
        HEX_INPUT_BUFFER[HEX_INPUT_INDEX] = input_char;
        HEX_INPUT_INDEX++;
        
        // Echo character
        uint8_t echo[2] = {input_char, 0};
        USART3_SendString(echo);
        
        // Add space after every 2 characters for readability
        if (HEX_INPUT_INDEX % 2 == 0 && HEX_INPUT_INDEX < sizeof(HEX_INPUT_BUFFER) - 1) {
            USART3_SendString((uint8_t *)" ");
        }
    } else {
        USART3_SendString((uint8_t *)"\nBuffer full! Press ENTER to execute or ESC to cancel.\n");
        USART3_SendString((uint8_t *)"HEX> ");
    }
}

/**
 * @brief Execute the hex message entered by user
 */
void ExecuteHexMessage()
{
    // Parse hex string to bytes
    uint8_t byte_count = 0;
    memset(HEX_MESSAGE_BUFFER, 0x55, sizeof(HEX_MESSAGE_BUFFER)); // Default padding
    
    for (int i = 0; i < HEX_INPUT_INDEX && byte_count < 8; i += 2) {
        if (i + 1 < HEX_INPUT_INDEX) {
            uint8_t high_nibble = ParseHexChar(HEX_INPUT_BUFFER[i]);
            uint8_t low_nibble = ParseHexChar(HEX_INPUT_BUFFER[i + 1]);
            
            if (high_nibble != 0xFF && low_nibble != 0xFF) {
                HEX_MESSAGE_BUFFER[byte_count] = (high_nibble << 4) | low_nibble;
                byte_count++;
            }
        }
    }
    
    if (byte_count == 0) {
        USART3_SendString((uint8_t *)"Error: No valid bytes parsed!\n");
        USART3_SendString((uint8_t *)"HEX> ");
        return;
    }
    
    // Display what will be executed
    USART3_SendString((uint8_t *)"Executing: ");
    for (int i = 0; i < 8; i++) {
        char hex_str[4];
        sprintf(hex_str, "%02X ", HEX_MESSAGE_BUFFER[i]);
        USART3_SendString((uint8_t *)hex_str);
    }
    USART3_SendString((uint8_t *)"\n");
    
    // Execute the message
    USART3_SendString((uint8_t *)"Processing diagnostic request...\n");
    PrintCANLog(DCM_REQUEST_CAN_ID, HEX_MESSAGE_BUFFER);
    
    // Determine message length from first byte (CAN-TP PCI)
    uint8_t message_length = 8;
    if ((HEX_MESSAGE_BUFFER[0] & 0xF0) == 0x00) { // Single Frame
        message_length = (HEX_MESSAGE_BUFFER[0] & 0x0F) + 1; // SF_DL + PCI byte
    }
    
    DCM_ProcessRequest(HEX_MESSAGE_BUFFER, message_length);
    
    // Reset for next input
    HEX_INPUT_INDEX = 0;
    memset(HEX_INPUT_BUFFER, 0, sizeof(HEX_INPUT_BUFFER));
    
    USART3_SendString((uint8_t *)"\nHEX> ");
}
/* USER CODE END 4 */

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
