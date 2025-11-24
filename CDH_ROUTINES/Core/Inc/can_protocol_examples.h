/**
  ******************************************************************************
  * @file    can_protocol_examples.h
  * @brief   Exemplos de uso do protocolo CAN do CubeSat
  ******************************************************************************
  */

#ifndef __CAN_PROTOCOL_EXAMPLES_H
#define __CAN_PROTOCOL_EXAMPLES_H

#include "can_protocol.h"

/* ============================================================================
   EXEMPLO 1: COM ENVIANDO COMANDO PARA ENTRAR EM MODO NOMINAL (MISSÃO 1)
   ============================================================================ */
/*
   No subsistema COM, enviar APENAS UM COMANDO:
   
   ID: 0x301 (CAN_COM_MODE_NOMINAL)
   Data: [1, 0, 0, 0, 0, 0, 0, 0]  // data[0] = 1 (MISSION_1)
   
   CDH responderá:
      ID: 0x101 (CAN_CDH_STATUS)
      Data: [1, 1, 1, 0, 0, 0, 0, 0]  // Mode=NOMINAL, Mission=1, Active=1
*/

/* ============================================================================
   EXEMPLO 2: COM ENVIANDO MODO NOMINAL COM MISSÃO 2 + DADOS AIS INICIAIS
   ============================================================================ */
/*
   No subsistema COM, enviar UM comando com tudo:
   
   ID: 0x301 (CAN_COM_MODE_NOMINAL)
   Data: [2, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11]
         ↑   └─────────────────────────────────────┘
      Missão 2     7 bytes de dados AIS iniciais
   
   CDH responderá:
      ID: 0x101 (CAN_CDH_STATUS) - Status: NOMINAL, Mission=2, Active=1
   
   Para enviar mais dados AIS:
      ID: 0x320 (CAN_COM_AIS_DATA)
      Data: [0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99]  // 8 bytes completos
*/

/* ============================================================================
   EXEMPLO 3: COM SAINDO DO MODO ATUAL
   ============================================================================ */
/*
   Para encerrar a rotina atual:
   
   ID: 0x30F (CAN_COM_MODE_EXIT)
   Data: [0, 0, 0, 0, 0, 0, 0, 0]
   
   CDH voltará para modo IDLE:
      ID: 0x101 (CAN_CDH_STATUS)
      Data: [0, 0, 0, 0, 0, 0, 0, 0]  // Mode=IDLE, Mission=NONE, Active=0
*/


/* ============================================================================
   TABELA DE RESUMO DE IDs
   ============================================================================ */
/*
   ┌──────────┬─────────────────────────────────────┬─────────────┐
   │    ID    │           DESCRIÇÃO                 │   ORIGEM    │
   ├──────────┼─────────────────────────────────────┼─────────────┤
   │  0x100   │  CDH - Telemetria geral             │     CDH     │
   │  0x101   │  CDH - Status atual                 │     CDH     │
   │  0x103   │  CDH - Erro reportado               │     CDH     │
   ├──────────┼─────────────────────────────────────┼─────────────┤
   │  0x200   │  EPS - Telemetria geral             │     EPS     │
   │  0x201   │  EPS - Dados de Bateria             │     EPS     │
   │  0x202   │  EPS - Voltagem dos Painéis solares │     EPS     │
   │  0x203   │  EPS - Corrente Painéis solares     │     EPS     │
   ├──────────┼─────────────────────────────────────┼─────────────┤
   │  0x300   │  COM - Entrar em modo IDLE          │     COM     │
   │  0x301   │  COM - Entrar em modo NOMINAL (data[0]=missão) │     COM     │
   │          │        data[1..7] = dados AIS se missão = 2    │             │
   │  0x302   │  COM - Entrar em modo ADCS                     │     COM     │
   │  0x303   │  COM - Entrar em modo DETUMBLING               │     COM     │
   │  0x30F   │  COM - Sair do modo atual                      │     COM     │
   │  0x320   │  COM - Dados AIS adicionais (8 bytes)          │     COM     │
   └──────────┴─────────────────────────────────────┴─────────────┘
*/

#endif /* __CAN_PROTOCOL_EXAMPLES_H */
