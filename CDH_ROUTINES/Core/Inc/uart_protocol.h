/**
  ******************************************************************************
  * @file    payload_protocol.h
  * @brief   Payload protocol abstraction layer
  ******************************************************************************
  */

#ifndef __UART_PROTOCOL_H
#define __UART_PROTOCOL_H

#include "usart.h"
#include <stdint.h>

#define UART_START_BYTE 0xFE
#define UART_MAX_PAYLOAD 256

/*
FORMATO: Start(1) + ID(1) + Len(2) + Data(N) + Checksum(1)
*/

/* IDs das Mensagens */ 
typedef enum {
    MSG_CMD_START_M1    = 0x01, // CDH -> Payload: Iniciar Missão 1
    MSG_CMD_START_M2    = 0x02, // CDH -> Payload: Iniciar Missão 2
    MSG_DATA_AIS        = 0x03, // CDH -> Payload: Enviar dados AIS do barco (Telemetria)
    MSG_RES_M1_OIL      = 0x10, // Payload -> CDH: Resultado Óleo (% área)
    MSG_RES_M2_SHIP     = 0x11, // Payload -> CDH: ID do Barco encontrado + Local de origem
    MSG_ACK             = 0xA0, // Confirmação de recebimento
    MSG_ERROR           = 0xEE  // Erro no processamento
} MsgID_t;

/* UART Message Structure  */
typedef struct {
    uint8_t id;
    uint8_t data[UART_MAX_PAYLOAD];
    uint8_t length;
} UART_Message_t;

/* Public Functions */
// Funções básicas de comunicação
void UART_Transmit(UART_HandleTypeDef *huart, UART_Message_t *msg, uint16_t size);
UART_Message_t UART_Receive(UART_HandleTypeDef *huart);
uint8_t CalculateChecksum(uint8_t msg_id, uint16_t length, uint8_t *data);
void Check_Payload_Response(void);

// Funções de controle de missão
void UART_StartMission1(void);
void UART_StartMission2(void);
void UART_SendAISData(uint8_t *ais_data);
void UART_ProcessMission(void);

// Getters para resultados das missões
uint8_t UART_GetOilDetected(void);
float UART_GetOilAreaPercentage(void);
uint32_t UART_GetShipMMSI(void);
float UART_GetShipOriginLat(void);
float UART_GetShipOriginLon(void);
uint8_t UART_IsMissionComplete(void);

#endif /* __UART_PROTOCOL_H */
