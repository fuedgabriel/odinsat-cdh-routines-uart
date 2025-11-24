# 🔧 Integração BMI088 ↔ ADCS

## 📋 Resumo da Integração

O sistema ADCS agora lê dados **reais** do sensor BMI088 (acelerômetro + giroscópio) via SPI4 e usa essas leituras para controlar o motor SimpleFOC.

---

## 🎯 Fluxo de Dados

```
BMI088 (SPI4)  →  ADCS_ReadSensors()  →  PID  →  SimpleFOC (UART4)
   ↓                      ↓                ↓           ↓
Gyro Z (rad/s)    sensors.gyro_z    motor_speed   M{speed}\n
```

---

## 🔌 Hardware

### **SPI4 - BMI088:**
- **Acelerômetro CS:** `OBC_CS_ACC` (GPIOE, Pin 4)
- **Giroscópio CS:** `OBC_CS_GYR` (GPIOI, Pin 7)
- **Clock:** SPI4 (configurado em `spi.c`)

### **UART4 - SimpleFOC:**
- **TX/RX:** Comunicação com motor
- **Baudrate:** 115200

---

## 🚀 Como Funciona

### **1. Inicialização Automática**

Quando você chama `ADCS_Init()`, o sistema:

```c
ADCS_Init();
// ↓
// 1. Inicializa BMI088 via SPI4
// 2. Configura acelerômetro (±3g, 100Hz)
// 3. Configura giroscópio (±1000°/s, 100Hz)
// 4. Envia "MC1\n" para SimpleFOC
// 5. Marca sensor como inicializado
```

### **2. Leitura de Sensores**

Dentro de `ADCS_Process()`, a cada ciclo:

```c
void ADCS_Process(void)
{
    ADCS_ReadSensors(&sensors);
    // ↓
    // 1. BMI088_ReadAccelerometer() via SPI
    // 2. BMI088_ReadGyroscope() via SPI
    // 3. Copia para sensors.gyro_x/y/z e accel_x/y/z
    
    // Usa gyro_z para PID
    float pid_output = ADCS_PID_Compute(&pid_controller, sensors.gyro_z);
    
    // Converte para velocidade do motor
    int16_t motor_speed = (int16_t)(pid_output * 10.0f);
    ADCS_SetSpeed(motor_speed);
}
```

---

## 📊 Dados Disponíveis

### **Giroscópio (rad/s):**
```c
sensors.gyro_x  // Rotação em X
sensors.gyro_y  // Rotação em Y
sensors.gyro_z  // Rotação em Z ← Usado no PID
```

### **Acelerômetro (m/s²):**
```c
sensors.accel_x  // Aceleração em X
sensors.accel_y  // Aceleração em Y
sensors.accel_z  // Aceleração em Z (≈9.81 em repouso)
```

---

## 🧪 Teste de Integração

### **Teste 1: Verificar Sensor Inicializado**

```c
/* USER CODE BEGIN 2 */
ADCS_Init();

if (ADCS_IsSensorReady()) {
    // ✅ BMI088 inicializado com sucesso
} else {
    // ❌ Falha ao inicializar sensor
}
/* USER CODE END 2 */
```

### **Teste 2: Ler Dados do Sensor (Debug via UART5)**

```c
/* USER CODE BEGIN WHILE */
while (1)
{
    ADCS_Sensors_t sensors;
    ADCS_ReadSensors(&sensors);
    
    // Debug: Envia dados via UART5 (JTAG)
    char buffer[128];
    sprintf(buffer, "Gyro: %.3f, %.3f, %.3f | Accel: %.3f, %.3f, %.3f\r\n",
            sensors.gyro_x, sensors.gyro_y, sensors.gyro_z,
            sensors.accel_x, sensors.accel_y, sensors.accel_z);
    
    HAL_UART_Transmit(&huart5, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    
    HAL_Delay(1000);
}
/* USER CODE END WHILE */
```

**Saída esperada (UART5):**
```
Gyro: 0.015, -0.008, 0.003 | Accel: 0.12, -0.05, 9.81
Gyro: 0.012, -0.010, 0.005 | Accel: 0.10, -0.03, 9.82
...
```

### **Teste 3: PID com Sensor Real**

```c
/* Envie comando CAN para entrar em modo ADCS */
CAN_Message_t cmd = {
    .id = 0x302,  // CAN_COM_MODE_ADCS
    .data = {0, 0, 0, 0, 0, 0, 0, 0}
};
CAN_Transmit(&cmd);
```

**O que acontece:**
1. `ADCS_Process()` lê `sensors.gyro_z` a cada 100ms
2. PID calcula erro entre setpoint (0°/s) e gyro_z atual
3. Motor recebe comando `M{speed}\n` para estabilizar

---

## 🔍 Detalhes Técnicos

### **Conversão de Unidades (Já feito pelo BMI088.c):**

```c
// Giroscópio: ±1000°/s → rad/s
gyrConversion = 0.01745329251f * 1000.0f / 32768.0f;
imu.gyr_rps[i] = gyrConversion * raw_value;

// Acelerômetro: ±3g → m/s²
accConversion = 9.81f / 32768.0f * 2.0f * 1.5f;
imu.acc_mps2[i] = accConversion * raw_value;
```

### **Taxa de Leitura:**
- **BMI088:** Configurado para 100Hz (ODR)
- **ADCS Loop:** 10Hz (HAL_Delay(100) no main)
- **Ratio:** 10 amostras BMI088 / 1 leitura ADCS

---

## ⚙️ Ajuste do PID

### **Ganhos Atuais:**
```c
pid_controller.Kp = 1.0f;   // Proporcional
pid_controller.Ki = 0.1f;   // Integral
pid_controller.Kd = 0.05f;  // Derivativo
```

### **Como Ajustar:**

1. **Kp (Proporcional):** Aumentar para resposta mais rápida
   - Se muito alto → Oscilação
   - Se muito baixo → Resposta lenta

2. **Ki (Integral):** Elimina erro em regime permanente
   - Se muito alto → Overshoot
   - Se muito baixo → Erro residual

3. **Kd (Derivativo):** Amortece oscilações
   - Se muito alto → Ruído amplificado
   - Se muito baixo → Pouco amortecimento

**Exemplo de ajuste manual:**

```c
// No adcs.c, linha ~22-27
static ADCS_PID_t pid_controller = {
    .Kp = 2.0f,   // ← Aumentar para resposta mais rápida
    .Ki = 0.2f,   // ← Aumentar para eliminar erro
    .Kd = 0.1f,   // ← Aumentar para reduzir overshoot
    .setpoint = 0.0f,
    .error_sum = 0.0f,
    .last_error = 0.0f
};
```

---

## 🐛 Troubleshooting

### **Problema: Sensor não inicializa**

**Sintomas:**
- `ADCS_IsSensorReady()` retorna 0
- Leituras sempre 0.0f

**Soluções:**
1. Verificar conexões SPI4 (MOSI, MISO, SCK)
2. Verificar CS do acelerômetro (GPIOE Pin 4)
3. Verificar CS do giroscópio (GPIOI Pin 7)
4. Confirmar que `MX_SPI4_Init()` foi chamado antes de `ADCS_Init()`

### **Problema: Leituras ruidosas**

**Sintomas:**
- `gyro_z` oscila muito (ex: -5 a +5 rad/s)
- Motor vibra constantemente

**Soluções:**
1. Adicionar filtro passa-baixa:
```c
// Filtro simples (média móvel)
static float gyro_z_filtered = 0.0f;
gyro_z_filtered = 0.9f * gyro_z_filtered + 0.1f * sensors.gyro_z;
```

2. Aumentar dead zone do motor:
```c
#define ADCS_DEAD_ZONE  20  // Era 10
```

3. Reduzir ganho Kd (derivativo amplifica ruído)

### **Problema: Motor não responde**

**Sintomas:**
- Sensor lê valores corretos
- Nenhum comando enviado via UART4

**Soluções:**
1. Verificar modo CAN (`CAN_GetCurrentMode()` deve ser `CDH_MODE_ADCS`)
2. Verificar flag `CAN_IsModeActive()` == 1
3. Adicionar debug no `ADCS_SetSpeed()`:
```c
void ADCS_SetSpeed(int16_t speed)
{
    // ... código existente ...
    
    // Debug
    char debug[50];
    sprintf(debug, "[ADCS] Speed: %d\r\n", speed);
    HAL_UART_Transmit(&huart5, (uint8_t*)debug, strlen(debug), 100);
}
```

---

## 📈 Próximos Passos

1. **Calibração do Sensor:** Implementar offset de giroscópio
2. **Filtro Complementar:** Fundir acelerômetro + giroscópio para estimativa de ângulo
3. **Telemetria via CAN:** Enviar dados do sensor para COM
4. **Modo Seguro:** Parar motor se leitura do sensor falhar

---

## 🎯 Exemplo Completo (main.c)

```c
/* USER CODE BEGIN 2 */
CAN_Protocol_Init();
HAL_Delay(10);

ADCS_Init();  // ← Inicializa BMI088 + SimpleFOC
HAL_Delay(10);

if (!ADCS_IsSensorReady()) {
    // Sensor falhou - usar modo manual ou desativar ADCS
}
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
while (1)
{
    CAN_Protocol_ProcessMessages();

    switch (CAN_GetCurrentMode()) {
        case CDH_MODE_IDLE:
            break;

        case CDH_MODE_NOMINAL:
            UART_ProcessMission();
            break;

        case CDH_MODE_ADCS:
            if (CAN_IsModeActive()) {
                ADCS_Process();  // ← Lê BMI088 + Controla motor
            }
            break;

        case CDH_MODE_DETUMBLING:
            if (CAN_IsModeActive()) {
                ADCS_Process();  // ← Amortecimento automático
            }
            break;
    }

    HAL_Delay(100);
}
/* USER CODE END WHILE */
```

---

## ✅ Checklist de Integração

- [x] BMI088 driver implementado (`BMI088.c/h`)
- [x] ADCS inicializa BMI088 automaticamente
- [x] `ADCS_ReadSensors()` lê dados reais via SPI
- [x] PID usa `gyro_z` para controle
- [x] Motor recebe comandos baseados em sensor
- [x] Getter `ADCS_IsSensorReady()` para verificação
- [ ] Calibração de offset (TODO)
- [ ] Filtro de ruído (TODO)
- [ ] Telemetria via CAN (TODO)

🚀 **Sistema BMI088 + ADCS totalmente integrado!**
