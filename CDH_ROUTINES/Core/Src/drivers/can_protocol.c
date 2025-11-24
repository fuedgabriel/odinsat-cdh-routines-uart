/**
  ******************************************************************************
  * @file    can_protocol.c
  * @brief   Implementação do protocolo CAN para CubeSat
  ******************************************************************************
  */

#include "can_protocol.h"
#include "can_driver.h"
#include "main.h"
#include <string.h>

/* ============================================================================
   VARIÁVEIS PRIVADAS
   ============================================================================ */
static CDH_Status_t cdh_status = {
    .current_mode = CDH_MODE_IDLE,
    .mission_type = MISSION_NONE,
    .mode_active = 0
};

static EPS_Telemetry_t eps_telemetry = {0};
static AIS_Data_t ais_buffer = {0};

/* ============================================================================
   INICIALIZAÇÃO
   ============================================================================ */
void CAN_Protocol_Init(void)
{
    // Inicializa driver CAN
    CAN_Init();
    
    // Estado inicial: IDLE
    cdh_status.current_mode = CDH_MODE_IDLE;
    cdh_status.mission_type = MISSION_NONE;
    cdh_status.mode_active = 0;
    
    // Envia status inicial
    CAN_Protocol_SendCDHStatus();
}

/* ============================================================================
   PROCESSAMENTO DE MENSAGENS
   ============================================================================ */
void CAN_Protocol_ProcessMessages(void)
{
    CAN_Message_t rx_msg;
    
    if (CAN_GetMessage(&rx_msg)) {
        
        /* ========== COMANDOS DE MODO (COM) ========== */
        if (rx_msg.id == CAN_COM_MODE_IDLE) {
            CAN_HandleModeCommand(CAN_COM_MODE_IDLE, rx_msg.data);
        }
        else if (rx_msg.id == CAN_COM_MODE_NOMINAL) {
            CAN_HandleModeCommand(CAN_COM_MODE_NOMINAL, rx_msg.data);
        }
        else if (rx_msg.id == CAN_COM_MODE_ADCS) {
            CAN_HandleModeCommand(CAN_COM_MODE_ADCS, rx_msg.data);
        }
        else if (rx_msg.id == CAN_COM_MODE_DETUMBLING) {
            CAN_HandleModeCommand(CAN_COM_MODE_DETUMBLING, rx_msg.data);
        }
        else if (rx_msg.id == CAN_COM_MODE_EXIT) {
            CAN_HandleModeCommand(CAN_COM_MODE_EXIT, rx_msg.data);
        }
        
        /* ========== DADOS AIS (MISSÃO 2) ========== */
        else if (rx_msg.id == CAN_COM_AIS_DATA) {
            CAN_HandleAISData(rx_msg.data);
        }
        
        /* ========== TELEMETRIA EPS ========== */
        else if (rx_msg.id >= CAN_ADDR_EPS_BASE && rx_msg.id < CAN_ADDR_COM_BASE) {
            CAN_HandleEPSTelemetry(rx_msg.id, rx_msg.data);
        }
        
    }
}

/* ============================================================================
   HANDLERS DE COMANDOS DE MODO
   ============================================================================ */
void CAN_HandleModeCommand(uint32_t mode_id, uint8_t *data)
{
    CDH_OperationMode_t new_mode;
    uint8_t success = 1;
    
    switch (mode_id) {
        case CAN_COM_MODE_IDLE:
            new_mode = CDH_MODE_IDLE;
            cdh_status.mode_active = 0;
            cdh_status.mission_type = MISSION_NONE;
            break;
            
        case CAN_COM_MODE_NOMINAL:
            new_mode = CDH_MODE_NOMINAL;
            
            // Missão vem em data[0]
            if (data[0] == 1 || data[0] == 2) {
                cdh_status.mission_type = (MissionType_t)data[0];
                cdh_status.mode_active = 1;
                
                // Se for Missão 2 e tiver dados AIS em data[1..7]
                if (data[0] == 2) {
                    // Copia primeiros dados AIS que vieram junto
                    memcpy(ais_buffer.data, &data[1], 7);  // 7 bytes de AIS (data[1] a data[7])
                }
            } else {
                // Missão inválida
                success = 0;
                new_mode = cdh_status.current_mode;  // Mantém modo atual
            }
            break;
            
        case CAN_COM_MODE_ADCS:
            new_mode = CDH_MODE_ADCS;
            cdh_status.mode_active = 1;
            break;
            
        case CAN_COM_MODE_DETUMBLING:
            new_mode = CDH_MODE_DETUMBLING;
            cdh_status.mode_active = 1;
            break;
            
        case CAN_COM_MODE_EXIT:
            // Encerra a rotina atual
            cdh_status.mode_active = 0;
            new_mode = CDH_MODE_IDLE;
            cdh_status.mission_type = MISSION_NONE;
            break;
            
        default:
            success = 0;
            new_mode = cdh_status.current_mode;  // Mantém modo atual
            break;
    }
    
    if (success) {
        cdh_status.current_mode = new_mode;
    }
    
    // Envia status atualizado
    CAN_Protocol_SendCDHStatus();
}

/* ============================================================================
   HANDLER DE DADOS AIS (MISSÃO 2)
   ============================================================================ */
void CAN_HandleAISData(uint8_t *data)
{
    // Só processa se estiver em Missão 2
    if (cdh_status.mission_type != MISSION_2) {
    	memcpy(ais_buffer.data, data, 8);
        return;
    }
}

/* ============================================================================
   HANDLER DE TELEMETRIA EPS
   ============================================================================ */
void CAN_HandleEPSTelemetry(uint32_t msg_id, uint8_t *data)
{
    switch (msg_id) {
        case CAN_EPS_BATTERY:
            // Formato: [Cell0_V_H, Cell0_V_L, Cell1_V_H, Cell1_V_L, 
            //           Cell0_DoD_H, Cell0_DoD_L, Cell1_DoD_H, Cell1_DoD_L]
            eps_telemetry.cell_voltage[0] = (data[0] << 8) | data[1];
            eps_telemetry.cell_voltage[1] = (data[2] << 8) | data[3];
            eps_telemetry.cell_depth_discharge[0] = (data[4] << 8) | data[5];
            eps_telemetry.cell_depth_discharge[1] = (data[6] << 8) | data[7];
            
            break;

            
        case CAN_EPS_SOLAR_PANEL_VOLTAGE:
            // Formato: [V_1_2_B3, V_1_2_B2, V_1_2_B1, V_1_2_B0,
            //           V_3_4_B3, V_3_4_B2, V_3_4_B1, V_3_4_B0]
            eps_telemetry.solar_voltage_1_2 = ((uint32_t)data[0] << 24) | 
                                               ((uint32_t)data[1] << 16) |
                                               ((uint32_t)data[2] << 8)  | 
                                                (uint32_t)data[3];
            eps_telemetry.solar_voltage_3_4 = ((uint32_t)data[4] << 24) | 
                                               ((uint32_t)data[5] << 16) |
                                               ((uint32_t)data[6] << 8)  | 
                                                (uint32_t)data[7];
            break;
            
        case CAN_EPS_SOLAR_PANEL_CURRENT:
            // Formato: [I_1_2_B3, I_1_2_B2, I_1_2_B1, I_1_2_B0,
            //           I_3_4_B3, I_3_4_B2, I_3_4_B1, I_3_4_B0]
            eps_telemetry.solar_current_1_2 = ((uint32_t)data[0] << 24) | 
                                               ((uint32_t)data[1] << 16) |
                                               ((uint32_t)data[2] << 8)  | 
                                                (uint32_t)data[3];
            eps_telemetry.solar_current_3_4 = ((uint32_t)data[4] << 24) | 
                                               ((uint32_t)data[5] << 16) |
                                               ((uint32_t)data[6] << 8)  | 
                                                (uint32_t)data[7];
            break;
            
        default:
            // Telemetria EPS desconhecida
            break;
    }
}

/* ============================================================================
   ENVIO DE STATUS CDH
   ============================================================================ */
void CAN_Protocol_SendCDHStatus(void)
{
    CAN_Message_t status_msg = {
        .id = CAN_CDH_STATUS,
        .data = {
            cdh_status.current_mode,     // [0] Modo atual
            cdh_status.mission_type,     // [1] Tipo de missão
            cdh_status.mode_active,      // [2] Flag de atividade
            0, 0, 0, 0, 0
        }
    };
    
    CAN_Transmit(&status_msg);
}

/* ============================================================================
   GETTERS - CDH STATUS
   ============================================================================ */
CDH_OperationMode_t CAN_GetCurrentMode(void)
{
    return cdh_status.current_mode;
}

MissionType_t CAN_GetMissionType(void)
{
    return cdh_status.mission_type;
}

uint8_t CAN_IsModeActive(void)
{
    return cdh_status.mode_active;
}

/* ============================================================================
   GETTERS - EPS TELEMETRY
   ============================================================================ */
uint16_t CAN_GetCellVoltage(uint8_t cell_index)
{
    if (cell_index > 1) return 0;
    return eps_telemetry.cell_voltage[cell_index];
}

uint16_t CAN_GetCellDepthDischarge(uint8_t cell_index)
{
    if (cell_index > 1) return 0;
    return eps_telemetry.cell_depth_discharge[cell_index];
}

uint32_t CAN_GetSolarVoltage_1_2(void)
{
    return eps_telemetry.solar_voltage_1_2;
}

uint32_t CAN_GetSolarVoltage_3_4(void)
{
    return eps_telemetry.solar_voltage_3_4;
}

uint32_t CAN_GetSolarCurrent_1_2(void)
{
    return eps_telemetry.solar_current_1_2;
}

uint32_t CAN_GetSolarCurrent_3_4(void)
{
    return eps_telemetry.solar_current_3_4;
}
