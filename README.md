# Projeto Final – ENG4033 – OceanMonitor 🚤
Projeto final desenvolvido pela turma 3VB da disciplina Projeto e Programação de Microcontroladores (ENG4033) da PUC-Rio.

## Resumo 📄
Este projeto implementa um sistema de monitoramento em tempo real para um barco, integrando sensores físicos, IoT e visualização de dados. Utiliza dois Arduinos para coleta e envio das medições, um broker MQTT para transporte das mensagens, além de Grafana e Streamlit para exibição em dashboards modernos e intuitivos.

O sistema registra e apresenta informações como:
- Localização (latitude e longitude)
- Temperatura do ar
- Umidade
- Pressão atmosférica
- Direção e velocidade
- Vibração do motor
- Pressão do tanque

Os dados são transmitidos continuamente pelo Arduino utilizando LoRaWAN, armazenados em um banco PostgreSQL e exibidos em tempo real para facilitar o acompanhamento do estado do barco e apoiar decisões operacionais.

---

## MQTT 🛜
MQTT é um protocolo de comunicação leve e eficiente, amplamente utilizado em aplicações IoT para envio e recebimento de dados entre dispositivos.  
Neste projeto, ele atua como intermediário entre os Arduinos e as interfaces de visualização (Grafana e Streamlit), transportando as medições coletadas pelos sensores.

---

## Node-RED ⭕
Node-RED é uma ferramenta visual baseada em fluxo que permite integrar, transformar e automatizar dados entre dispositivos, APIs e serviços.

No projeto, o Node-RED serve como camada de integração entre o MQTT e o banco de dados PostgreSQL, garantindo que todas as mensagens enviadas pelo Arduino sejam processadas e armazenadas corretamente.

*Esquema do fluxo utilizado:*  
<img width="1033" alt="image" src="https://github.com/user-attachments/assets/04fce427-ebaf-4877-a377-a0da35b1fa69" />

---

## Grafana 📈
Grafana é uma plataforma de visualização e monitoramento que permite criar dashboards interativos para análise de dados em tempo real.  
Aqui, ele apresenta as medições recebidas dos sensores do barco, oferecendo uma visualização clara do estado geral da embarcação.

---

## Streamlit 📊
Streamlit é um framework Python para criação rápida de aplicações web interativas.  
Neste projeto, ele fornece uma segunda forma de visualização dos dados, oferecendo métricas, gráficos e envio de alertas de maneira simples e dinâmica.

---

## Banco de Dados 🎲
O banco de dados PostgreSQL atua como armazenamento persistente e hub central de dados do sistema.  
A tabela **dados_barco** reúne todas as informações necessárias para geração dos dashboards.  
Ela contém:

- **id** — inteiro, chave primária  
- **data_hora** — timestamp da coleta  
- **latitude** — float  
- **longitude** — float  
- **temperatura_ar** — float (°C)  
- **umidade** — float (%)  
- **pressao_atmosferica** — float (hPa)  
- **direcao** — inteiro (1 = frente, 2 = esquerda, 3 = direita)  
- **velocidade** — float  
- **vibracao_motor** — float  
- **pressao_tanque** — float  

Além disso, o banco possui a tabela auxiliar **alertas**, responsável por armazenar mensagens enviadas via Streamlit, contendo:

- **data_hora** — timestamp  
- **mensagem**  
- **status** — “concluído” ou “em aberto”

---

## Esquemático (Fritzing) 🔎
<img width="1475" alt="image" src="https://github.com/user-attachments/assets/68ce910d-e26b-4f06-9352-10264cfb20f3" />


