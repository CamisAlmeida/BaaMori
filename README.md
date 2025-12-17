# BaaMori

**BaaMori** (junção de *Baa-san* + *Mamori*, que significa “proteger”) é um projeto inspirado no anime *Jiisan Baasan Wakagaeru* (“Vovô e Vovó Ficam Jovens de Novo”), cujo objetivo é desenvolver um sistema de **detecção de quedas para idosos**, promovendo maior independência ao usuário e oferecendo segurança às famílias, com **notificação automática em caso de queda** para socorro imediato.

---

## Sobre o Projeto

Definir o que caracteriza uma queda foi o maior desafio, pois trata-se de um evento complexo com início, meio e fim bem definidos. Inicialmente, foi analisado o banco de dados [WEDA-Fall](https://github.com/joaojtmarques/WEDA-FALL/tree/main), porém a ausência de critérios claros dificultou sua utilização, motivando a criação de um **banco de dados próprio**.

Para a coleta precisa dos dados, foi utilizado um botão físico que inicia a gravação no momento exato da queda e encerra ao ser pressionado novamente, evitando a mistura entre movimentos normais e eventos de queda.

Com os dados coletados, foi construída uma **árvore de decisão**, que inicialmente atingiu mais de 90% de precisão, porém apresentava muitos falsos positivos em movimentos simples. A partir do uso de **estatísticas (mínimo, máximo e média)** extraídas dos sinais de aceleração e giroscópio, o modelo foi refinado, reduzindo significativamente os falsos alarmes.

O sistema conta com uma **interface web** que exibe os dados coletados, indica eventos de queda e permite visualização histórica. Funcionalidades como ligação automática para emergência ficam previstas para versões futuras.

---

## Como Funciona

- O sensor **MPU6050** realiza a leitura de aceleração e giroscópio via I2C.
- Os dados são armazenados em um **buffer circular**.
- Estatísticas (mínimo, máximo e média) são calculadas em janelas de tempo.
- Uma **árvore de decisão embarcada** avalia os dados para detectar quedas.
- O sistema utiliza o **padrão Observer**, desacoplando a detecção do envio de dados.
- Em caso de queda:
  - Os dados são enviados para o **ThingsBoard**;
  - Um evento é enviado ao **API Gateway**.
- Sem queda:
  - O ESP32 envia apenas dados de status periódicos ao ThingsBoard.
- O **Dashboard Web** consulta o ThingsBoard e exibe os dados em tempo quase real.

---

## Arquitetura do Sistema

O sistema é dividido em três camadas principais:

### Dispositivo Embarcado
- ESP32
- Sensor MPU6050
- Processamento local
- Detecção de quedas
- Envio de dados via HTTP

### Backend (Microserviços)
- **API Gateway (FastAPI)**: recebe eventos de queda e os encaminha para outros serviços
- **Dashboard Service (FastAPI)**: consulta dados no ThingsBoard

### Frontend
- Página HTML estática
- Consome a API do Dashboard Service
- Exibe dados históricos e status de queda

---

## Estrutura do Código

### Firmware (ESP32)
- Leitura do MPU6050 via I2C
- Buffer circular de dados
- Cálculo de estatísticas
- Árvore de decisão convertida para C/C++
- Implementação do padrão **Observer**
- Envio de dados para ThingsBoard e Gateway
- Sincronização de tempo via SNTP
- Identificação única do dispositivo via MAC

### Backend
- FastAPI
- Comunicação HTTP
- Integração com ThingsBoard
- Arquitetura baseada em microserviços

### Frontend
- HTML + CSS + JavaScript puro
- Atualização automática dos dados
- Tabela com estatísticas do acelerômetro e giroscópio

---

##  Como Rodar o Sistema

### 1 ESP32

1. Configure as credenciais Wi-Fi.
2. Compile e grave o firmware no ESP32 (ESP-IDF ou PlatformIO).
3. Conecte o sensor MPU6050.
4. Ao iniciar:
   - O ESP32 sincroniza o horário via SNTP;
   - Inicia a coleta contínua;
   - Detecta quedas e envia os dados automaticamente.

---

### 2 API Gateway

Execute o serviço:

```bash
uvicorn gateway_service:app --host 0.0.0.0 --port 8000
