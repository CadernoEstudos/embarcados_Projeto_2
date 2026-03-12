#ifndef ATUADORES_H
#define ATUADORES_H

#include <stdbool.h>

void iniciar_atuadores(void);
void controlar_buzzer(bool estado);
void controlar_led_rgb(bool r, bool g, bool b);

#endif