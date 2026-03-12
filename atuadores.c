#include "driver/gpio.h"
#include "atuadores.h"

// Pinos definidos diretamente para simplificar o Wokwi
#define PINO_BUZZER 13
#define PINO_LED_R  10
#define PINO_LED_G  11
#define PINO_LED_B  12

void iniciar_atuadores(void) {
    // Configura o Buzzer
    gpio_reset_pin(PINO_BUZZER);
    gpio_set_direction(PINO_BUZZER, GPIO_MODE_OUTPUT);
    gpio_set_level(PINO_BUZZER, 0);

    // Configura os LEDs
    gpio_reset_pin(PINO_LED_R);
    gpio_set_direction(PINO_LED_R, GPIO_MODE_OUTPUT);
    gpio_set_level(PINO_LED_R, 0);

    gpio_reset_pin(PINO_LED_G);
    gpio_set_direction(PINO_LED_G, GPIO_MODE_OUTPUT);
    gpio_set_level(PINO_LED_G, 0);

    gpio_reset_pin(PINO_LED_B);
    gpio_set_direction(PINO_LED_B, GPIO_MODE_OUTPUT);
    gpio_set_level(PINO_LED_B, 0);
}

void controlar_buzzer(bool estado) {
    gpio_set_level(PINO_BUZZER, estado ? 1 : 0);
}

void controlar_led_rgb(bool r, bool g, bool b) {
    gpio_set_level(PINO_LED_R, r ? 1 : 0);
    gpio_set_level(PINO_LED_G, g ? 1 : 0);
    gpio_set_level(PINO_LED_B, b ? 1 : 0);
}