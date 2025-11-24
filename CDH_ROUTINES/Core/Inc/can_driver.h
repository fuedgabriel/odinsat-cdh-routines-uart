/**
  ******************************************************************************
  * @file    can_driver.h
  * @brief   CAN driver abstraction layer
  ******************************************************************************
  */

#ifndef __CAN_DRIVER_H
#define __CAN_DRIVER_H

#include "fdcan.h"
#include <stdint.h>

/* CAN Message Structure - Fixed 8 bytes */
typedef struct {
    uint32_t id;
    uint8_t data[8];
} CAN_Message_t;

/* Public Functions */
void CAN_Init(void);
void CAN_Transmit(CAN_Message_t *msg);
uint8_t CAN_GetMessage(CAN_Message_t *msg);

#endif /* __CAN_DRIVER_H */
