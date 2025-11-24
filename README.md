## Estrutura do Projeto

```
odinsat-cdh-routines/
├── Core/
│   ├── inc/            # Cabeçalhos e definições globais
│   └── src/
│       ├── main.cpp        # Inicialização do sistema e scheduler de modos
│       ├── drivers/    # Abstração de sensores e atuadores
│       ├── modules/    # Lógicas principais de cada modo
│       └── utils/      # Bibliotecas auxiliares, timers, logs e handlers
└── README.md
```

## Modos de Operação

Cada rotina principal será desenvolvida em **branches independentes**:

| Branch | Descrição |
|---------|------------|
| `adcs-mode` |Envia sinal para abertura da antena, determina e controla a atitude do satélite a partir da fusão dos dados de sensores ópticos e inerciais. |
| `detumbling-mode` | Estabiliza a rotação até entrar em um limite seguro. |
| `idle-mode` | Modo de espera, coleta e envia dados de telemetria. Utilizado para testes ambientais e testes básicos de telemetria. |
| `nominal-mode` | Executa a missão principal,  enviando e recebendo dados de Payload. |

## Configuração de Acesso Remoto via USB/IP

Quando a placa de CDH estiver fisicamente no laboratório, os membros remotos poderão programar e depurar o microcontrolador via rede.

### Passo 1 — Instalar o usbip-win2

1. Vá até a página de [releases](https://github.com/vadimgrn/usbip-win2/releases).
2. Baixe o **`.exe`** e o **`.zip`** da **última versão estável**.
3. Descompacte ambos em uma pasta local.
4. Execute o `.exe` **como administrador**.

### Passo 2 — Habilitar o modo de teste do Windows

Se os drivers não forem assinados pela Microsoft, habilite o modo de teste:

```powershell
bcdedit.exe /set testsigning on
```

Reinie o sistema para aplicar.

### Passo 3 — Conectar ao ST-Link remoto

1. Abra o executável USPip gerado após a instalação.
2. Clique em Attach e insira o IP e a porta fornecidos pelo laboratório.
3. Clique no dispositivo ST-Link listado.

### Passo 4 — Testar a conexão

Abra o STM32CubeProgrammer e tente conectar-se à placa. Se a conexão for bem-sucedida, o setup está completo.



