# Estação IoT Interativa com FreeRTOS e MQTT 🌐

Projeto de conclusão desenvolvendo uma Estação IoT utilizando a placa **Franzininho WiFi LAB01** (ESP32-S2). O sistema integra sensores ambientais, atuadores, interface física (OLED e Teclado Matricial) e conectividade em nuvem via protocolo MQTT.

## 📋 Descrição do Projeto

Esta estação atua como um nó inteligente de monitoramento e controle. As suas principais funcionalidades incluem:
- **Monitoramento:** Leitura contínua de Temperatura e Umidade (DHT11) e Luminosidade (LDR).
- **Conectividade Cloud:** Publicação periódica dos dados via MQTT para o broker **Adafruit IO**, permitindo visualização em gráficos em tempo real.
- **Controle Remoto e Local:** O LED RGB pode ser controlado remotamente pelo painel web do Adafruit IO. Localmente, um *Buzzer* atua como alarme sonoro caso os sensores ultrapassem os limites configurados.
- **Interface Homem-Máquina (IHM):** Display OLED para exibição de dados e status de rede, e um Teclado Matricial 4x4 para navegação de menus, configuração de limites de alarme (Set Points) e silenciamento do Buzzer.
- **Datalogging Não-Volátil:** Registro persistente dos eventos de alarme (com Timestamp) utilizando a partição NVS da memória Flash do ESP32.

## ⚙️ Instruções de Instalação e Uso

### Configuração do Hardware (Pinos)
O projeto foi mapeado para a seguinte pinagem no ESP32-S2:
- **Sensores:** DHT11 (GPIO 15) | LDR (ADC CH 4 / GPIO 5)
- **I2C (OLED):** SDA (GPIO 8) | SCL (GPIO 9)
- **Atuadores:** Buzzer (GPIO 13) | LED RGB (GPIO 10, 11, 12)
- **Teclado 4x4:** Linhas (1, 2, 3, 4) | Colunas (33, 6, 7, 34)

### Configuração da Nuvem (Adafruit IO)
1. Crie uma conta no [Adafruit IO](https://io.adafruit.com/).
2. Obtenha o seu *Username* e *Active Key*.
3. Insira essas credenciais no arquivo `components/wifi_mqtt/wifi_mqtt.h`.
4. Crie um Dashboard e vincule os blocos aos feeds: `temperatura`, `umidade`, `luminosidade` e `rgb`.

### Como compilar e executar
```bash
# Compile o projeto
idf.py build

# Grave na placa 
idf.py -p PORTA flash

# Abra o monitor serial para gerenciar os logs (comandos: 'lerlog' e 'apagarlog')
idf.py -p PORTA monitor
