/**
  ******************************************************************************
  * @file    uart_protocol.c
  * @brief   UART protocol implementation
  ******************************************************************************
  */

#include "uart_protocol.h"
#include "can_protocol.h"  // Para acessar missão atual
#include "main.h"
#include <string.h>

/* ============================================================================
   VARIÁVEIS PRIVADAS
   ============================================================================ */
static uint8_t rx_buffer[UART_MAX_PAYLOAD + 10];  // Buffer para recepção
static UART_Message_t last_received_msg = {0};
static volatile uint8_t msg_received_flag = 0;

// Armazenamento dos resultados das missões
typedef struct {
    uint8_t oil_detected;        // Missão 1: Há derramamento? (0=não, 1=sim)
    float oil_area_percentage;   // Missão 1: Área do óleo (%)
    uint32_t ship_mmsi;          // Missão 2: ID do barco (MMSI)
    float ship_origin_lat;       // Missão 2: Latitude origem
    float ship_origin_lon;       // Missão 2: Longitude origem
    uint8_t mission_complete;    // Flag indicando que missão terminou
} MissionResults_t;

static MissionResults_t mission_results = {0};

/* ============================================================================
   CÁLCULO DE CHECKSUM
   ============================================================================ */
/**
 * @brief Calcula checksum simples (XOR de todos os bytes)
 * @param msg_id ID da mensagem
 * @param length Tamanho do payload
 * @param data Ponteiro para os dados
 * @return Checksum calculado
 */
uint8_t CalculateChecksum(uint8_t msg_id, uint16_t length, uint8_t *data)
{
    uint8_t checksum = UART_START_BYTE;
    
    checksum ^= msg_id;
    checksum ^= (length >> 8) & 0xFF;   // High byte do length
    checksum ^= length & 0xFF;          // Low byte do length
    
    for (uint16_t i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    
    return checksum;
}

/* ============================================================================
   TRANSMISSÃO UART
   ============================================================================ */
/**
 * @brief Transmite mensagem via UART com protocolo
 * @param huart Handle da UART
 * @param msg Estrutura da mensagem a enviar
 * @param size Tamanho dos dados úteis
 */
void UART_Transmit(UART_HandleTypeDef *huart, UART_Message_t *msg, uint16_t size)
{
    uint8_t tx_buffer[UART_MAX_PAYLOAD + 10];
    uint16_t tx_index = 0;
    
    // Monta o frame: [START][ID][LEN_H][LEN_L][DATA...][CHECKSUM]
    tx_buffer[tx_index++] = UART_START_BYTE;
    tx_buffer[tx_index++] = msg->id;
    tx_buffer[tx_index++] = (size >> 8) & 0xFF;   // Length high byte
    tx_buffer[tx_index++] = size & 0xFF;          // Length low byte
    
    // Copia dados
    for (uint16_t i = 0; i < size; i++) {
        tx_buffer[tx_index++] = msg->data[i];
    }
    
    // Adiciona checksum
    uint8_t checksum = CalculateChecksum(msg->id, size, msg->data);
    tx_buffer[tx_index++] = checksum;
    
    // Transmite via UART
    HAL_UART_Transmit(huart, tx_buffer, tx_index, HAL_MAX_DELAY);
}

/* ============================================================================
   RECEPÇÃO UART
   ============================================================================ */
/**
 * @brief Recebe e valida mensagem UART
 * @param huart Handle da UART
 * @return Estrutura com mensagem recebida (id=0 se erro)
 */
UART_Message_t UART_Receive(UART_HandleTypeDef *huart)
{
    UART_Message_t msg = {0};
    uint8_t byte;
    uint16_t rx_index = 0;
    
    // 1. Aguarda START_BYTE
    if (HAL_UART_Receive(huart, &byte, 1, 1000) != HAL_OK) {
        return msg;  // Timeout ou erro
    }
    
    if (byte != UART_START_BYTE) {
        return msg;  // Byte inicial inválido
    }
    
    rx_buffer[rx_index++] = byte;
    
    // 2. Recebe ID
    if (HAL_UART_Receive(huart, &byte, 1, 100) != HAL_OK) return msg;
    msg.id = byte;
    rx_buffer[rx_index++] = byte;
    
    // 3. Recebe LENGTH (2 bytes)
    uint8_t len_h, len_l;
    if (HAL_UART_Receive(huart, &len_h, 1, 100) != HAL_OK) return msg;
    if (HAL_UART_Receive(huart, &len_l, 1, 100) != HAL_OK) return msg;
    
    uint16_t length = (len_h << 8) | len_l;
    rx_buffer[rx_index++] = len_h;
    rx_buffer[rx_index++] = len_l;
    
    // Validação de tamanho
    if (length > UART_MAX_PAYLOAD) {
        msg.id = 0;  // Erro
        return msg;
    }
    
    msg.length = length;
    
    // 4. Recebe DATA
    for (uint16_t i = 0; i < length; i++) {
        if (HAL_UART_Receive(huart, &byte, 1, 100) != HAL_OK) {
            msg.id = 0;
            return msg;
        }
        msg.data[i] = byte;
        rx_buffer[rx_index++] = byte;
    }
    
    // 5. Recebe CHECKSUM
    uint8_t received_checksum;
    if (HAL_UART_Receive(huart, &received_checksum, 1, 100) != HAL_OK) {
        msg.id = 0;
        return msg;
    }
    
    // 6. Valida CHECKSUM
    uint8_t calculated_checksum = CalculateChecksum(msg.id, length, msg.data);
    
    if (received_checksum != calculated_checksum) {
        msg.id = 0;  // Checksum inválido
        return msg;
    }
    
    return msg;
}

/* ============================================================================
   PROCESSAMENTO DE RESPOSTAS DO PAYLOAD
   ============================================================================ */
/**
 * @brief Processa respostas recebidas do Payload
 */
void Check_Payload_Response(void)
{
    UART_Message_t msg = UART_Receive(&huart5);
    
    if (msg.id == 0) {
        return;  // Nenhuma mensagem válida ou erro
    }
    
    switch (msg.id) {
        case MSG_RES_M1_OIL:
            // Resposta da Missão 1: Detecção de óleo
            // Formato: [oil_detected(1)] [area_percentage(4 bytes float)]
            if (msg.length >= 5) {
                mission_results.oil_detected = msg.data[0];
                
                // Extrai float (4 bytes, little-endian)
                uint32_t temp;
                memcpy(&temp, &msg.data[1], 4);
                memcpy(&mission_results.oil_area_percentage, &temp, 4);
                
                mission_results.mission_complete = 1;
            }
            break;
            
        case MSG_RES_M2_SHIP:
            // Resposta da Missão 2: Identificação do barco
            // Formato: [mmsi(4)] [lat(4 float)] [lon(4 float)]
            if (msg.length >= 12) {
                // MMSI (4 bytes)
                mission_results.ship_mmsi = ((uint32_t)msg.data[0] << 24) |
                                            ((uint32_t)msg.data[1] << 16) |
                                            ((uint32_t)msg.data[2] << 8)  |
                                             (uint32_t)msg.data[3];
                
                // Latitude (4 bytes float)
                uint32_t temp_lat;
                memcpy(&temp_lat, &msg.data[4], 4);
                memcpy(&mission_results.ship_origin_lat, &temp_lat, 4);
                
                // Longitude (4 bytes float)
                uint32_t temp_lon;
                memcpy(&temp_lon, &msg.data[8], 4);
                memcpy(&mission_results.ship_origin_lon, &temp_lon, 4);
                
                mission_results.mission_complete = 1;
            }
            break;
            
        case MSG_ACK:
            // Payload confirmou recebimento
            // TODO: Implementar lógica se necessário
            break;
            
        case MSG_ERROR:
            // Payload reportou erro
            // TODO: Implementar tratamento de erro
            mission_results.mission_complete = 0;
            break;
            
        default:
            // Mensagem desconhecida
            break;
    }
}

/* ============================================================================
   FUNÇÕES DE CONTROLE DE MISSÃO
   ============================================================================ */
/**
 * @brief Inicia Missão 1 enviando comando ao Payload
 */
void UART_StartMission1(void)
{
    UART_Message_t msg;
    msg.id = MSG_CMD_START_M1;
    msg.length = 0;  // Sem dados adicionais
    
    UART_Transmit(&huart5, &msg, 0);
    
    // Reseta resultados
    mission_results.mission_complete = 0;
    mission_results.oil_detected = 0;
    mission_results.oil_area_percentage = 0.0f;
}

/**
 * @brief Inicia Missão 2 enviando comando ao Payload
 */
void UART_StartMission2(void)
{
    UART_Message_t msg;
    msg.id = MSG_CMD_START_M2;
    msg.length = 0;  // Sem dados adicionais
    
    UART_Transmit(&huart5, &msg, 0);
    
    // Reseta resultados
    mission_results.mission_complete = 0;
    mission_results.ship_mmsi = 0;
    mission_results.ship_origin_lat = 0.0f;
    mission_results.ship_origin_lon = 0.0f;
}

/**
 * @brief Envia dados AIS para o Payload (Missão 2)
 * @param ais_data Ponteiro para dados AIS (8 bytes)
 */
void UART_SendAISData(uint8_t *ais_data)
{
    UART_Message_t msg;
    msg.id = MSG_DATA_AIS;
    msg.length = 8;  // 8 bytes de dados AIS
    
    memcpy(msg.data, ais_data, 8);
    
    UART_Transmit(&huart5, &msg, 8);
}

/* ============================================================================
   INTEGRAÇÃO COM CAN PROTOCOL
   ============================================================================ */
/**
 * @brief Processa missão baseado no estado do CAN
 *        Deve ser chamada no loop principal
 */
void UART_ProcessMission(void)
{
        MissionType_t mission = CAN_GetMissionType();
        
        static uint8_t mission_started = 0;
        
        switch (mission) {
            case MISSION_1:
                if (!mission_started) {
                    UART_StartMission1();
                    mission_started = 1;
                }
                
                // Verifica resposta do Payload
                Check_Payload_Response();
                
                // Se missão completou, pode processar resultado
                if (mission_results.mission_complete) {
                    // Enviar resultado via CAN para COM
                    CAN_Message_t payloadResponse;
                    payloadResponse.id = CAN_CDH_TELEMETRY;  // 0x100 - Telemetria geral
                    
                    // Formato: [mission_type(1)] [oil_detected(1)] [area_percentage(4 bytes)] [unused(2)]
                    payloadResponse.data[0] = MISSION_1;
                    payloadResponse.data[1] = mission_results.oil_detected;
                    
                    // Converte float para bytes (little-endian)
                    uint32_t area_temp;
                    memcpy(&area_temp, &mission_results.oil_area_percentage, 4);
                    payloadResponse.data[2] = (area_temp >> 24) & 0xFF;
                    payloadResponse.data[3] = (area_temp >> 16) & 0xFF;
                    payloadResponse.data[4] = (area_temp >> 8) & 0xFF;
                    payloadResponse.data[5] = area_temp & 0xFF;
                    payloadResponse.data[6] = 0;  // Unused
                    payloadResponse.data[7] = 0;  // Unused
                    
                    CAN_Transmit(&payloadResponse);
                    mission_started = 0;  // Permite nova execução
                    mission_results.mission_complete = 0;  // Reseta flag
                }
                break;
                
            case MISSION_2:
                if (!mission_started) {
                    UART_StartMission2();
                    mission_started = 1;
                }
                
                // Verifica resposta do Payload
                Check_Payload_Response();
                
                if (mission_results.mission_complete) {
                    // Enviar resultado via CAN - Mensagem 1: MMSI
                    CAN_Message_t payloadResponse;
                    payloadResponse.id = CAN_CDH_TELEMETRY;  // 0x100 - Telemetria geral
                    
                    // Formato MSG 1: [mission_type(1)] [packet_id(1)] [mmsi(4 bytes)] [unused(2)]
                    payloadResponse.data[0] = MISSION_2;
                    payloadResponse.data[1] = 0x01;  // Packet 1 - MMSI
                    payloadResponse.data[2] = (mission_results.ship_mmsi >> 24) & 0xFF;
                    payloadResponse.data[3] = (mission_results.ship_mmsi >> 16) & 0xFF;
                    payloadResponse.data[4] = (mission_results.ship_mmsi >> 8) & 0xFF;
                    payloadResponse.data[5] = mission_results.ship_mmsi & 0xFF;
                    payloadResponse.data[6] = 0;
                    payloadResponse.data[7] = 0;
                    
                    CAN_Transmit(&payloadResponse);
                    HAL_Delay(5);  // Pequeno delay entre mensagens
                    
                    // Mensagem 2: Latitude
                    payloadResponse.data[1] = 0x02;  // Packet 2 - Latitude
                    uint32_t lat_temp;
                    memcpy(&lat_temp, &mission_results.ship_origin_lat, 4);
                    payloadResponse.data[2] = (lat_temp >> 24) & 0xFF;
                    payloadResponse.data[3] = (lat_temp >> 16) & 0xFF;
                    payloadResponse.data[4] = (lat_temp >> 8) & 0xFF;
                    payloadResponse.data[5] = lat_temp & 0xFF;
                    payloadResponse.data[6] = 0;
                    payloadResponse.data[7] = 0;
                    
                    CAN_Transmit(&payloadResponse);
                    HAL_Delay(5);
                    
                    // Mensagem 3: Longitude
                    payloadResponse.data[1] = 0x03;  // Packet 3 - Longitude
                    uint32_t lon_temp;
                    memcpy(&lon_temp, &mission_results.ship_origin_lon, 4);
                    payloadResponse.data[2] = (lon_temp >> 24) & 0xFF;
                    payloadResponse.data[3] = (lon_temp >> 16) & 0xFF;
                    payloadResponse.data[4] = (lon_temp >> 8) & 0xFF;
                    payloadResponse.data[5] = lon_temp & 0xFF;
                    payloadResponse.data[6] = 0;
                    payloadResponse.data[7] = 0;
                    
                    CAN_Transmit(&payloadResponse);
                    
                    mission_started = 0;
                    mission_results.mission_complete = 0;  // Reseta flag
                }
                break;
                
            default:
                mission_started = 0;
                break;
        }
}

/* ============================================================================
   GETTERS PARA RESULTADOS DAS MISSÕES
   ============================================================================ */
/**
 * @brief Retorna se há óleo detectado (Missão 1)
 */
uint8_t UART_GetOilDetected(void)
{
    return mission_results.oil_detected;
}

/**
 * @brief Retorna percentual de área de óleo (Missão 1)
 */
float UART_GetOilAreaPercentage(void)
{
    return mission_results.oil_area_percentage;
}

/**
 * @brief Retorna MMSI do barco (Missão 2)
 */
uint32_t UART_GetShipMMSI(void)
{
    return mission_results.ship_mmsi;
}

/**
 * @brief Retorna latitude de origem do barco (Missão 2)
 */
float UART_GetShipOriginLat(void)
{
    return mission_results.ship_origin_lat;
}

/**
 * @brief Retorna longitude de origem do barco (Missão 2)
 */
float UART_GetShipOriginLon(void)
{
    return mission_results.ship_origin_lon;
}

/**
 * @brief Verifica se a missão foi completada
 */
uint8_t UART_IsMissionComplete(void)
{
    return mission_results.mission_complete;
}