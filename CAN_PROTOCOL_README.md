# 🛰️ Protocolo CAN - OdinSat CubeSat

## 📋 Visão Geral

Sistema de comunicação CAN para o CubeSat OdinSat, permitindo comunicação entre os subsistemas **CDH** (Command & Data Handling), **EPS** (Electrical Power System) e **COM** (Communications).

## 🗺️ Mapa de Endereços

### Estrutura de IDs
```
0x100 - 0x1FF : CDH (Command & Data Handling)
0x200 - 0x2FF : EPS (Electrical Power System)  
0x300 - 0x3FF : COM (Communications)
```

### Tabela Completa de IDs

| ID    | Nome                    | Origem | Descrição                              |
|-------|-------------------------|--------|----------------------------------------|
| 0x100 | `CDH_TELEMETRY`        | CDH    | Telemetria geral do CDH                |
| 0x101 | `CDH_STATUS`           | CDH    | Status atual (modo, missão, atividade) |
| 0x102 | `CDH_ACK`              | CDH    | Confirmação de comando (ACK/NACK)      |
| 0x103 | `CDH_ERROR`            | CDH    | Erro reportado                         |
| 0x200 | `EPS_TELEMETRY`        | EPS    | Telemetria completa do EPS             |
| 0x201 | `EPS_BATTERY_V`        | EPS    | Tensão da bateria                      |
| 0x202 | `EPS_BATTERY_I`        | EPS    | Corrente da bateria                    |
| 0x203 | `EPS_SOLAR_PANEL`      | EPS    | Status dos painéis solares             |
| 0x204 | `EPS_POWER_STATUS`     | EPS    | Status geral de potência               |
| 0x300 | `COM_MODE_IDLE`        | COM    | Comando: Entrar em modo IDLE                    |
| 0x301 | `COM_MODE_NOMINAL`     | COM    | Comando: Modo NOMINAL (data[0]=missão 1 ou 2)  |
| 0x302 | `COM_MODE_ADCS`        | COM    | Comando: Entrar em modo ADCS                    |
| 0x303 | `COM_MODE_DETUMBLING`  | COM    | Comando: Entrar em modo DETUMBLING              |
| 0x30F | `COM_MODE_EXIT`        | COM    | Comando: Sair do modo atual                     |
| 0x320 | `COM_AIS_DATA`         | COM    | Dados AIS adicionais (8 bytes)                  |

## 🔄 Modos de Operação do CDH

```c
typedef enum {
    CDH_MODE_IDLE = 0,       // Modo ocioso (padrão)
    CDH_MODE_NOMINAL,        // Modo nominal (Missão 1 ou 2)
    CDH_MODE_ADCS,          // Modo ADCS
    CDH_MODE_DETUMBLING     // Modo destumbling
} CDH_OperationMode_t;
```

## 🎯 Missões

```c
typedef enum {
    MISSION_NONE = 0,
    MISSION_1 = 1,      // Missão 1
    MISSION_2 = 2       // Missão 2 (com dados AIS)
} MissionType_t;
```

## 📦 Formatos de Mensagem

### CDH Status (ID: 0x101)
```
Byte 0: Modo atual (0=IDLE, 1=NOMINAL, 2=ADCS, 3=DETUMBLING)
Byte 1: Tipo de missão (0=NONE, 1=MISSION_1, 2=MISSION_2)
Byte 2: Flag de atividade (0=inativo, 1=ativo)
Bytes 3-7: Reservados (0x00)
```

### CDH ACK (ID: 0x102)
```
Byte 0: ID do comando original (high byte)
Byte 1: ID do comando original (low byte)
Byte 2: Status (1=ACK, 0=NACK)
Bytes 3-7: Reservados (0x00)
```

### EPS Telemetry (ID: 0x200)
```
Byte 0-1: Tensão da bateria em mV (16 bits, big-endian)
Byte 2-3: Corrente da bateria em mA (16 bits signed, big-endian)
Byte 4:   State of Charge (%) (0-100)
Bytes 5-7: Reservados
```

### COM Mode NOMINAL (ID: 0x301) - OTIMIZADO
```
Byte 0:   Número da missão (1 ou 2)
Bytes 1-7: Se missão = 2, pode conter 7 bytes de dados AIS iniciais
           Se missão = 1, reservados (0x00)
```

### COM AIS Data Adicional (ID: 0x320)
```
Bytes 0-7: Dados brutos do pacote AIS
```

## 🚀 Exemplos de Uso

### Exemplo 1: COM enviando comando para Modo Nominal (Missão 1) - OTIMIZADO

```c
// Apenas UM comando para entrar em modo NOMINAL com Missão 1
CAN_Message_t msg = {
    .id = 0x301,  // CAN_COM_MODE_NOMINAL
    .data = {1, 0, 0, 0, 0, 0, 0, 0}  // data[0] = 1 (MISSION_1)
};
CAN_Transmit(&msg);

// CDH responde com ACK (0x102) e Status (0x101)
// Redução: 2 mensagens → 1 mensagem!
```

### Exemplo 2: COM entrando em Missão 2 com dados AIS iniciais - OTIMIZADO

```c
// UM comando com modo + missão + dados AIS iniciais
CAN_Message_t msg = {
    .id = 0x301,  // CAN_COM_MODE_NOMINAL
    .data = {
        2,      // data[0] = 2 (MISSION_2)
        0xAA,   // data[1-7] = primeiros 7 bytes de dados AIS
        0xBB,
        0xCC,
        0xDD,
        0xEE,
        0xFF,
        0x11
    }
};
CAN_Transmit(&msg);

// Se precisar enviar mais dados AIS:
msg.id = 0x320;  // CAN_COM_AIS_DATA
msg.data[0] = 0x22;
msg.data[1] = 0x33;
// ... 8 bytes completos de dados AIS adicionais
CAN_Transmit(&msg);

// Redução: 3 mensagens → 1 ou 2 mensagens!
```

### Exemplo 3: COM encerrando modo atual

```c
CAN_Message_t msg = {
    .id = 0x30F,  // CAN_COM_MODE_EXIT
    .data = {0, 0, 0, 0, 0, 0, 0, 0}
};
CAN_Transmit(&msg);

// CDH volta para IDLE e envia status atualizado
```

### Exemplo 4: EPS enviando telemetria

```c
uint16_t voltage = 3800;  // 3800mV
int16_t current = 200;    // 200mA
uint8_t soc = 85;         // 85%

CAN_Message_t msg = {
    .id = 0x200,  // CAN_EPS_TELEMETRY
    .data = {
        (voltage >> 8) & 0xFF,
        voltage & 0xFF,
        (current >> 8) & 0xFF,
        current & 0xFF,
        soc,
        0, 0, 0
    }
};
CAN_Transmit(&msg);
```

## 🔧 Integração no Main Loop

```c
int main(void) {
    // Inicialização
    HAL_Init();
    SystemClock_Config();
    MX_FDCAN1_Init();
    
    // Inicializa protocolo CAN
    CAN_Protocol_Init();
    
    while (1) {
        // Processa mensagens recebidas
        CAN_Protocol_ProcessMessages();
        
        // Executa rotina baseada no modo atual
        switch (CAN_GetCurrentMode()) {
            case CDH_MODE_IDLE:
                // Modo ocioso
                break;
                
            case CDH_MODE_NOMINAL:
                if (CAN_IsModeActive()) {
                    if (CAN_GetMissionType() == MISSION_1) {
                        // TODO: Rotina Missão 1
                    } else if (CAN_GetMissionType() == MISSION_2) {
                        // TODO: Processar dados AIS
                    }
                }
                break;
                
            case CDH_MODE_ADCS:
                if (CAN_IsModeActive()) {
                    // TODO: Rotina ADCS
                }
                break;
                
            case CDH_MODE_DETUMBLING:
                if (CAN_IsModeActive()) {
                    // TODO: Rotina destumbling
                }
                break;
        }
        
        HAL_Delay(100);
    }
}
```

## 📝 Arquivos do Projeto

```
Core/
├── Inc/
│   ├── can_driver.h              # Driver CAN baixo nível
│   ├── can_protocol.h            # Protocolo de alto nível
│   └── can_protocol_examples.h   # Exemplos de uso
└── Src/
    ├── drivers/
    │   └── can_driver.c          # Implementação driver
    ├── can_protocol.c            # Implementação protocolo
    └── main.c                    # Loop principal
```

## ⚙️ Configuração FDCAN

- **Modo**: Internal Loopback (para testes)
- **Bitrate**: 500 kbps (nominal)
- **Frame**: Classic CAN (8 bytes fixos)
- **Filtro**: Aceita todos os IDs (0x000 / 0x000)

Para uso em produção, mudar para `FDCAN_MODE_NORMAL` e configurar filtros específicos.

## 🧪 Testando o Sistema

1. **Modo Loopback**: O código atual está em modo loopback interno para testes
2. **Simulação COM**: Use as funções em `can_protocol_examples.h`
3. **Debugger**: Coloque breakpoints em:
   - `CAN_Protocol_ProcessMessages()` - Ver mensagens recebidas
   - `CAN_HandleModeCommand()` - Ver mudanças de modo
   - `HAL_FDCAN_RxFifo0Callback()` - Ver interrupções de RX

## 📊 Fluxo de Comunicação Típico (OTIMIZADO)

### Missão 1:
```
COM → CDH: 0x301 [1, 0, 0, 0, 0, 0, 0, 0] (Modo NOMINAL + Missão 1)
CDH → COM: 0x102 (ACK)
CDH → COM: 0x101 (Status: NOMINAL, MISSION_1, ACTIVE)

[CDH executa rotina da Missão 1]

COM → CDH: 0x30F (Sair do modo)
CDH → COM: 0x102 (ACK)
CDH → COM: 0x101 (Status: IDLE, MISSION_NONE, INACTIVE)
```

### Missão 2 com dados AIS:
```
COM → CDH: 0x301 [2, AA, BB, CC, DD, EE, FF, 11] (Modo NOMINAL + Missão 2 + AIS inicial)
CDH → COM: 0x102 (ACK)
CDH → COM: 0x101 (Status: NOMINAL, MISSION_2, ACTIVE)

COM → CDH: 0x320 [22, 33, 44, 55, 66, 77, 88, 99] (Mais dados AIS)
CDH → COM: 0x102 (ACK)

[CDH processa/repassa dados AIS continuamente]

COM → CDH: 0x30F (Sair do modo)
CDH → COM: 0x102 (ACK)
CDH → COM: 0x101 (Status: IDLE, MISSION_NONE, INACTIVE)
```

**Vantagem:** Redução de 50% nas mensagens enviadas! ✨

## 📚 Referências

- STM32H7 FDCAN HAL Driver
- Classic CAN 2.0B Protocol
- OdinSat Mission Requirements

---

**Desenvolvido para**: OdinSat CubeSat Project  
**Subsistema**: CDH (Command & Data Handling)  
**Última atualização**: 2025
