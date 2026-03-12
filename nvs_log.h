#ifndef NVS_LOG_H
#define NVS_LOG_H

void iniciar_nvs(void);
void registrar_alarme_nvs(const char* data_hora, float temp, float umid, int luz);
void ler_logs_nvs(void);
void apagar_logs_nvs(void);

#endif