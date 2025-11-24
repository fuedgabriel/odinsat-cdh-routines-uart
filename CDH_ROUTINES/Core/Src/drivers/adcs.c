/**
  ******************************************************************************
  * @file    adcs.c
  * @brief   ADCS implementation - Controle de motor SimpleFOC
  ******************************************************************************
  */

#include "adcs.h"
#include "can_protocol.h"
#include "BMI088.h"
#include "spi.h"
#include "gpio.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ============================================================================
   VARIÁVEIS PRIVADAS
   ============================================================================ */

// Instância do BMI088
static BMI088 imu;
static uint8_t bmi088_initialized = 0;

static ADCS_State_t adcs_state = {
    .target_speed = 0,
    .current_speed = 0,
    .motor_active = 0,
    .motor_initialized = 0
};

static ADCS_Sensors_t sensors = {0};

static ADCS_PID_t pid_controller = {
    .Kp = 1.0f,
    .Ki = 0.1f,
    .Kd = 0.05f,
    .setpoint = 0.0f,
    .error_sum = 0.0f,
    .last_error = 0.0f
};

/* ============================================================================
   INICIALIZAÇÃO
   ============================================================================ */
/**
 * @brief Inicializa o sistema ADCS
 */
void ADCS_Init(UART_HandleTypeDef *huart)
{
    // Reseta estado
    adcs_state.target_speed = 0;
    adcs_state.current_speed = 0;
    adcs_state.motor_active = 0;
    adcs_state.motor_initialized = 0;
    
    // Reseta PID
    ADCS_PID_Reset(&pid_controller);
    
    // ===== INICIALIZA BMI088 (Sensor IMU) =====
    // SPI4: Acelerômetro CS = OBC_CS_ACC (GPIOE), Giroscópio CS = OBC_CS_GYR (GPIOI)
    if (!bmi088_initialized) {
        extern SPI_HandleTypeDef hspi4;
        
        // Inicializa BMI088 com SPI4
        uint8_t imu_status = BMI088_Init(&imu, 
                                          &hspi4, 
                                          OBC_CS_ACC_GPIO_Port, OBC_CS_ACC_Pin,
                                          OBC_CS_GYR_GPIO_Port, OBC_CS_GYR_Pin);
        
        if (imu_status == HAL_OK) {
            bmi088_initialized = 1;
            // Debug: Sensor inicializado com sucesso
        } else {
            // Debug: Falha ao inicializar sensor
            bmi088_initialized = 0;
        }
        
        HAL_Delay(100);
    }
    
    // Pequeno delay para estabilizar UART
    HAL_Delay(100);
    
    // Envia comando de seleção de motor (equivalente ao Serial.print("MC1\\n"))
    ADCS_SendCommand(huart, ADCS_CMD_SELECT_MOTOR);
    HAL_Delay(50);
    
    adcs_state.motor_initialized = 1;
}

/* ============================================================================
   COMUNICAÇÃO COM SIMPLEFOC
   ============================================================================ */
/**
 * @brief Envia comando genérico para o motor SimpleFOC
 * @param cmd String de comando (ex: "M0\\n", "MC1\\n")
 */
void ADCS_SendCommand(UART_HandleTypeDef *huart, const char *cmd)
{
    HAL_UART_Transmit(huart, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);
}

/**
 * @brief Define velocidade do motor (-127 a +127)
 * @param speed Velocidade desejada (negativo = esquerda, positivo = direita)
 */
void ADCS_SetSpeed(UART_HandleTypeDef *huart, int16_t speed)
{
    // Aplica dead zone
    if (speed >= -ADCS_DEAD_ZONE && speed <= ADCS_DEAD_ZONE) {
        speed = ADCS_STOP_SPEED;
    }
    
    // Limita velocidade
    if (speed > ADCS_MAX_SPEED) speed = ADCS_MAX_SPEED;
    if (speed < ADCS_MIN_SPEED) speed = ADCS_MIN_SPEED;
    
    adcs_state.target_speed = speed;
    
    // Monta comando no formato "M{speed}\\n" (ex: "M-125\\n" ou "M125\\n")
    char cmd_buffer[16];
    snprintf(cmd_buffer, sizeof(cmd_buffer), "M%d\n", speed);
    
    // Envia via UART
    ADCS_SendCommand(huart, cmd_buffer);
    
    // Atualiza flags
    if (speed == 0) {
        adcs_state.motor_active = 0;
    } else {
        adcs_state.motor_active = 1;
    }
    
    adcs_state.current_speed = speed;
}

/**
 * @brief Para o motor imediatamente
 */
void ADCS_Stop(UART_HandleTypeDef *huart)
{
    ADCS_SendCommand(huart, ADCS_CMD_STOP);
    adcs_state.target_speed = 0;
    adcs_state.current_speed = 0;
    adcs_state.motor_active = 0;
}

/* ============================================================================
   LEITURA DE SENSORES (TODO: Implementar com I2C/SPI)
   ============================================================================ */
/**
 * @brief Lê dados dos sensores IMU
 * @param sensors Ponteiro para estrutura de sensores
 * @note TODO: Implementar leitura real de MPU6050, LSM6DS3, etc.
 */
void ADCS_ReadSensors(ADCS_Sensors_t *sensors)
{
    if (!bmi088_initialized) {
        // Sensor não inicializado, retorna valores zero
        sensors->gyro_x = 0.0f;
        sensors->gyro_y = 0.0f;
        sensors->gyro_z = 0.0f;
        sensors->accel_x = 0.0f;
        sensors->accel_y = 0.0f;
        sensors->accel_z = 0.0f;
        return;
    }
    
    // Lê acelerômetro e giroscópio via SPI
    HAL_StatusTypeDef acc_status = BMI088_ReadAccelerometer(&imu);
    HAL_StatusTypeDef gyr_status = BMI088_ReadGyroscope(&imu);
    
    if (acc_status == HAL_OK && gyr_status == HAL_OK) {
        // Copia dados do BMI088 para estrutura ADCS
        // Giroscópio em rad/s (BMI088 já retorna em rad/s)
        sensors->gyro_x = imu.gyr_rps[0];
        sensors->gyro_y = imu.gyr_rps[1];
        sensors->gyro_z = imu.gyr_rps[2];
        
        // Acelerômetro em m/s² (BMI088 já retorna em m/s²)
        sensors->accel_x = imu.acc_mps2[0];
        sensors->accel_y = imu.acc_mps2[1];
        sensors->accel_z = imu.acc_mps2[2];
    } else {
        // Erro na leitura, mantém valores anteriores ou zera
        // (Opcional: implementar lógica de fallback)
    }
}

/* ============================================================================
   CONTROLE PID
   ============================================================================ */
/**
 * @brief Calcula saída do controlador PID
 * @param pid Ponteiro para estrutura PID
 * @param current_value Valor atual medido
 * @return Saída do controlador PID
 */
float ADCS_PID_Compute(ADCS_PID_t *pid, float current_value)
{
    // Calcula erro
    float error = pid->setpoint - current_value;
    
    // Termo Proporcional
    float P = pid->Kp * error;
    
    // Termo Integral (com anti-windup simples)
    pid->error_sum += error;
    if (pid->error_sum > 100.0f) pid->error_sum = 100.0f;
    if (pid->error_sum < -100.0f) pid->error_sum = -100.0f;
    float I = pid->Ki * pid->error_sum;
    
    // Termo Derivativo
    float derivative = error - pid->last_error;
    float D = pid->Kd * derivative;
    
    // Atualiza último erro
    pid->last_error = error;
    
    // Saída PID
    float output = P + I + D;
    
    return output;
}

/**
 * @brief Reseta o controlador PID
 * @param pid Ponteiro para estrutura PID
 */
void ADCS_PID_Reset(ADCS_PID_t *pid)
{
    pid->error_sum = 0.0f;
    pid->last_error = 0.0f;
}

/* ============================================================================
   ROTINA PRINCIPAL ADCS
   ============================================================================ */
/**
 * @brief Processa rotina ADCS (chamada no loop principal)
 * @note Implementa controle baseado em sensores ou comandos CAN
 */
void ADCS_Process(UART_HandleTypeDef *huart)
{
    // Verifica se motor está inicializado
    if (!adcs_state.motor_initialized) {
        ADCS_Init(huart);
        return;
    }
    
    // Verifica modo atual do CAN
    CDH_OperationMode_t mode = CAN_GetCurrentMode();
    
    // Só processa se estiver em modo ADCS ou DETUMBLING
    if (mode != CDH_MODE_ADCS && mode != CDH_MODE_DETUMBLING) {
        // Se não está em modo ADCS, para o motor
        if (adcs_state.motor_active) {
            ADCS_Stop(huart);
        }
        return;
    }
    
    // Lê sensores
    ADCS_ReadSensors(&sensors);
    
    // ===== MODO ADCS: Controle PID baseado em giroscópio =====
    if (mode == CDH_MODE_ADCS) {
        // Define setpoint (velocidade angular desejada)
        pid_controller.setpoint = 0.0f;  // 0 deg/s = estável
        
        // Calcula controle PID baseado no giroscópio Z
        float pid_output = ADCS_PID_Compute(&pid_controller, sensors.gyro_z);
        
        // Converte saída PID para velocidade do motor (-127 a +127)
        int16_t motor_speed = (int16_t)(pid_output * 10.0f);  // Ajuste de escala
        
        // Envia comando para motor
        ADCS_SetSpeed(huart, motor_speed);
    }
    
    // ===== MODO DETUMBLING: Amortecimento de rotação =====
    else if (mode == CDH_MODE_DETUMBLING) {
        /*

                    TO DO!

        */
        // Lógica de detumbling: aplicar torque oposto à rotação
        // Quanto maior a velocidade angular, maior o torque de frenagem
        
        float angular_velocity = sensors.gyro_z;
        int16_t brake_speed = (int16_t)(-angular_velocity * 5.0f);  // Ganho negativo
        
        // Limita velocidade de frenagem
        if (brake_speed > 50) brake_speed = 50;
        if (brake_speed < -50) brake_speed = -50;
        
        ADCS_SetSpeed(huart, brake_speed);
        
        // Se velocidade angular está muito baixa, considera destumbling completo
        if (fabs(angular_velocity) < 0.5f) {  // < 0.5 deg/s
            ADCS_Stop(huart);
        }
    }
}

/* ============================================================================
   GETTERS
   ============================================================================ */
/**
 * @brief Retorna velocidade atual do motor
 */
int16_t ADCS_GetCurrentSpeed(void)
{
    return adcs_state.current_speed;
}

/**
 * @brief Retorna velocidade alvo do motor
 */
int16_t ADCS_GetTargetSpeed(void)
{
    return adcs_state.target_speed;
}

/**
 * @brief Retorna se motor está ativo
 */
uint8_t ADCS_IsMotorActive(void)
{
    return adcs_state.motor_active;
}

/**
 * @brief Retorna se o sensor BMI088 está inicializado
 */
uint8_t ADCS_IsSensorReady(void)
{
    return bmi088_initialized;
}

/**
 * @brief Retorna ponteiro para dados brutos do IMU (para debug)
 */
BMI088* ADCS_GetIMU(void)
{
    return &imu;
}