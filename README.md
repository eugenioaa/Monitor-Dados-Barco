# Monitor-Dados-Barco
Projeto Final para a turma 3VB de PROJ PROG MICROCONTROLADORES (ENG4033) da PUC-Rio

Este projeto implementa um sistema completo de monitoramento em tempo real para um barco, integrando sensores físicos, IoT e visualização de dados. Utiliza um Arduino para coleta e envio das medições, um broker MQTT para transporte das mensagens, e ferramentas como Grafana e Streamlit para exibição dos dados em dashboards modernos e intuitivos.

O sistema registra e apresenta informações como:
- Localização (latitude e longitude)
- Temperatura do ar
- Umidade
- Pressão atmosférica
- Direção e velocidade
- Vibração do motor
- Pressão do tanque
  
Os dados são enviados continuamente pelo Arduino, armazenados em banco de dados e exibidos em tempo real para facilitar o acompanhamento do estado do barco e apoiar decisões operacionais.
