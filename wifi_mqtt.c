#include <stdio.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "wifi_mqtt.h"
#include "atuadores.h" // Adicionado para poder controlar o LED RGB

#define TOPICO_BASE ADAFRUIT_USER "/feeds/"

static const char *TAG = "WIFI_MQTT";
esp_mqtt_client_handle_t cliente_mqtt = NULL;

static void tratador_eventos_wifi(void* arg, esp_event_base_t base, int32_t id, void* data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi desconectado. Tentando reconectar...");
        esp_wifi_connect(); 
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) data;
        ESP_LOGI(TAG, "Conectado! IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void iniciar_wifi_estacao(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &tratador_eventos_wifi, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &tratador_eventos_wifi, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "Wokwi-GUEST", 
            .password = "",
            .threshold.authmode = WIFI_AUTH_OPEN,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "WiFi Station Iniciado.");
}

static void tratador_eventos_mqtt(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    
    // Calcula o tópico esperado para o LED RGB
    char topico_led[100];
    sprintf(topico_led, "%srgb", TOPICO_BASE);

    if (event_id == MQTT_EVENT_CONNECTED) {
        ESP_LOGI(TAG, "MQTT Conectado ao Adafruit IO!");
        esp_mqtt_client_subscribe(cliente_mqtt, topico_led, 0);
        
    } else if (event_id == MQTT_EVENT_DATA) {
        // Se a mensagem chegou no tópico do LED RGB e possui o formato Hexadecimal (ex: #FF0000)
        if (strncmp(event->topic, topico_led, event->topic_len) == 0) {
            if (event->data_len >= 7 && event->data[0] == '#') {
                int r, g, b;
                // Converte a string "#RRGGBB" para valores inteiros
                sscanf(event->data, "#%02x%02x%02x", &r, &g, &b);
                
                // Como nossos LEDs no Wokwi são digitais (só aceitam 0 ou 1), 
                // consideramos "ligado" qualquer cor com intensidade maior que 127
                controlar_led_rgb(r > 127, g > 127, b > 127);
                ESP_LOGI(TAG, "LED RGB Atualizado para R:%d G:%d B:%d", r, g, b);
            }
        }
    }
}

void iniciar_mqtt(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://io.adafruit.com:1883",
        .credentials.username = ADAFRUIT_USER,
        .credentials.authentication.password = ADAFRUIT_KEY,
    };

    cliente_mqtt = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(cliente_mqtt, ESP_EVENT_ANY_ID, tratador_eventos_mqtt, NULL);
    esp_mqtt_client_start(cliente_mqtt);
}

void publicar_mqtt(const char* feed, float valor) {
    if (cliente_mqtt == NULL) return;
    
    char topico[100];
    char payload[20];
    
    sprintf(topico, "%s%s", TOPICO_BASE, feed);
    sprintf(payload, "%.2f", valor);
    
    esp_mqtt_client_publish(cliente_mqtt, topico, payload, 0, 1, 0);
}