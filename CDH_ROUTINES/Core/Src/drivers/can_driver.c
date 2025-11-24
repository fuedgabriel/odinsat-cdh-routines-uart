/**
  ******************************************************************************
  * @file    can_driver.c
  * @brief   CAN driver implementation
  ******************************************************************************
  */

#include "can_driver.h"
#include "main.h"

/* Private variables */
static FDCAN_RxHeaderTypeDef RxHeader;
static volatile uint8_t RxBuffer[8];  // ← volatile para evitar otimização
static volatile uint8_t msg_received = 0;

/* CAN Initialization */
void CAN_Init(void)
{
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
        Error_Handler();
    }
    
    if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
        Error_Handler();
    }
}

/* CAN Transmit - Always sends 8 bytes */
void CAN_Transmit(CAN_Message_t *msg)
{
    FDCAN_TxHeaderTypeDef TxHeader;
    
    TxHeader.Identifier = msg->id;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;  // Sempre 8 bytes
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;
    
    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, msg->data) != HAL_OK) {
        Error_Handler();
    }
}

/* Get received message - Always 8 bytes */
uint8_t CAN_GetMessage(CAN_Message_t *msg)
{
    if (!msg_received) {
        return 0;
    }
    
    // Desabilita interrupções durante cópia (proteção de seção crítica)
    __disable_irq();
    
    // Copia o ID recebido
    msg->id = RxHeader.Identifier;
    
    // Copia todos os 8 bytes
    for (uint8_t i = 0; i < 8; i++) {
        msg->data[i] = RxBuffer[i];
    }
    
    // Limpa flag ANTES de retornar (importante!)
    msg_received = 0;
    
    __enable_irq();
    
    return 1;
}

/* CAN RX Callback */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET) {
        // Lê mensagem diretamente para buffer temporário
        uint8_t tempBuffer[8];
        
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, tempBuffer) == HAL_OK) {
            // Copia para buffer volátil
            for (uint8_t i = 0; i < 8; i++) {
                RxBuffer[i] = tempBuffer[i];
            }
            
            // Seta flag por último
            msg_received = 1;
        }
    }
}
