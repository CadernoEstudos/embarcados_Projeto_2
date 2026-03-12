#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "nvs_log.h"

static const char *TAG = "NVS_LOG";
int contador_logs = 0; // Controla quantos alarmes já foram salvos

void iniciar_nvs(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Recupera o contador salvo anteriormente, se existir
    nvs_handle_t my_handle;
    if (nvs_open("storage", NVS_READWRITE, &my_handle) == ESP_OK) {
        nvs_get_i32(my_handle, "contador", &contador_logs);
        nvs_close(my_handle);
    }
    ESP_LOGI(TAG, "NVS Inicializado. Logs atuais: %d", contador_logs);
}

void registrar_alarme_nvs(const char* data_hora, float temp, float umid, int luz) {
    nvs_handle_t my_handle;
    if (nvs_open("storage", NVS_READWRITE, &my_handle) != ESP_OK) return;

    char chave[15];
    char mensagem[100];
    
    // Cria a chave (ex: log_0, log_1) e formata a mensagem
    sprintf(chave, "log_%d", contador_logs);
    sprintf(mensagem, "%s - Temp:%.1fC Umid:%.1f%% Luz:%d", data_hora, temp, umid, luz);

    nvs_set_str(my_handle, chave, mensagem);
    
    contador_logs++;
    nvs_set_i32(my_handle, "contador", contador_logs); // Atualiza o contador geral
    nvs_commit(my_handle);
    nvs_close(my_handle);
    
    ESP_LOGW(TAG, "Alarme Salvo: %s", mensagem);
}

void ler_logs_nvs(void) {
    nvs_handle_t my_handle;
    if (nvs_open("storage", NVS_READONLY, &my_handle) != ESP_OK) {
        printf("Nenhum log encontrado.\n");
        return;
    }

    printf("\n--- ALARM LOGS ---\n");
    for (int i = 0; i < contador_logs; i++) {
        char chave[15];
        char mensagem[100];
        size_t tamanho = sizeof(mensagem);
        sprintf(chave, "log_%d", i);
        
        if (nvs_get_str(my_handle, chave, mensagem, &tamanho) == ESP_OK) {
            printf("[%d] %s\n", i, mensagem);
        }
    }
    printf("------------------\n\n");
    nvs_close(my_handle);
}

void apagar_logs_nvs(void) {
    nvs_handle_t my_handle;
    if (nvs_open("storage", NVS_READWRITE, &my_handle) == ESP_OK) {
        nvs_erase_all(my_handle); // Apaga tudo
        nvs_commit(my_handle);
        nvs_close(my_handle);
        contador_logs = 0;
        printf("Historico de alarmes apagado!\n");
    }
}