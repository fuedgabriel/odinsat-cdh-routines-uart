/**
  ******************************************************************************
  * @file    adcs.h
  * @brief   ADCS (Attitude Determination and Control System) protocol
  *          Comunicação com motor SimpleFOC via UART4
  * 
  * Arduino (PS4)                 →  STM32 (CDH)
  * Serial.print("M125")          →  ADCS_SetSpeed(125)
  * Serial.print("M-80")          →  ADCS_SetSpeed(-80)
  * Serial.print("M0")            →  ADCS_Stop()
  * if (lStickX >= -10 && <= 10)  →  Dead zone automática
  ******************************************************************************
  */

#ifndef __ADCS_H
#define __ADCS_H


#include "usart.h"
#include <stdint.h>
#include "BMI088.h"

/* ============================================================================
   DEFINIÇÕES DO PROTOCOLO ADCS
   ============================================================================ */
#define ADCS_DEAD_ZONE          10      // Dead zone para evitar drift
#define ADCS_MAX_SPEED          127     // Velocidade máxima (-127 a +127)
#define ADCS_MIN_SPEED          -127
#define ADCS_STOP_SPEED         0

/* Comandos SimpleFOC */
#define ADCS_CMD_STOP           "M0\n"      // Para o motor
#define ADCS_CMD_SELECT_MOTOR   "MC1\n"    // Seleciona motor 1

/* ============================================================================
   ESTRUTURAS DE DADOS
   ============================================================================ */

/* Estado do ADCS */
typedef struct {
    int16_t target_speed;       // Velocidade alvo (-127 a +127)
    int16_t current_speed;      // Velocidade atual do motor
    uint8_t motor_active;       // Flag: motor ativo?
    uint8_t motor_initialized;  // Flag: motor inicializado?
} ADCS_State_t;

/* Dados dos sensores para PID (estrutura para expansão futura) */
typedef struct {
    float gyro_x;            
    float gyro_y;              
    float gyro_z;               
    float accel_x;             
    float accel_y;              
    float accel_z;             
} ADCS_Sensors_t;

/* Parâmetros do PID */
typedef struct {
    float Kp;                   // Ganho proporcional
    float Ki;                   // Ganho integral
    float Kd;                   // Ganho derivativo
    float setpoint;             // Valor desejado
    float error_sum;            // Soma dos erros (integral)
    float last_error;           // Último erro (derivativo)
} ADCS_PID_t;

/* ============================================================================
   FUNÇÕES PÚBLICAS
   ============================================================================ */

// Inicialização
void ADCS_Init(UART_HandleTypeDef *huart);

// Controle do motor

void ADCS_SetSpeed(UART_HandleTypeDef *huart, int16_t speed);
void ADCS_Stop(UART_HandleTypeDef *huart);
void ADCS_SendCommand(UART_HandleTypeDef *huart, const char *cmd);

void ADCS_ReadSensors(ADCS_Sensors_t *sensors);

// Controle PID
float ADCS_PID_Compute(ADCS_PID_t *pid, float current_value);
void ADCS_PID_Reset(ADCS_PID_t *pid);

// Rotina ADCS (chamada no loop principal)
void ADCS_Process(UART_HandleTypeDef *huart);

// Getters de estado
int16_t ADCS_GetCurrentSpeed(void);
int16_t ADCS_GetTargetSpeed(void);
uint8_t ADCS_IsMotorActive(void);
uint8_t ADCS_IsSensorReady(void);

// Debug: Acesso direto ao IMU (opcional)
#ifdef __cplusplus
extern "C" {
#endif
BMI088* ADCS_GetIMU(void);
#ifdef __cplusplus
}
#endif

#endif /* __ADCS_H */