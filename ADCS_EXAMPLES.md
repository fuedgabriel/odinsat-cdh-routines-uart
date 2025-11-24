

- **Joystick Esquerda (X negativo)** → `Serial.print("M-125")` → Motor gira para **esquerda**
- **Joystick Direita (X positivo)** → `Serial.print("M125")` → Motor gira para **direita**
- **Botão X (Cross)** → `Serial.print("M0")` → Motor **para**

---


```c
/* USER CODE BEGIN 2 */
CAN_Protocol_Init();
HAL_Delay(10);

ADCS_Init();  // Inicializa motor SimpleFOC
HAL_Delay(100);

// ===== TESTE MANUAL =====
// Gira para direita com velocidade 80
ADCS_SetSpeed(80);
HAL_Delay(3000);  // 3 segundos

// Para o motor
ADCS_Stop();
HAL_Delay(1000);

// Gira para esquerda com velocidade -60
ADCS_SetSpeed(-60);
HAL_Delay(3000);

// Para novamente
ADCS_Stop();
/* USER CODE END 2 */
```

**O que vai acontecer:**
1. Motor inicializa → Envia `MC1\n`
2. Motor gira direita → Envia `M80\n`
3. Motor para → Envia `M0\n`
4. Motor gira esquerda → Envia `M-60\n`
5. Motor para → Envia `M0\n`

---

