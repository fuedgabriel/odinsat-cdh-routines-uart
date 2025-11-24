/**
  ******************************************************************************
  * @file    can_protocol.h
  * @brief   Protocolo CAN para comunicação entre subsistemas do CubeSat
  ******************************************************************************
  */

#ifndef __CAN_PROTOCOL_H
#define __CAN_PROTOCOL_H

#include <stdint.h>
#include "can_driver.h"

/* ============================================================================
   ENDEREÇOS BASE DOS SUBSISTEMAS
   ============================================================================ */
#define CAN_ADDR_CDH_BASE       0x100  // CDH
#define CAN_ADDR_EPS_BASE       0x200  // EPS
#define CAN_ADDR_COM_BASE       0x300  // COM

/* ============================================================================
   COMANDOS CDH (0x100 - 0x1FF)
   ============================================================================ */
#define CAN_CDH_TELEMETRY       (CAN_ADDR_CDH_BASE + 0x00)  // 0x100 - Telemetria geral
#define CAN_CDH_STATUS          (CAN_ADDR_CDH_BASE + 0x01)  // 0x101 - Status atual
#define CAN_CDH_ERROR           (CAN_ADDR_CDH_BASE + 0x03)  // 0x103 - Erro reportado

/* ============================================================================
   COMANDOS EPS (0x200 - 0x2FF)
   ============================================================================ */
#define CAN_EPS_TELEMETRY       (CAN_ADDR_EPS_BASE + 0x00)  // 0x200 - Telemetria EPS
#define CAN_EPS_BATTERY       (CAN_ADDR_EPS_BASE + 0x01)  // 0x201 - Cell voltage 0/ Cell Voltage 1/ Cell 0 Depth of Discharge / Cell 1 Depth of Discharge
#define CAN_EPS_SOLAR_PANEL_VOLTAGE   (CAN_ADDR_EPS_BASE + 0x02)  // 0x202 - Voltagem dos painéis solares
#define CAN_EPS_SOLAR_PANEL_CURRENT   (CAN_ADDR_EPS_BASE + 0x03)  // 0x203 - Corrente dos painéis solares


/* ============================================================================
   COMANDOS COM (0x300 - 0x3FF)
   ============================================================================ */
// Comandos de modo de operação
#define CAN_COM_MODE_IDLE       (CAN_ADDR_COM_BASE + 0x00)  // 0x300 - Entrar em modo IDLE
#define CAN_COM_MODE_NOMINAL    (CAN_ADDR_COM_BASE + 0x01)  // 0x301 - Entrar em modo NOMINAL (data[0] = missão)
#define CAN_COM_MODE_ADCS       (CAN_ADDR_COM_BASE + 0x02)  // 0x302 - Entrar em modo ADCS
#define CAN_COM_MODE_DETUMBLING (CAN_ADDR_COM_BASE + 0x03)  // 0x303 - Entrar em modo DETUMBLING
#define CAN_COM_MODE_EXIT       (CAN_ADDR_COM_BASE + 0x0F)  // 0x30F - Sair do modo atual

// Dados de missão
#define CAN_COM_AIS_DATA        (CAN_ADDR_COM_BASE + 0x20)  // 0x320 - Dados AIS 

/* ============================================================================
   MODOS DE OPERAÇÃO DO CDH
   ============================================================================ */
typedef enum {
    CDH_MODE_IDLE = 0,       // Modo ocioso
    CDH_MODE_NOMINAL,        // Modo nominal (missão 1 ou 2)
    CDH_MODE_ADCS,          // Modo ADCS
    CDH_MODE_DETUMBLING     // Modo destumbling
} CDH_OperationMode_t;

/* ============================================================================
   TIPOS DE MISSÃO NOMINAL
   ============================================================================ */
typedef enum {
    MISSION_NONE = 0,
    MISSION_1 = 1,
    MISSION_2 = 2  // Missão com dados AIS
} MissionType_t;

/* ============================================================================
   ESTRUTURAS DE DADOS
   ============================================================================ */

/* Status do CDH */
typedef struct {
    CDH_OperationMode_t current_mode;
    MissionType_t mission_type;
    uint8_t mode_active;  // flag indicando se está executando uma rotina
} CDH_Status_t;

/* Telemetria EPS - Estrutura organizada por arrays */
typedef struct {
    // Battery data (ID: 0x201)
    uint16_t cell_voltage[2];           // [0] = Cell 0, [1] = Cell 1
    uint16_t cell_depth_discharge[2];   // [0] = Cell 0 DoD, [1] = Cell 1 DoD
    
    // Solar Panel Voltage (ID: 0x202)
    uint32_t solar_voltage_1_2;         // Painéis 1 e 2
    uint32_t solar_voltage_3_4;         // Painéis 3 e 4
    
    // Solar Panel Current (ID: 0x203)
    uint32_t solar_current_1_2;         // Painéis 1 e 2
    uint32_t solar_current_3_4;         // Painéis 3 e 4
} EPS_Telemetry_t;

/* Dados AIS da Missão 2 */
typedef struct {
    uint32_t mmsi;          // Maritime Mobile Service Identity
    uint8_t data[8];        // Dados brutos do pacote AIS
    uint8_t packet_index;   // Índice do pacote (se fragmentado)
} AIS_Data_t;

/* Getters para estado atual CDH */
CDH_OperationMode_t CAN_GetCurrentMode(void);
MissionType_t CAN_GetMissionType(void);
uint8_t CAN_IsModeActive(void);

/* Getters para telemetria EPS */
uint16_t CAN_GetCellVoltage(uint8_t cell_index);        // cell_index: 0 ou 1
uint16_t CAN_GetCellDepthDischarge(uint8_t cell_index); // cell_index: 0 ou 1
uint32_t CAN_GetSolarVoltage_1_2(void);
uint32_t CAN_GetSolarVoltage_3_4(void);
uint32_t CAN_GetSolarCurrent_1_2(void);
uint32_t CAN_GetSolarCurrent_3_4(void);

/* Inicialização do protocolo */
void CAN_Protocol_Init(void);

/* Processamento de mensagens recebidas */
void CAN_Protocol_ProcessMessages(void);

/* Envio de telemetria CDH */
void CAN_Protocol_SendCDHStatus(void);

/* Handlers para comandos COM */
void CAN_HandleModeCommand(uint32_t mode_id, uint8_t *data);
void CAN_HandleAISData(uint8_t *data);

/* Handlers para telemetria EPS */
void CAN_HandleEPSTelemetry(uint32_t msg_id, uint8_t *data);

#endif /* __CAN_PROTOCOL_H */
