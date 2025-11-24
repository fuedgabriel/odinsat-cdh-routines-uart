/*
 * SolarTracker.h
 *
 * Created on: Nov 24, 2025
 * Author: Fued Gabriel
 */

#ifndef INC_SOLARTRACKER_H_
#define INC_SOLARTRACKER_H_

#include "main.h" // Para ter acesso aos uint16_t, etc.
#include <math.h>
#include <string.h>
#include <stdio.h>

// --- Configurações ---
#define NUM_SENSORES 8
#define HISTERESE_FINA 3

// --- Mapeamento dos Sensores (Índices) ---
#define S_Y_MINUS_RIGHT 0
#define S_Y_MINUS_LEFT  1
#define S_Y_PLUS_LEFT   2
#define S_X_PLUS_LEFT   3
#define S_X_PLUS_RIGHT  4
#define S_X_MINUS_LEFT  5
#define S_X_MINUS_RIGHT 6
#define S_Y_PLUS_RIGHT  7

// --- Estruturas de Dados ---

// Estrutura para calibração
typedef struct {
    uint16_t min_val;
    uint16_t max_val;
} LDR_Calib_t;

// Estrutura principal de controle (Contexto)
typedef struct {
    // Entradas
    uint16_t adc_raw[NUM_SENSORES];       // Valores brutos lidos do ADC

    // Processamento Interno
    uint16_t adc_norm[NUM_SENSORES];      // Valores normalizados (0-100%)
    LDR_Calib_t calibracao[NUM_SENSORES]; // Dados de calibração

    // Saídas (Telemetria e Controle)
    float angulo_solar_graus;             // Ângulo calculado 0-360
    char debug_quadrante[5];              // "FR", "FL", etc.
    int32_t erro_tracker;                 // Diferença de luz
    int acao_motor;                       // -1: Esq, 0: Parado, 1: Dir
} SolarTracker_t;

// --- Protótipos das Funções ---

// Inicializa a estrutura com valores padrão de calibração
void Solar_Init(SolarTracker_t *tracker);

// Processa os dados: Normaliza -> Calcula Ângulo -> Define Ação do Motor
void Solar_Process(SolarTracker_t *tracker);

// Função auxiliar para aplicar a calibração
uint16_t Solar_Normalize(uint16_t raw, uint16_t min, uint16_t max);

#endif /* INC_SOLARTRACKER_H_ */
