/**
  ******************************************************************************
  * @file    can_eps_examples.h
  * @brief   Exemplos de uso da telemetria EPS via CAN
  ******************************************************************************
  */

#ifndef __CAN_EPS_EXAMPLES_H
#define __CAN_EPS_EXAMPLES_H

#include "can_protocol.h"
#include "can_driver.h"

/* ============================================================================
   FORMATO DOS PACOTES EPS
   ============================================================================ */

/*
   PACOTE 1: CAN_EPS_BATTERY (0x201)
   ─────────────────────────────────────────────────────────────────
   8 bytes: Cell Voltage + Depth of Discharge
   
   Byte 0-1: Cell 0 Voltage (uint16_t, big-endian)
   Byte 2-3: Cell 1 Voltage (uint16_t, big-endian)
   Byte 4-5: Cell 0 Depth of Discharge (uint16_t, big-endian)
   Byte 6-7: Cell 1 Depth of Discharge (uint16_t, big-endian)
   
   PACOTE 2: CAN_EPS_SOLAR_PANEL_VOLTAGE (0x202)
   ─────────────────────────────────────────────────────────────────
   8 bytes: Tensão dos painéis solares
   
   Byte 0-3: Solar Input Voltage 1-2 (uint32_t, big-endian)
   Byte 4-7: Solar Input Voltage 3-4 (uint32_t, big-endian)
   
   PACOTE 3: CAN_EPS_SOLAR_PANEL_CURRENT (0x203)
   ─────────────────────────────────────────────────────────────────
   8 bytes: Corrente dos painéis solares
   
   Byte 0-3: Solar Input Current 1-2 (uint32_t, big-endian)
   Byte 4-7: Solar Input Current 3-4 (uint32_t, big-endian)
*/

/* ============================================================================
   EXEMPLO 1: EPS ENVIANDO TELEMETRIA DE BATERIA
   ============================================================================ */
#ifdef SUBSYSTEM_EPS

static inline void EPS_SendBatteryTelemetry(void)
{
    CAN_Message_t msg;
    
    // Valores de exemplo
    uint16_t cell_0_voltage = 3700;  // 3.7V
    uint16_t cell_1_voltage = 3650;  // 3.65V
    uint16_t cell_0_dod = 25;        // 25% DoD
    uint16_t cell_1_dod = 30;        // 30% DoD
    
    msg.id = CAN_EPS_BATTERY;
    
    // Cell 0 Voltage (bytes 0-1)
    msg.data[0] = (cell_0_voltage >> 8) & 0xFF;
    msg.data[1] = cell_0_voltage & 0xFF;
    
    // Cell 1 Voltage (bytes 2-3)
    msg.data[2] = (cell_1_voltage >> 8) & 0xFF;
    msg.data[3] = cell_1_voltage & 0xFF;
    
    // Cell 0 DoD (bytes 4-5)
    msg.data[4] = (cell_0_dod >> 8) & 0xFF;
    msg.data[5] = cell_0_dod & 0xFF;
    
    // Cell 1 DoD (bytes 6-7)
    msg.data[6] = (cell_1_dod >> 8) & 0xFF;
    msg.data[7] = cell_1_dod & 0xFF;
    
    CAN_Transmit(&msg);
}

static inline void EPS_SendSolarPanelVoltage(void)
{
    CAN_Message_t msg;
    
    // Valores de exemplo (em mV)
    uint32_t voltage_1_2 = 5000000;  // 5V
    uint32_t voltage_3_4 = 4980000;  // 4.98V
    
    msg.id = CAN_EPS_SOLAR_PANEL_VOLTAGE;
    
    // Voltage 1-2 (bytes 0-3)
    msg.data[0] = (voltage_1_2 >> 24) & 0xFF;
    msg.data[1] = (voltage_1_2 >> 16) & 0xFF;
    msg.data[2] = (voltage_1_2 >> 8) & 0xFF;
    msg.data[3] = voltage_1_2 & 0xFF;
    
    // Voltage 3-4 (bytes 4-7)
    msg.data[4] = (voltage_3_4 >> 24) & 0xFF;
    msg.data[5] = (voltage_3_4 >> 16) & 0xFF;
    msg.data[6] = (voltage_3_4 >> 8) & 0xFF;
    msg.data[7] = voltage_3_4 & 0xFF;
    
    CAN_Transmit(&msg);
}

static inline void EPS_SendSolarPanelCurrent(void)
{
    CAN_Message_t msg;
    
    // Valores de exemplo (em uA)
    uint32_t current_1_2 = 2500000;  // 2.5A
    uint32_t current_3_4 = 2450000;  // 2.45A
    
    msg.id = CAN_EPS_SOLAR_PANEL_CURRENT;
    
    // Current 1-2 (bytes 0-3)
    msg.data[0] = (current_1_2 >> 24) & 0xFF;
    msg.data[1] = (current_1_2 >> 16) & 0xFF;
    msg.data[2] = (current_1_2 >> 8) & 0xFF;
    msg.data[3] = current_1_2 & 0xFF;
    
    // Current 3-4 (bytes 4-7)
    msg.data[4] = (current_3_4 >> 24) & 0xFF;
    msg.data[5] = (current_3_4 >> 16) & 0xFF;
    msg.data[6] = (current_3_4 >> 8) & 0xFF;
    msg.data[7] = current_3_4 & 0xFF;
    
    CAN_Transmit(&msg);
}

// Enviar telemetria completa
static inline void EPS_SendCompleteTelemetry(void)
{
    EPS_SendBatteryTelemetry();
    HAL_Delay(5);  // Pequeno delay entre mensagens
    
    EPS_SendSolarPanelVoltage();
    HAL_Delay(5);
    
    EPS_SendSolarPanelCurrent();
}

#endif // SUBSYSTEM_EPS

/* ============================================================================
   EXEMPLO 2: CDH LENDO TELEMETRIA EPS
   ============================================================================ */
#ifdef SUBSYSTEM_CDH

static inline void CDH_PrintEPSTelemetry(void)
{
    // Ler dados de bateria
    uint16_t cell0_v = CAN_GetCellVoltage(0);
    uint16_t cell1_v = CAN_GetCellVoltage(1);
    uint16_t cell0_dod = CAN_GetCellDepthDischarge(0);
    uint16_t cell1_dod = CAN_GetCellDepthDischarge(1);
    
    // Ler dados de painéis solares
    uint32_t solar_v_1_2 = CAN_GetSolarVoltage_1_2();
    uint32_t solar_v_3_4 = CAN_GetSolarVoltage_3_4();
    uint32_t solar_i_1_2 = CAN_GetSolarCurrent_1_2();
    uint32_t solar_i_3_4 = CAN_GetSolarCurrent_3_4();
    
    // Exemplo de uso (com printf ou log)
    // printf("Battery Cell 0: %dmV, DoD: %d%%\n", cell0_v, cell0_dod);
    // printf("Battery Cell 1: %dmV, DoD: %d%%\n", cell1_v, cell1_dod);
    // printf("Solar Voltage 1-2: %duV\n", solar_v_1_2);
    // printf("Solar Current 1-2: %duA\n", solar_i_1_2);
    
    // Exemplo de verificação
    if (cell0_dod > 80 || cell1_dod > 80) {
        // Bateria crítica!
        // Entrar em modo seguro
    }
}

static inline void CDH_MonitorPowerStatus(void)
{
    // Obter dados das células
    uint16_t cell_voltages[2];
    cell_voltages[0] = CAN_GetCellVoltage(0);
    cell_voltages[1] = CAN_GetCellVoltage(1);
    
    // Encontrar menor tensão
    uint16_t min_voltage = (cell_voltages[0] < cell_voltages[1]) ? 
                           cell_voltages[0] : cell_voltages[1];
    
    // Critérios de segurança
    if (min_voltage < 3300) {
        // Tensão crítica: < 3.3V
        // Ação: desligar cargas não essenciais
    }
    else if (min_voltage < 3500) {
        // Tensão baixa: < 3.5V
        // Ação: modo economia de energia
    }
    
    // Verificar painéis solares
    uint32_t total_solar_current = CAN_GetSolarCurrent_1_2() + 
                                    CAN_GetSolarCurrent_3_4();
    
    if (total_solar_current < 500000) {  // < 500mA total
        // Pouca geração solar (eclipse ou problema)
        // Ação: reduzir consumo
    }
}

#endif // SUBSYSTEM_CDH

/* ============================================================================
   TABELA RESUMO - TELEMETRIA EPS
   ============================================================================ */
/*
   ┌──────────┬───────────────────────────┬──────────────────────────────┐
   │    ID    │        DESCRIÇÃO          │         FORMATO              │
   ├──────────┼───────────────────────────┼──────────────────────────────┤
   │  0x201   │  Battery Cells            │  [V0_H, V0_L, V1_H, V1_L,    │
   │          │  Voltage + DoD            │   D0_H, D0_L, D1_H, D1_L]    │
   ├──────────┼───────────────────────────┼──────────────────────────────┤
   │  0x202   │  Solar Panel Voltage      │  [V12_B3, V12_B2, V12_B1,    │
   │          │  (Painéis 1-2 e 3-4)      │   V12_B0, V34_B3, V34_B2,    │
   │          │                           │   V34_B1, V34_B0]            │
   ├──────────┼───────────────────────────┼──────────────────────────────┤
   │  0x203   │  Solar Panel Current      │  [I12_B3, I12_B2, I12_B1,    │
   │          │  (Painéis 1-2 e 3-4)      │   I12_B0, I34_B3, I34_B2,    │
   │          │                           │   I34_B1, I34_B0]            │
   └──────────┴───────────────────────────┴──────────────────────────────┘
*/

#endif /* __CAN_EPS_EXAMPLES_H */
