
#include "antena.h"
#include "main.h"
#include "gpio.h"

void Deploy_Antenna(void) {

    // 1. LIGA O MOSFET (Envia 3.3V para o Gate)
    HAL_GPIO_WritePin(ANT_DEPLOYER_IO_2_GPIO_Port, ANT_DEPLOYER_IO_2_Pin, GPIO_PIN_SET); 
    HAL_GPIO_WritePin(ANT_DEPLOYER_IO_3_GPIO_Port, ANT_DEPLOYER_IO_3_Pin, GPIO_PIN_SET); 

    // 2. ESPERA O TEMPO DE QUEIMA 
    HAL_Delay(3000); 

    // 3. DESLIGA O MOSFET (SAFETY)
    HAL_GPIO_WritePin(ANT_DEPLOYER_IO_2_GPIO_Port, ANT_DEPLOYER_IO_2_Pin, GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(ANT_DEPLOYER_IO_3_GPIO_Port, ANT_DEPLOYER_IO_3_Pin, GPIO_PIN_RESET); 
}