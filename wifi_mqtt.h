#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

// --- Dados do Adafruit IO ---
#define ADAFRUIT_USER "HASF"
#define ADAFRUIT_KEY  "aio_BlqB97OJW5kuZqXZQWJUA6bp6pPo"
// ----------------------------

void iniciar_wifi_estacao(void);
void iniciar_mqtt(void);
void publicar_mqtt(const char* feed, float valor);

#endif