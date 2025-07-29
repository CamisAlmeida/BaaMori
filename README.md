# BaaMori

**BaaMori** (junção de *Baa-san* + *Mamori*, que significa “proteger”) é um projeto inspirado no anime *Jiisan Baasan Wakagaeru* (“Vovô e Vovó Ficam Jovens de Novo”), cujo objetivo é desenvolver um sensor de quedas para idosos que permita maior independência ao usuário e ofereça segurança às famílias, notificando rapidamente em caso de queda para socorro imediato.

---

## Sobre o Projeto

Definir o que caracteriza uma queda foi o maior desafio, pois é um evento complexo com começo, meio e fim. Inicialmente, utilizei o banco de dados [WEDA-Fall](https://github.com/joaojtmarques/WEDA-FALL/tree/main), porém a falta de definições claras dificultou seu uso, então construí meu próprio banco de dados.

Para coletar dados precisos, utilizei um botão que inicia a gravação no momento exato da queda e para quando pressionado novamente, evitando misturas entre quedas e movimentos normais.

Com os dados coletados, criei uma árvore de decisão que inicialmente alcançou mais de 90% de precisão, mas gerava muitos falsos positivos com movimentos simples. Refinando o modelo com estatísticas (mínimo, máximo e média) dos dados, a detecção melhorou consideravelmente.

Uma interface web exibe os dados em tempo real, indica quedas detectadas e permite o download dos dados para análise posterior. A implementação da ligação automática para emergência ficou para versões futuras.

---

## Como Funciona

- O sensor MPU6050 lê aceleração e giroscópio via I2C.
- Dados são armazenados em buffer circular para cálculo de estatísticas.
- Uma árvore de decisão embutida avalia os dados para identificar quedas.
- Quando uma queda é detectada, um alerta é acionado (LED ou buzzer) e uma mensagem é exibida no monitor serial.
- Uma página web atualiza os dados a cada 0,5 segundos, mostrando os valores coletados, o estado de queda e permite baixar os dados em CSV.
- Botão físico controla o início e o fim da coleta de dados.
- O ESP32 atua como cliente Wi-Fi, conectando-se à rede definida.

---

## Estrutura do Código

- Inclusão das bibliotecas `Wire.h`, `WiFi.h` e `WebServer.h`.
- Definição de pinos para botão (`GPIO4`) e alerta (`GPIO2`).
- Configuração do MPU6050 via I2C (pinos SDA 21, SCL 22).
- Função `datacollection()` para ler e converter dados do sensor.
- Buffer circular para armazenar últimas 5 leituras.
- Cálculo de estatísticas (média, mínimo, máximo, delta).
- Implementação da árvore de decisão para detectar queda.
- Servidor web com rotas para página principal, dados em tempo real, CSV e estado de queda.
- Atualização da página web via JavaScript a cada 500 ms.

---

## Como usar

1. Configure seu ambiente Arduino com a placa ESP32.
2. Ajuste as credenciais Wi-Fi no código (`ssid` e `password`).
3. Faça o upload do código para o ESP32.
4. Conecte o sensor MPU6050 e o botão conforme os pinos definidos.
5. Acesse o IP do ESP32 no navegador para visualizar dados em tempo real.
6. Pressione o botão para iniciar e parar a coleta durante uma queda simulada.

---

## Considerações Finais

O sensor no pulso apresentou muitas falhas devido aos movimentos variados do braço. Locais como antebraço ou cintura podem oferecer maior precisão e reduzir falsos positivos. Futuras versões devem incluir chamadas automáticas para emergências e um app para monitoramento e integração de dados biométricos adicionais.

---

## Referências

- [Como medir ACELERAÇÃO com ARDUINO | TUTORIAL MPU-6050 e Arduino Serial Plotter](https://www.youtube.com/)
- BRASIL 61. Acidente doméstico: 1 em cada 3 idosos sofre uma queda por ano, estima Into. 2021. https://brasil61.com/n/acidente-domestico-1-em-cada-3-idosos-sofre-uma-queda-por-ano-estima-into-bras2412869
- AGÊNCIA ESTADO. Mais idosos vivem sozinhos no Brasil, revela censo do IBGE. UOL Notícias, 25 out. 2024. https://noticias.uol.com.br/ultimas-noticias/agencia-estado/2024/10/25/mais-idosos-vivem-sozinhos-no-brasil-revela-censo-do-ibge.htm
- SILVA, L. S. et al. Sistema de detecção e alerta de quedas para idosos usando aceleração e giroscópio. Revista Brasileira de Informática em Saúde, 2023. https://bdm.unb.br/bitstream/10483/27845/1/2019_JoaoPauloVieira_MatheusHenriqueAraujo_tcc.pdf
- PEREIRA, J. C. et al. Dispositivos IoT vestíveis: viabilidade e eficácia na detecção de quedas com ESP32. Simpósio Brasileiro de IoT, 2024. https://repositorio.ifes.edu.br/handle/123456789/5494
- FERNANDES, A. R. et al. Protótipo IoHT para monitoramento de quedas em idosos. Healthcare Technology, 2023. https://periodicos.ufersa.edu.br/ecop/article/view/11164
- [WEDA-FALL - Base de dados para detecção de quedas](https://github.com/joaojtmarques/WEDA-FALL/tree/main)

---

## Licença

Este projeto está sob a licença MIT. Veja o arquivo LICENSE para mais detalhes.
