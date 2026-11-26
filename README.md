# 🌹 Microclimate Controller (Projeto Rosas Azuis)

## 📌 Inspiração & Conceito
Este projeto foi desenvolvido a partir do desafio visto no episódio das _Rosas Azuis_ de **The Apothecary Diaries** (_Kusuriya no Hitorigoto_).

A proposta explora como o problema de cultivo forçado fora de estação resolvido empiricamente pela personagem Maomao, seria abordado utilizando a tecnologia moderna de sistemas embarcados. 
O projeto substitui o controle manual por um firmware em C++ rodando em EPS32, capaz de monitorar variáveis ambientais em tempo real e atuar automaticamente no microclima.

## Simulação Interativa (Wokwi)
O circuito e a lógica do firnware podem ser testados interativamente diretamente no navegador sem necessidade de hardware físico.

## Arquitetura & Especificações Técnicas

### Hardware & Componentes
- ESP32: Microcontrolador com Wi-Fi embutido.
- Sensor DHT22: Mede temperatura e umidade.
- LED com Resistor de 220Ω: Indica o comando recebido.

### Software & Ferramentas
- Wokwi: Simulador onde o projeto foi montado e testado.
- ThingSpeak: Guarda as leituras e mostra em gráficos.
- Telegram (bot): Envia alerta para o celular.
## Autores & Integrantes
* Arthur Batista
* Felipe Francisco
* Guilherme
* Manuella Nascimento
* Milton Silva
