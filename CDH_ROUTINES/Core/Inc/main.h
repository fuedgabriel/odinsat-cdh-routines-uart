/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OBC_SCK_Pin GPIO_PIN_2
#define OBC_SCK_GPIO_Port GPIOE
#define INT_GYR_Pin GPIO_PIN_3
#define INT_GYR_GPIO_Port GPIOE
#define OBC_CS_ACC_Pin GPIO_PIN_4
#define OBC_CS_ACC_GPIO_Port GPIOE
#define OBC_MISO_Pin GPIO_PIN_5
#define OBC_MISO_GPIO_Port GPIOE
#define OBC_MOSI_Pin GPIO_PIN_6
#define OBC_MOSI_GPIO_Port GPIOE
#define INT_ACC_Pin GPIO_PIN_8
#define INT_ACC_GPIO_Port GPIOI
#define SUN_SENSOR_X__1_Pin GPIO_PIN_3
#define SUN_SENSOR_X__1_GPIO_Port GPIOF
#define SUN_SENSOR_X__2_Pin GPIO_PIN_4
#define SUN_SENSOR_X__2_GPIO_Port GPIOF
#define SUN_SENSOR_X__3_Pin GPIO_PIN_5
#define SUN_SENSOR_X__3_GPIO_Port GPIOF
#define SUN_SENSOR_Y__1_Pin GPIO_PIN_1
#define SUN_SENSOR_Y__1_GPIO_Port GPIOC
#define SUN_SENSOR_Y__2_Pin GPIO_PIN_2
#define SUN_SENSOR_Y__2_GPIO_Port GPIOC
#define SUN_SENSOR_Y__3_Pin GPIO_PIN_3
#define SUN_SENSOR_Y__3_GPIO_Port GPIOC
#define ADCS_TX_Pin GPIO_PIN_0
#define ADCS_TX_GPIO_Port GPIOA
#define ADCS_RX_Pin GPIO_PIN_1
#define ADCS_RX_GPIO_Port GPIOA
#define ADCS_PWM_Pin GPIO_PIN_2
#define ADCS_PWM_GPIO_Port GPIOA
#define ADCS_DIR_Pin GPIO_PIN_2
#define ADCS_DIR_GPIO_Port GPIOH
#define ADCS_EN_Pin GPIO_PIN_3
#define ADCS_EN_GPIO_Port GPIOH
#define SUN_SENSOR_X__1H4_Pin GPIO_PIN_4
#define SUN_SENSOR_X__1H4_GPIO_Port GPIOH
#define SUN_SENSOR_X__2H5_Pin GPIO_PIN_5
#define SUN_SENSOR_X__2H5_GPIO_Port GPIOH
#define SUN_SENSOR_X__3A3_Pin GPIO_PIN_3
#define SUN_SENSOR_X__3A3_GPIO_Port GPIOA
#define RADIO_SCK_Pin GPIO_PIN_5
#define RADIO_SCK_GPIO_Port GPIOA
#define RADIO_MISO_Pin GPIO_PIN_6
#define RADIO_MISO_GPIO_Port GPIOA
#define RADIO_MOSI_Pin GPIO_PIN_7
#define RADIO_MOSI_GPIO_Port GPIOA
#define ANT_DEPLOYER_IO_3_Pin GPIO_PIN_5
#define ANT_DEPLOYER_IO_3_GPIO_Port GPIOC
#define ANT_DEPLOYER_IO_2_Pin GPIO_PIN_0
#define ANT_DEPLOYER_IO_2_GPIO_Port GPIOB
#define ANT_DEPLOYER_IO_1_Pin GPIO_PIN_1
#define ANT_DEPLOYER_IO_1_GPIO_Port GPIOB
#define SUN_SENSOR_Y__3F12_Pin GPIO_PIN_12
#define SUN_SENSOR_Y__3F12_GPIO_Port GPIOF
#define SUN_SENSOR_Y__2F13_Pin GPIO_PIN_13
#define SUN_SENSOR_Y__2F13_GPIO_Port GPIOF
#define SUN_SENSOR_Y__1F14_Pin GPIO_PIN_14
#define SUN_SENSOR_Y__1F14_GPIO_Port GPIOF
#define SYS_SCL_Pin GPIO_PIN_7
#define SYS_SCL_GPIO_Port GPIOH
#define SYS_SDA_Pin GPIO_PIN_8
#define SYS_SDA_GPIO_Port GPIOH
#define PAY_SCL_Pin GPIO_PIN_11
#define PAY_SCL_GPIO_Port GPIOH
#define PAY_SDA_Pin GPIO_PIN_12
#define PAY_SDA_GPIO_Port GPIOH
#define SYS_SCK_Pin GPIO_PIN_13
#define SYS_SCK_GPIO_Port GPIOB
#define SYS_MISO_Pin GPIO_PIN_14
#define SYS_MISO_GPIO_Port GPIOB
#define SYS_MOSI_Pin GPIO_PIN_15
#define SYS_MOSI_GPIO_Port GPIOB
#define PAY_TX_Pin GPIO_PIN_8
#define PAY_TX_GPIO_Port GPIOD
#define PAY_RX_Pin GPIO_PIN_9
#define PAY_RX_GPIO_Port GPIOD
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define GPHIO_02_Pin GPIO_PIN_0
#define GPHIO_02_GPIO_Port GPIOI
#define GPHIO_01_Pin GPIO_PIN_1
#define GPHIO_01_GPIO_Port GPIOI
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define LED_A_Pin GPIO_PIN_7
#define LED_A_GPIO_Port GPIOD
#define LED_B_Pin GPIO_PIN_9
#define LED_B_GPIO_Port GPIOG
#define LED_C_Pin GPIO_PIN_10
#define LED_C_GPIO_Port GPIOG
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define DEBUG_UART_RX_Pin GPIO_PIN_5
#define DEBUG_UART_RX_GPIO_Port GPIOB
#define DEBUG_UART_TX_Pin GPIO_PIN_6
#define DEBUG_UART_TX_GPIO_Port GPIOB
#define OBC_SCL_Pin GPIO_PIN_8
#define OBC_SCL_GPIO_Port GPIOB
#define OBC_SDA_Pin GPIO_PIN_9
#define OBC_SDA_GPIO_Port GPIOB
#define CAM_RX_Pin GPIO_PIN_0
#define CAM_RX_GPIO_Port GPIOE
#define CAM_TX_Pin GPIO_PIN_1
#define CAM_TX_GPIO_Port GPIOE
#define CAM_IO_1_Pin GPIO_PIN_5
#define CAM_IO_1_GPIO_Port GPIOI
#define CAM_IO_2_Pin GPIO_PIN_6
#define CAM_IO_2_GPIO_Port GPIOI
#define OBC_CS_GYR_Pin GPIO_PIN_7
#define OBC_CS_GYR_GPIO_Port GPIOI

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
