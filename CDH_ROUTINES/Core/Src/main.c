/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "fdcan.h"
#include "i2c.h"
#include "quadspi.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SolarTracker.h"
#include "can_driver.h"
#include "can_protocol.h"
#include "uart_protocol.h"
#include "adcs.h"
#include "antena.h"
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

/* USER CODE BEGIN PV */
SolarTracker_t satelite;
int16_t velo = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

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

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();


  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_I2C1_Init();
  MX_FDCAN1_Init();
  MX_UART5_Init();
  MX_QUADSPI_Init();
  MX_I2C3_Init();
  MX_SPI4_Init();
  MX_SPI1_Init();
  MX_UART4_Init();
  MX_UART8_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  //CAN_Init();
  //CAN_Protocol_Init();
  //HAL_Delay(10);  // Aguarda CAN estabilizar
  
  ADCS_Init(&huart4);  // Inicializa ADCS (motor SimpleFOC)
  HAL_Delay(10);

  // --- ADICIONADO: INICIALIZAÇÃO DO SOLAR TRACKER ---
  // Calibração dos ADCs para garantir precisão na leitura de luz
  if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK) Error_Handler();
  if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK) Error_Handler();

  // Inicializa a estrutura do satélite (zera variáveis e define calibração padrão)
  Solar_Init(&satelite);
  // --------------------------------------------------

  // Envia mensagem de boot (8 bytes)
  /*
  CAN_Message_t tx_msg = {
    .id = 0x6A5,
    .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
  };
  CAN_Transmit(&tx_msg);
  */

  
  // Aguarda transmissão completar
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    // Teste rápido da UART_PROTOCOL na COM5
    UART_Message_t msg;
    msg.id = MSG_CMD_START_M2;
    msg.length = 3;
    msg.data[0] = 0xAA;
    msg.data[1] = 0xBB;
    msg.data[2] = 0xCC;
    UART_Transmit(&huart5, &msg, msg.length);
    
    // Aguarda o payload processar e responder (importante!)
    HAL_Delay(200);  // 200ms para dar tempo do Python processar

    Check_Payload_Response();
    
    // Nota: Esse delay de 2s vai fazer o SolarTracker atualizar a cada 2s.
    // Se precisar de resposta rápida do motor, diminua esse tempo.
//    ADCS_SetSpeed(&huart4, velo);

    // Primeira parte: aumentar a velocidade até 100
    while (velo <= 100) {
        ADCS_SetSpeed(&huart4, velo);
        velo += 15;
        HAL_Delay(3000);  // Atraso de 3 segundos
    }

    // Segunda parte: diminuir a velocidade até 0
    while (velo >= 0) {
        ADCS_SetSpeed(&huart4, velo);
        velo -= 15;
        HAL_Delay(3000);  // Atraso de 3 segundos
    }

    // Terceira parte: diminuir a velocidade até -100
    while (velo >= -100) {
        ADCS_SetSpeed(&huart4, velo);
        velo -= 15;
        HAL_Delay(3000);  // Atraso de 3 segundos
    }



    HAL_Delay(2000);

    // testa deploy da antena
    //Deploy_Antenna();

    /* ... (Seus blocos comentados de CAN permanecem aqui) ... */

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // --- ADICIONADO: LÓGICA DO SOLAR TRACKER ---

    // 1. LEITURA DOS SENSORES (Polling ADC2 e ADC3)
    // Nota: A ordem depende do 'Rank' configurado no CubeMX, mas polling força a leitura sequencial se configurado corretamente.

    // Leitura ADC2 (Canais 0, 1, 2 da struct)
//    HAL_ADC_Start(&hadc2); HAL_ADC_PollForConversion(&hadc2, 10); satelite.adc_raw[0] = HAL_ADC_GetValue(&hadc2);
//    HAL_ADC_Start(&hadc2); HAL_ADC_PollForConversion(&hadc2, 10); satelite.adc_raw[1] = HAL_ADC_GetValue(&hadc2);
//    HAL_ADC_Start(&hadc2); HAL_ADC_PollForConversion(&hadc2, 10); satelite.adc_raw[2] = HAL_ADC_GetValue(&hadc2);

    // Leitura ADC3 (Canais 3 a 7 da struct)
//    HAL_ADC_Start(&hadc3); HAL_ADC_PollForConversion(&hadc3, 10); satelite.adc_raw[3] = HAL_ADC_GetValue(&hadc3);
//    HAL_ADC_Start(&hadc3); HAL_ADC_PollForConversion(&hadc3, 10); satelite.adc_raw[4] = HAL_ADC_GetValue(&hadc3);
//    HAL_ADC_Start(&hadc3); HAL_ADC_PollForConversion(&hadc3, 10); satelite.adc_raw[5] = HAL_ADC_GetValue(&hadc3);
//    HAL_ADC_Start(&hadc3); HAL_ADC_PollForConversion(&hadc3, 10); satelite.adc_raw[6] = HAL_ADC_GetValue(&hadc3);
//    HAL_ADC_Start(&hadc3); HAL_ADC_PollForConversion(&hadc3, 10); satelite.adc_raw[7] = HAL_ADC_GetValue(&hadc3);

    // 2. PROCESSAMENTO MATEMÁTICO
    // Calcula: Normalização -> Ângulo Solar -> Ação do Motor
//    Solar_Process(&satelite);

    // 3. ATUAÇÃO DO MOTOR (Exemplo de integração)
    // Aqui você conecta a lógica do tracker com seu driver de motor real

    /* Exemplo:
    if (satelite.acao_motor == 1) {
        // Enviar comando para girar Horário
    } else if (satelite.acao_motor == -1) {
        // Enviar comando para girar Anti-Horário
    } else {
        // Enviar comando para Parar
    }
    */

    // Debug: Você pode inspecionar 'satelite.angulo_solar_graus' no Live Watch do STM32CubeIDE.
    // -------------------------------------------
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 9;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOMEDIUM;
  RCC_OscInitStruct.PLL.PLLFRACN = 3072;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLL2.PLL2M = 1;
  PeriphClkInitStruct.PLL2.PLL2N = 9;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 3072;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
  * where the assert_param error has occurred.
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
