# Projeto Final - ENG4033
Projeto Final para a turma 3VB de Projeto Programação de Microcontroladores (ENG4033) da PUC-Rio

# Resumo
Este projeto implementa um sistema de monitoramento em tempo real para um barco, integrando sensores físicos, IoT e visualização de dados. Utiliza dois Arduinos para coleta e envio das medições, um broker MQTT para transporte das mensagens, e ferramentas como Grafana e Streamlit para exibição dos dados em dashboards modernos e intuitivos.

O sistema registra e apresenta informações como:
- Localização (latitude e longitude)
- Temperatura do ar
- Umidade
- Pressão atmosférica
- Direção e velocidade
- Vibração do motor
- Pressão do tanque
  
Os dados são enviados continuamente pelo Arduino, utilizando o protocolo de comunicação de rede de área ampla de baixa potência (LoRaWAN), armazenados em um banco de dados e exibidos em tempo real para facilitar o acompanhamento do estado do barco e apoiar decisões operacionais.

# MQTT
MQTT é um protocolo de comunicação leve e eficiente, usado para enviar e receber dados entre dispositivos em tempo real, especialmente em aplicações de IoT. 
Neste projeto, o MQTT é utilizado como uma ferramenta de comunicação entre o Arduino e as GUI's do Grafana e Streamlit, permitindo a visualização dos dados enviados pelos sensores.

# Node-Red
O Node-RED consiste em uma ferramenta visual baseada em fluxo que permite integrar, processar e automatizar dados entre dispositivos, APIs e serviços de forma simples e intuitiva.
No projeto, o Node-Red age como a camada de integração entre os dados enviados via MQTT e o banco de dados PostgreSQL.
<img width="1033" height="192" alt="image" src="https://github.com/user-attachments/assets/04fce427-ebaf-4877-a377-a0da35b1fa69" />


# Grafana
Grafana é uma plataforma de visualização e monitoramento que permite criar dashboards interativos para analisar dados em tempo real provenientes de diversas fontes.
Aqui, ele mostra os dados enviados pelos sensores do barco.

# Streamlit
Streamlit é um framework em Python que permite criar aplicações web interativas e dashboards de forma rápida e simples, sem necessidade de conhecimentos avançados de front-end.
Neste projeto, ele é outra alternativa de visualização de dados, trazendo uma versão simplificada do que é mostrado no dashboard do Grafana

# Banco de Dados
O banco de dados PostgreSQL (postgresql.janks.dev.br) serve como a memória persistente e o hub de dados do sistema.
A tabela dados_barco é o registro que contém todas as informações necessárias para gerar as visualizações e métricas dos dashboards.
ela possui as seguintes colunas:
- id - um valor inteiro para a identificação das linhas da tabela (chave primária)
- data_hora - um timestamp, que mostra quando os dados foram armazenados no banco
- latitude - float, representando coordenadas
- longitude - float, representando coordenadas
- temperatura_ar - float, representando a temperatura do ar em Celsius
- umidade - float, representando o percentual de umidade
- pressao_atmosferica - float, representando a pressão atmosférica em hPa
- direcao - inteiros, variando entre 1, 2 e 3, representando respectivamente "frente", "esquerda" e "direita"
- velocidade - float, representando a velocidade do barco
- vibracao_motor - float, representando as vibrações do motor
- pressao_tanque - float, mostrando o nível de pressão no tanque de combustível

# Esquemático Fritzing

<img width="1475" height="696" alt="image" src="https://github.com/user-attachments/assets/68ce910d-e26b-4f06-9352-10264cfb20f3" />

