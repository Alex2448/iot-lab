#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include <driver/adc.h>

#define USER_NAME "9" 
#define USERID 11  //to be found in the platform 
#define DEVICEID 7  //to be found in the platform
#define SENSOR_NAME "count" 
#define TOPIC "11_7" //userID_deviceID
#define JWT_TOKEN   "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE2NTMzMTMyMTMsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMTFfNyJ9.onsrMI-GViz-9jTI3sdFcTj90E5B5KcPnyAXE49Tc3MyDjrUKj-rmXbCLthBhntyzhgTQU-GbUoNVqvPJJLzTomCsfqQexPdl-rNC0JVa3ws99au2eAyPRVBJUp0vSsHkzLJNNlU2soleJ3ZixMdVOT-Bl-N2kk4sQD5m-DFzBp1Cv5lS1iWNpg_vZE0fby0uZBEovW6T1QnXWn5Xc9DfKD1EMboBoPeK1JUSFzQHs5nI9-0hT3n-_abU2HZkYBCRXGWrhUQ-ukMNvjs5kwe8w1r-ULjx6QIPB04pmibQtOXTTPL8X5zVu20A8ymp1pQYnbbVaLCdg7y7WC0euTkZlKB9hIMF9lG2LjXt6bbpXKO_MbjpmZoQEZyRy_swzBd5A3gcNqb1rputI3aC2rZ5j7c1AEJfn6gDF78Qw2SeJFXsSElTEm4DtbSiZoQBvaImcmfEpyB_-zxlW1jwi15YRSUVQWbJxcziK3YU92WLIH-79mlOWu70QsRKznCNgXJSPLss5P1VQbgLPUX3iip6sKwwTxkrj2AGgzxXPV3w4NPeur5MjJvT3T0Tku72lcFiHPyjUuMhlMRzHDKREt58R1ba2uRiBh-G5NkolAK5nu4-41ax93kZLjVk17W2kqYkGLUwoi5GIHLzqeM6uuswNyQRgzpCXEi_-rB9OW5-JY"
#define MQTT_SERVER "138.246.236.107"

const static char *TAG = "MQTT";
esp_mqtt_client_handle_t mqttClient = NULL;
esp_mqtt_client_handle_t mqttlogClient = NULL;
EventGroupHandle_t mqtt_event_group;
volatile uint8_t prediction = 0;
const static int CONNECTED_BIT = BIT0;

static char *expected_data = NULL;
static char *actual_data = NULL;
static size_t expected_size = 0;
static size_t expected_published = 0;
static size_t actual_published = 0;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    static int msg_id = 0;
    static int actual_len = 0;
    // your_context_t *context = event->context;
    switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
		ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
		xEventGroupSetBits(mqtt_event_group, CONNECTED_BIT);
		//msg_id = esp_mqtt_client_subscribe(client, CONFIG_EXAMPLE_SUBSCIBE_TOPIC, qos_test);
        //ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
		esp_restart();
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        int length = event->data_len;
        printf("LENGTH: %d", length);

        char str[256];
        sprintf(str, event->data);
        char subbuff[256];
        memcpy( subbuff, &str[0],length );
        subbuff[length+1] = '\0';
        printf("HERE:%s\n", subbuff);
        prediction = atoi(subbuff);
        // printf(&str);
        //printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        //printf("DATA=%.*s\r\n", event->data_len, event->data);
        //printf("ID=%d, total_len=%d, data_len=%d, current_data_offset=%d\n", event->msg_id, event->total_data_len, event->data_len, event->current_data_offset);
        // if (event->topic) {
        //     actual_len = event->data_len;
        //     msg_id = event->msg_id;
        // } else {
        //     actual_len += event->data_len;
        //     // check consisency with msg_id across multiple data events for single msg
        //     if (msg_id != event->msg_id) {
        //         ESP_LOGI(TAG, "Wrong msg_id in chunked message %d != %d", msg_id, event->msg_id);
        //         abort();
        //     }
        // }
        // memcpy(actual_data + event->current_data_offset, event->data, event->data_len);
        // if (actual_len == event->total_data_len) {
        //     if (0 == memcmp(actual_data, expected_data, expected_size)) {
        //         printf("OK!");
        //         memset(actual_data, 0, expected_size);
        //         actual_published ++;
        //         if (actual_published == expected_published) {
        //             printf("Correct pattern received exactly x times\n");
        //             ESP_LOGI(TAG, "Test finished correctly!");
        //         }
        //     } else {
        //         printf("FAILED!");
        //         abort();
        //     }
        // }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
	case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
        break;
	default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

void initMQTT(void)
{
    ESP_LOGI("PROGRESS", "Initializing MQTT");

    mqtt_event_group = xEventGroupCreate();
    const esp_mqtt_client_config_t mqtt_cfg = {
        .event_handle = mqtt_event_handler,
		.host = MQTT_SERVER,
		.username = "JWT",
		.password = JWT_TOKEN,
		.port = 1883
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    mqttClient = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(mqttClient);
	xEventGroupWaitBits(mqtt_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
	ESP_LOGI(TAG, "Finished initMQTT");
}

void initLogMQTT(void)
{
    ESP_LOGI("PROGRESS", "Initializing Log MQTT");

    mqtt_event_group = xEventGroupCreate();
    const esp_mqtt_client_config_t mqtt_cfg = {
        .event_handle = mqtt_event_handler,
		.host = "broker.emqx.io",
		.username = "",
		.password = "",
		.port = 1883
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    mqttlogClient = esp_mqtt_client_init(&mqtt_cfg);
    //esp_mqtt_client_register_event(mqttlogClient, MQTT_EVENT_SUBSCRIBED, mqtt_receiving_event_handler);

    //esp_mqtt_client_subscribe(mqttlogClient, "/iotLabPrediction/#", 0);
    esp_mqtt_client_start(mqttlogClient);
    // esp_mqtt_client_subscribe(mqttlogClient, "/iotLabPrediction/#", 0);
	xEventGroupWaitBits(mqtt_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
	ESP_LOGI(TAG, "Finished initlogMQTT");
    esp_mqtt_client_subscribe(mqttlogClient, "/iotLabPrediction/#", 0);
}
