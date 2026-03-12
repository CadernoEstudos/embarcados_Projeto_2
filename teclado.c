#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "teclado.h"

// Pinos do teclado no ESP32-S2 
const int pinos_linhas[4] = {1, 2, 3, 4};
const int pinos_colunas[4] = {33, 6, 7, 34};

const char mapa_teclas[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void iniciar_teclado(void) {
    for (int i = 0; i < 4; i++) {
        gpio_reset_pin(pinos_linhas[i]);
        gpio_set_direction(pinos_linhas[i], GPIO_MODE_OUTPUT);
        gpio_set_level(pinos_linhas[i], 1); // Linhas em HIGH por padrão

        gpio_reset_pin(pinos_colunas[i]);
        gpio_set_direction(pinos_colunas[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(pinos_colunas[i], GPIO_PULLUP_ONLY); // Colunas com pull-up
    }
}

char ler_teclado(void) {
    for (int l = 0; l < 4; l++) {
        gpio_set_level(pinos_linhas[l], 0); // Puxa a linha para LOW
        vTaskDelay(pdMS_TO_TICKS(5)); // Pequeno atraso de estabilização

        for (int c = 0; c < 4; c++) {
            if (gpio_get_level(pinos_colunas[c]) == 0) { // Se o botão foi acionado
                gpio_set_level(pinos_linhas[l], 1); // Restaura a linha
                return mapa_teclas[l][c];
            }
        }
        gpio_set_level(pinos_linhas[l], 1); // Restaura a linha
    }
    return '\0'; // Nenhuma tecla acionada
}