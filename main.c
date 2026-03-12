#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

// Módulos Externos (Sensores)
#include "dht.h"
#include "ssd1306.h"

// Nossos Componentes
#include "nvs_log.h"
#include "wifi_mqtt.h"
#include "atuadores.h"
#include "teclado.h"

static const char *TAG = "ESTACAO_IOT";

// --- Pinos Fixos (Wokwi / ESP32-S2) ---
#define PINO_DHT 15
#define PINO_LDR_ADC_CH 4 
#define PINO_OLED_SDA 8
#define PINO_OLED_SCL 9

// --- Variáveis Globais de Monitorização ---
float temp_atual = 0.0;
float umid_atual = 0.0;
int luz_atual = 0;

// --- Limites de Alarme (Set Points) ---
float temp_max = 30.0, temp_min = 20.0;
int luz_max = 900, luz_min = 100;
float umid_max = 80.0, umid_min = 30.0;

bool alarme_disparado = false;
int ecra_atual = 0; // Telas de 0 a 7

SSD1306_t oled;
adc_oneshot_unit_handle_t adc1_handle;

// --- Função Auxiliar: Obter Data/Hora Simulada ---
void obter_timestamp(char* buffer) {
    // Usa o tempo de atividade do sistema (ticks) para gerar segundos dinâmicos no log
    uint32_t tempo_segundos = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
    sprintf(buffer, "2024-12-12 14:30:%02d", (int)(tempo_segundos % 60)); 
}

// Tarefa 1: Leitura de Sensores, Alarmes e MQTT
void task_sistema(void *pvParameter) {
    char data_hora[30];

    while(1) {
        // 1. Ler DHT11
        int16_t t = 0, h = 0;
        if(dht_read_data(DHT_TYPE_DHT11, PINO_DHT, &h, &t) == ESP_OK) {
            temp_atual = t / 10.0;
            umid_atual = h / 10.0;
        }

        // 2. Ler LDR
        int raw_val = 0;
        adc_oneshot_read(adc1_handle, PINO_LDR_ADC_CH, &raw_val);
        luz_atual = raw_val;

        // 3. Verificar Alarmes
        bool condicao_alarme = (temp_atual > temp_max || temp_atual < temp_min ||
                                umid_atual > umid_max || umid_atual < umid_min ||
                                luz_atual > luz_max || luz_atual < luz_min);

        if (condicao_alarme && !alarme_disparado) {
            alarme_disparado = true;
            controlar_buzzer(true);
            obter_timestamp(data_hora);
            registrar_alarme_nvs(data_hora, temp_atual, umid_atual, luz_atual);
            ESP_LOGW(TAG, "ALARME ATIVADO!");
        }

        // 4. Publicar no Adafruit IO
        publicar_mqtt("temperatura", temp_atual);
        publicar_mqtt("umidade", umid_atual);
        publicar_mqtt("luminosidade", luz_atual);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// Tarefa 2: Interface (Ecrã OLED e Teclado)
void task_interface(void *pvParameter) {
    char linha[32];
    char tecla_anterior = '\0';

    while(1) {
        char tecla = ler_teclado();
        
        if (tecla != '\0' && tecla != tecla_anterior) {
            if (tecla == 'A') { 
                ecra_atual++;
                if(ecra_atual > 7) ecra_atual = 0;
            } 
            else if (tecla == 'D' && alarme_disparado) {
                alarme_disparado = false;
                controlar_buzzer(false);
                ESP_LOGI(TAG, "Buzzer desligado pelo teclado.");
            }
            else if (tecla == 'B') { 
                switch(ecra_atual) {
                    case 1: temp_max += 1.0; break;
                    case 2: temp_min += 1.0; break;
                    case 3: umid_max += 5.0; break;
                    case 4: umid_min += 5.0; break;
                    case 5: luz_max += 50; break;
                    case 6: luz_min += 50; break;
                }
            }
            else if (tecla == 'C') { 
                switch(ecra_atual) {
                    case 1: temp_max -= 1.0; break;
                    case 2: temp_min -= 1.0; break;
                    case 3: umid_max -= 5.0; break;
                    case 4: umid_min -= 5.0; break;
                    case 5: luz_max -= 50; break;
                    case 6: luz_min -= 50; break;
                }
            }
        }
        tecla_anterior = tecla;

        ssd1306_clear_screen(&oled, false);

        if (alarme_disparado) {
            ssd1306_display_text(&oled, 0, "!!! ALARME !!!", 14, false);
            ssd1306_display_text(&oled, 2, "Pressione 'D'", 13, false);
            ssd1306_display_text(&oled, 3, "Para Silenciar", 14, false);
        } else {
            switch(ecra_atual) {
                case 0:
                    sprintf(linha, "Temp: %.1fC", temp_atual);
                    ssd1306_display_text(&oled, 0, linha, strlen(linha), false);
                    sprintf(linha, "Umid: %.1f%%", umid_atual);
                    ssd1306_display_text(&oled, 2, linha, strlen(linha), false);
                    sprintf(linha, "Luz: %d", luz_atual);
                    ssd1306_display_text(&oled, 4, linha, strlen(linha), false);
                    break;
                case 1: sprintf(linha, "Temp Max: %.1f", temp_max); ssd1306_display_text(&oled, 0, linha, strlen(linha), false); break;
                case 2: sprintf(linha, "Temp Min: %.1f", temp_min); ssd1306_display_text(&oled, 0, linha, strlen(linha), false); break;
                case 3: sprintf(linha, "Umid Max: %.1f", umid_max); ssd1306_display_text(&oled, 0, linha, strlen(linha), false); break;
                case 4: sprintf(linha, "Umid Min: %.1f", umid_min); ssd1306_display_text(&oled, 0, linha, strlen(linha), false); break;
                case 5: sprintf(linha, "Luz Max: %d", luz_max); ssd1306_display_text(&oled, 0, linha, strlen(linha), false); break;
                case 6: sprintf(linha, "Luz Min: %d", luz_min); ssd1306_display_text(&oled, 0, linha, strlen(linha), false); break;
                case 7: 
                    ssd1306_display_text(&oled, 0, "> SISTEMA", 9, false);
                    ssd1306_display_text(&oled, 2, "WiFi: OK", 8, false);
                    ssd1306_display_text(&oled, 4, "MQTT: Conectado", 15, false);
                    break;
            }
            if(ecra_atual >= 1 && ecra_atual <= 6) {
                ssd1306_display_text(&oled, 4, "B: +  | C: -", 12, false);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Tarefa 3: Leitura de Comandos da Consola (Serial)
void task_serial(void *pvParameter) {
    char comando[20];
    while(1) {
        if (scanf("%19s", comando) == 1) {
            if (strcmp(comando, "lerlog") == 0) {
                ler_logs_nvs();
            } else if (strcmp(comando, "apagarlog") == 0) {
                apagar_logs_nvs();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "A Iniciar Estacao IoT...");

    iniciar_nvs();
    iniciar_atuadores();
    iniciar_teclado();
    iniciar_wifi_estacao();
    iniciar_mqtt();

    adc_oneshot_unit_init_cfg_t init_config = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&init_config, &adc1_handle);
    adc_oneshot_chan_cfg_t config_adc = { .bitwidth = ADC_BITWIDTH_DEFAULT, .atten = ADC_ATTEN_DB_12 };
    adc_oneshot_config_channel(adc1_handle, PINO_LDR_ADC_CH, &config_adc);

    i2c_master_init(&oled, PINO_OLED_SDA, PINO_OLED_SCL, -1);
    ssd1306_init(&oled, 128, 64);
    ssd1306_clear_screen(&oled, false);

    xTaskCreate(task_sistema, "TaskSistema", 8192, NULL, 5, NULL);
    xTaskCreate(task_interface, "TaskInterface", 4096, NULL, 5, NULL);
    xTaskCreate(task_serial, "TaskSerial", 4096, NULL, 1, NULL);
}