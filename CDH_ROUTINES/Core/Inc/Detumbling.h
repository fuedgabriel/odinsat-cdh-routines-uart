/*
 * Detumbling.h
 *
 * Created on: Nov 27, 2025
 * Author: Fued Gabriel
 *
 * Descrição: Algoritmo de estabilização (Detumbling) baseado na taxa
 * de variação da luminosidade (LDRs). O objetivo é reduzir a velocidade
 * angular do satélite antes de iniciar o apontamento fino.
 */

#ifndef INC_DETUMBLING_H_
#define INC_DETUMBLING_H_

#include "SolarTracker.h" // Precisa acesso aos dados dos sensores

// --- Configurações ---
#define DETUMBLE_MAX_TORQUE 100.0f
#define DETUMBLE_MIN_TORQUE -100.0f

// Estrutura de Controle do Detumbling
typedef struct {
    // Parâmetro de Ajuste
    float Kd;            // Ganho Derivativo (Força contrária à rotação)
    float Deadzone;      // Zona morta (se a rotação for muito lenta, ignora)

    // Estado Interno
    float prev_light_diff; // Valor da diferença de luz anterior
    float output_torque;   // Saída calculada para a roda (-100 a +100)
    uint8_t is_stable;     // Flag: 1 se estiver estável, 0 se estiver girando
} Detumble_t;

// --- Protótipos ---

// Inicializa o sistema de Detumbling com o ganho desejado
void Detumble_Init(Detumble_t *detumble, float kd_gain);

// Processa os dados do SolarTracker para calcular o torque de frenagem
void Detumble_Process(Detumble_t *detumble, SolarTracker_t *solar_data);

#endif /* INC_DETUMBLING_H_ */