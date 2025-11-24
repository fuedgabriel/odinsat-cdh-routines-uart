/*
 * SolarTracker.c
 *
 * Created on: Nov 24, 2025
 * Author: Fued Gabriel
 */

#include "SolarTracker.h"

// Inicializa calibração e variáveis
void Solar_Init(SolarTracker_t *tracker) {
    // Zera toda a estrutura
    memset(tracker, 0, sizeof(SolarTracker_t));

    // Define calibração padrão (pode ser ajustada em runtime se necessário)
    for(int i = 0; i < NUM_SENSORES; i++) {
        tracker->calibracao[i].min_val = 100;
        tracker->calibracao[i].max_val = 65000;
    }
    strcpy(tracker->debug_quadrante, "--");
}

// Função utilitária interna
uint16_t Solar_Normalize(uint16_t raw, uint16_t min, uint16_t max) {
    if (raw <= min) return 0;
    if (raw >= max) return 100;
    return (uint16_t)( ((uint32_t)(raw - min) * 100) / (max - min) );
}

// Lógica principal
void Solar_Process(SolarTracker_t *tracker) {

    // 1. Normalização
    for(int i = 0; i < NUM_SENSORES; i++) {
        tracker->adc_norm[i] = Solar_Normalize(tracker->adc_raw[i],
                                               tracker->calibracao[i].min_val,
                                               tracker->calibracao[i].max_val);
    }

    // 2. Telemetria: Cálculo do Ângulo Solar
    float sum_frente = (float)(tracker->adc_norm[S_X_PLUS_LEFT] + tracker->adc_norm[S_X_PLUS_RIGHT]);
    float sum_tras   = (float)(tracker->adc_norm[S_X_MINUS_LEFT] + tracker->adc_norm[S_X_MINUS_RIGHT]);
    float sum_dir    = (float)(tracker->adc_norm[S_Y_MINUS_LEFT] + tracker->adc_norm[S_Y_MINUS_RIGHT]);
    float sum_esq    = (float)(tracker->adc_norm[S_Y_PLUS_LEFT] + tracker->adc_norm[S_Y_PLUS_RIGHT]);

    float vect_x = sum_frente - sum_tras;
    float vect_y = sum_esq - sum_dir;

    float ang_rad = atan2f(vect_y, vect_x);
    tracker->angulo_solar_graus = ang_rad * (180.0f / 3.141592f);
    if (tracker->angulo_solar_graus < 0) tracker->angulo_solar_graus += 360.0f;

    // 3. Lógica de Rastreio (Quina/Diagonal)

    // Somas das quinas
    uint32_t q_FR = tracker->adc_norm[S_X_PLUS_RIGHT] + tracker->adc_norm[S_Y_MINUS_LEFT];
    uint32_t q_FL = tracker->adc_norm[S_X_PLUS_LEFT]  + tracker->adc_norm[S_Y_PLUS_RIGHT];
    uint32_t q_BR = tracker->adc_norm[S_X_MINUS_LEFT] + tracker->adc_norm[S_Y_MINUS_RIGHT];
    uint32_t q_BL = tracker->adc_norm[S_X_MINUS_RIGHT]+ tracker->adc_norm[S_Y_PLUS_LEFT];

    // Determina quadrante vencedor
    uint32_t max_luz = q_FR;
    int quadrante = 1;

    if (q_FL > max_luz) { max_luz = q_FL; quadrante = 2; }
    if (q_BR > max_luz) { max_luz = q_BR; quadrante = 3; }
    if (q_BL > max_luz) { max_luz = q_BL; quadrante = 4; }

    tracker->acao_motor = 0;

    // Controle Fino
    switch(quadrante) {
        case 1: // FR
            strcpy(tracker->debug_quadrante, "FR");
            tracker->erro_tracker = (int32_t)tracker->adc_norm[S_X_PLUS_RIGHT] - (int32_t)tracker->adc_norm[S_Y_MINUS_LEFT];
            if (tracker->erro_tracker > HISTERESE_FINA) tracker->acao_motor = -1;
            else if (tracker->erro_tracker < -HISTERESE_FINA) tracker->acao_motor = 1;
            break;

        case 2: // FL
            strcpy(tracker->debug_quadrante, "FL");
            tracker->erro_tracker = (int32_t)tracker->adc_norm[S_X_PLUS_LEFT] - (int32_t)tracker->adc_norm[S_Y_PLUS_RIGHT];
            if (tracker->erro_tracker > HISTERESE_FINA) tracker->acao_motor = 1;
            else if (tracker->erro_tracker < -HISTERESE_FINA) tracker->acao_motor = -1;
            break;

        case 3: // BR
            strcpy(tracker->debug_quadrante, "BR");
            tracker->erro_tracker = (int32_t)tracker->adc_norm[S_X_MINUS_LEFT] - (int32_t)tracker->adc_norm[S_Y_MINUS_RIGHT];
            if (tracker->erro_tracker > HISTERESE_FINA) tracker->acao_motor = 1;
            else if (tracker->erro_tracker < -HISTERESE_FINA) tracker->acao_motor = -1;
            break;

        case 4: // BL
            strcpy(tracker->debug_quadrante, "BL");
            tracker->erro_tracker = (int32_t)tracker->adc_norm[S_X_MINUS_RIGHT] - (int32_t)tracker->adc_norm[S_Y_PLUS_LEFT];
            if (tracker->erro_tracker > HISTERESE_FINA) tracker->acao_motor = -1;
            else if (tracker->erro_tracker < -HISTERESE_FINA) tracker->acao_motor = 1;
            break;
    }
}
