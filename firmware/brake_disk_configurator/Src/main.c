/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include <stdio.h>
#include "mlx90614.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
enum CLICommandType {
	CLI_UNKNOWN 		= 0,
	CLI_HELP			= 'H',
	CLI_SCAN 			= 'S',
	CLI_READ_OBJECT 	= 'O',
	CLI_READ_AMBIENT 	= 'A',
	CLI_SET_EMISSIVITY 	= 'E',
	CLI_CHANGE_ADDRESS 	= 'C',
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */
static const uint8_t MIN_I2C_ADDR = 0x01;
static const uint8_t MAX_I2C_ADDR = 0x7F;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_I2C1_Init(void);
static void MX_UART4_Init(void);
/* USER CODE BEGIN PFP */
int handle_calibration(uint8_t* buffer_in, uint32_t length_in, char output_message[], const size_t OUT_BUF_LEN);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static HAL_StatusTypeDef led_set_duty(uint8_t led, uint16_t duty) {
	TIM_OC_InitTypeDef sConfigOC = {0};
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = duty;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

	switch (led) {
	case 1:
		return HAL_TIM_PWM_ConfigChannel(&LED_TIMER, &sConfigOC, TIM_CHANNEL_1);
	case 2:
		return HAL_TIM_PWM_ConfigChannel(&LED_TIMER, &sConfigOC, TIM_CHANNEL_2);
	case 3:
		return HAL_TIM_PWM_ConfigChannel(&LED_TIMER, &sConfigOC, TIM_CHANNEL_3);
	default:
		return HAL_ERROR; // Unrecognized LED number
	}

	return HAL_ERROR; // This shouldn't ever be reached
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
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */

	const uint16_t IN_BUFFER_SIZE = 1024;
	const uint16_t OUT_BUFFER_SIZE = 1024;
	uint8_t in_buffer[IN_BUFFER_SIZE];
	memset(in_buffer, 0, IN_BUFFER_SIZE);
	uint16_t out_length = 0;
	uint8_t out_buffer[OUT_BUFFER_SIZE];
	memset(out_buffer, 0, OUT_BUFFER_SIZE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		led_set_duty(3, UINT16_MAX);
		memset(in_buffer, 0, IN_BUFFER_SIZE);
		HAL_UART_Receive(&DEBUG_UART, in_buffer, IN_BUFFER_SIZE, 1000);
		led_set_duty(3, 0);

		out_length = handle_calibration(in_buffer, IN_BUFFER_SIZE, (char*)out_buffer, OUT_BUFFER_SIZE);
		if (out_length > 0) {
			HAL_UART_Transmit(&DEBUG_UART, out_buffer, out_length, 1000);
			memset(out_buffer, 0, out_length);
		}
		HAL_Delay(500);
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
  RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL_NONE;
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

  /** Configure the Systick interrupt time
  */
  __HAL_RCC_PLLI2S_ENABLE();
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 10000;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 50000;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
int perform_i2c_scan(I2C_HandleTypeDef* bus, uint8_t result_buffer[], const uint8_t MAX_MATCHES) {
	assert(MAX_MATCHES > 0);

	uint_fast8_t match_count = 0;

	const uint8_t I2C_TRIALS = 3;
	const uint32_t I2C_TIMEOUT = 10;

	for (uint8_t add = MIN_I2C_ADDR; add <= MAX_I2C_ADDR; add++) {
		if (HAL_I2C_IsDeviceReady(bus, add << 1, I2C_TRIALS, I2C_TIMEOUT) != HAL_OK) continue;

		// Avoid writing out of bounds, but keep counting
		if (match_count < MAX_MATCHES) result_buffer[match_count] = add;
		match_count++;
	}

	return match_count;
}

uint8_t parse_address(uint8_t buffer[]) {
	uint8_t address = 0;
	for (int i = 0; i < 2; i++) {
		address = address << 4;
		int8_t temp = buffer[i];

		if (temp >= '0' && temp <= '9') address = address + (temp - '0');
		else if (temp >= 'A' && temp <= 'F') address = address + 10 + (temp - 'A');
		else return UINT8_MAX; // Definitely invalid
	}
	return address;
}

float parse_emissivity(uint8_t buffer[]) {
	uint32_t numerator = 0;
	uint32_t denominator = 1;

	for (uint8_t i = 0; buffer[i] != 0; i++) {
		denominator = denominator * 10;
		numerator = numerator * 10;

		int8_t temp = buffer[i];
		if (temp < '0' || temp > '9') return -1.0; // Definitely invalid
		numerator = numerator + (temp - '0');
	}

	float emissivity = (float)numerator / (float)denominator;
	return emissivity;
}

int handle_calibration(uint8_t* buffer_in, uint32_t length_in, char output_message[], const size_t OUT_BUF_LEN) {
	if (length_in < 1) return 0;
	if (buffer_in[0] == 0) return 0;

	HAL_StatusTypeDef ret = HAL_OK;

	output_message[0] = 0; // Default to an empty string

	int pos = 0;

	enum CLICommandType command = buffer_in[0];

	struct MLXDevice target_mlx = {
			.address = 0,
			.i2c_bus = &WHEEL_I2C,
	};

	switch (command) {
	case CLI_READ_AMBIENT:
	case CLI_READ_OBJECT:
	case CLI_CHANGE_ADDRESS:
	case CLI_SET_EMISSIVITY:
		if (length_in < 3) {
			pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Insufficient characters received for a valid I2C address\n");
			return pos;
		}

		uint8_t temp_buf[3] = {0, 0, 0};
		temp_buf[0] = buffer_in[1];
		temp_buf[1] = buffer_in[2];
		target_mlx.address = parse_address(temp_buf);

		if (!(target_mlx.address >= MIN_I2C_ADDR && target_mlx.address <= MAX_I2C_ADDR)) {
			pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Invalid I2C address specified \'%s\', must be between 0x%02X and 0x%02X inclusive\n", temp_buf, MIN_I2C_ADDR, MAX_I2C_ADDR);
			return pos;
		}
		break;

	case CLI_HELP:
	case CLI_SCAN:
	default:
		break;
	}


	const uint8_t MAX_I2C_MATCHES = 10;
	uint8_t matches[MAX_I2C_MATCHES];

	switch (command) {
	case CLI_HELP:
		pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "\n\nTITAN CALIBRATION FIRMWARE HELP\n");
		pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "All commands follow format \"CA,P\" where:\n\tC is command character\n\tA is target address in hexadecimal\n\tP is optional parameter with leading comma\n");
		pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Valid command characters:\n");
		pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "\t%c - Print this help message (no node/parameter)\n", CLI_HELP);
		pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "\t%c - Scan wheel I2C bus for sensors (A and P will be ignored)\n", CLI_SCAN);
		pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "\t%c - Read the object temperature of the sensor\n", CLI_READ_OBJECT);
		pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "\t%c - Read the ambient temperature of the sensor\n", CLI_READ_AMBIENT);
		pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "\t%c - Set sensor's object's emissivity as P (decimal points after 0, so 0.73 would be \'73\', enter \'0\' for 1.0)\n", CLI_SET_EMISSIVITY);
		pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "\t%c - Change sensor's address from A to P (hexadecimal)\n", CLI_CHANGE_ADDRESS);
		pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "\nNOTE: Sensor address and object emissivity are stored to non-volatile memory AND NEED A REBOOT!\n\n");
		break;
	case CLI_SCAN:
		int match_count = perform_i2c_scan(target_mlx.i2c_bus, matches, MAX_I2C_MATCHES);

		if (match_count == 0) pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "No devices found on wheel I2C bus. Check connections.\n");
		else if (match_count < 0) pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Error happened during I2C scan: %d\n", match_count);
		else if (match_count <= MAX_I2C_MATCHES) pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Found %d devices during I2C scan of wheel bus at these addresses:\n", match_count);
		else pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Found %d devices during scan of wheel bus, but only the first %d addresses can be listed:\n", match_count, MAX_I2C_MATCHES);

		for (int_fast8_t i = 0; i < match_count; i++) {
			pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "\t0x%02X\n", matches[i]);
		}
		break;
	case CLI_CHANGE_ADDRESS:
		if (length_in < 6) {
			pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Insufficient characters received for a valid new I2C address\n");
			break;
		}


		uint8_t temp_buf[3] = {0, 0, 0};
		temp_buf[0] = buffer_in[4];
		temp_buf[1] = buffer_in[5];
		uint8_t new_address = parse_address(temp_buf);
		uint8_t old_address = target_mlx.address;
		if (new_address < MIN_I2C_ADDR || new_address > MAX_I2C_ADDR) {
			pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Invalid new I2C address specified \'%s\', must be between 0x%02X and 0x%02X\n", temp_buf, MIN_I2C_ADDR, MAX_I2C_ADDR);
			break;
		}
		ret = mlx_change_address(&target_mlx, new_address);

		if (ret == HAL_OK) pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Successfully changed brake disk sensor address from 0x%02X to 0x%02X\n", old_address, new_address);
		else pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Failed to change brake disk sensor address from 0x%02X to 0x%02X, error code %d\n", old_address, new_address, ret);
		break;
	case CLI_READ_OBJECT:
		float object_temperature_deg_c = 0;
		ret = mlx_read_object_temperature_deg_c(target_mlx, &object_temperature_deg_c);

		if (ret == HAL_OK) pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Object temperature at 0x%02X is %.2f *C\n", target_mlx.address, object_temperature_deg_c);
		else pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Failed to read object temperature at 0x%02X, error code %d\n", target_mlx.address, ret);
		break;

	case CLI_READ_AMBIENT:
		float ambient_temperature_deg_c = 0;
		ret = mlx_read_object_temperature_deg_c(target_mlx, &ambient_temperature_deg_c);

		if (ret == HAL_OK) pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Ambient temperature at 0x%02X is %.2f *C\n", target_mlx.address, ambient_temperature_deg_c);
		else pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Failed to read ambient temperature at 0x%02X, error code %d\n", target_mlx.address, ret);
		break;

	case CLI_SET_EMISSIVITY:
		if (length_in < 5) {
			pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Insufficient characters received for a valid new emissivity\n");
			break;
		}
		float new_emissivity = parse_emissivity(&buffer_in[4]);
		if (new_emissivity == 0) new_emissivity = 1;

		if (new_emissivity < 0) {
			pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Invalid numeric string provided for emissivity: \'%s\'\n", &buffer_in[4]);
			break;
		}

		ret = mlx_write_emissivity(target_mlx, new_emissivity);

		if (ret == HAL_OK) pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Configured object emissivity at 0x%02X to %.4f\n", target_mlx.address, new_emissivity);
		else pos += snprintf(&output_message[pos], OUT_BUF_LEN - pos, "Failed to set object emissivity at 0x%02X, error code %d\n", target_mlx.address, ret);
		break;

	case CLI_UNKNOWN:
	default:
		pos = snprintf(&output_message[pos], OUT_BUF_LEN, "Command not recognized \"%s\". Enter \'%c\' for help message\n", buffer_in, CLI_HELP);
		break;
	}

	return pos;
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
#ifdef USE_FULL_ASSERT
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
