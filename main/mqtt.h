#ifndef MQTT_H
#define MQTT_H

#include "mqtt_client.h"

void initMQTT(void);
void initLogMQTT(void);

extern esp_mqtt_client_handle_t mqttClient;
extern esp_mqtt_client_handle_t mqttlogClient;
extern EventGroupHandle_t mqtt_event_group;
extern volatile uint8_t prediction;


#endif
