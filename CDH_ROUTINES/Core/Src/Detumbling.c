/*
 * Detumbling.c
 *
 * Created on: Nov 27, 2025
 * Author: Fued Gabriel
 */

#include "Detumbling.h"
#include <math.h>

void Detumble_Init(Detumble_t *detumble, float kd_gain) {
    detumble->Kd = kd_gain;
    detumble->Deadzone = 1.0f; // Ignora variações menores que 1% (ruído)
    detumble->prev_light_diff = 0.0f;
    detumble->output_torque = 0.0f;
    detumble->is_stable = 0;
}

void Detumble_Process(Detumble_t *detumble, SolarTracker_t *solar_data) {
    
    // 1. Obter a diferença de luz atual (Esquerda vs Direita)
    // Reutilizamos o cálculo já feito no SolarTracker para consistência
    // erro_luz_direcional = (Luz Direita) - (Luz Esquerda)
    float current_diff = solar_data->erro_luz_direcional;

    // 2. Calcular a Taxa de Variação (Derivada)
    // Se rate > 0: A luz está indo para a direita (Sol "passando" rápido)
    // Se rate < 0: A luz está indo para a esquerda
    float rate_of_change = current_diff - detumble->prev_light_diff;

    // 3. Atualizar histórico
    detumble->prev_light_diff = current_diff;

    // 4. Lógica de Controle (Freio Magnético/Inercial Simulado)
    // O torque deve ser OPOSTO à taxa de variação.
    // Se a variação é alta, aplicamos força contrária para frear.
    float torque_calc = -1.0f * detumble->Kd * rate_of_change;

    // 5. Verificação de Estabilidade (Zona Morta)
    if (fabs(rate_of_change) < detumble->Deadzone) {
        // Se a mudança for muito pequena, consideramos estável
        torque_calc = 0.0f;
        detumble->is_stable = 1;
    } else {
        detumble->is_stable = 0;
    }

    // 6. Saturação (Clamp) -100 a +100
    if (torque_calc > DETUMBLE_MAX_TORQUE) torque_calc = DETUMBLE_MAX_TORQUE;
    if (torque_calc < DETUMBLE_MIN_TORQUE) torque_calc = DETUMBLE_MIN_TORQUE;

    // Salva na estrutura
    detumble->output_torque = torque_calc;
}