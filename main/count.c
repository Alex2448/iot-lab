#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "ssd1306.h"
#include "timeMgmt.h"
#include <driver/adc.h>
#include "mqtt.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "esp_sleep.h"


#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA 

volatile uint8_t count = 0;
// #define I2C_APB_CLK_FREQ  APB_CLK_FREQ 
// #define I2C_SCLK_SRC_FLAG_FOR_NOMAL       (0)         /*!< Any one clock source that is available for the specified frequency may be choosen*/

int debounceTime = 5;
bool innerState = false;
bool outerState = false;
int OUTER_GPIO_PIN = GPIO_NUM_18;
int INNER_GPIO_PIN = GPIO_NUM_19;
int READOUTER_GPIO_PIN = GPIO_NUM_33;
int READINNER_GPIO_PIN = GPIO_NUM_34;

QueueHandle_t xQueueLeftIn, xQueueLog;

enum State{
	IDLE,
	FIRSTONLY,
	BOTH,
	SECONDONLY
};
bool innerFirst = true;

enum State state = IDLE;

void initDisplay(){
	ssd1306_128x64_i2c_init();
	ssd1306_setFixedFont(ssd1306xled_font6x8);
}


static const char *pcTextForTask1 = "Counter Task is running\r\n";
static const char *pcTextForTask2 = "Publish Task is running\r\n";

long long lastInnerMilliseconds = 0;
long long lastOuterMilliseconds = 0;
long long lastMilliseconds = 0;

void handleInnerBarrier()
{
	struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds

	if(milliseconds > lastInnerMilliseconds + debounceTime){
		if (milliseconds > lastMilliseconds + 1000){
			state = IDLE;
			innerState = false;
			outerState = false;
			char msg[256];
			sprintf(msg,"going to state: %d \ttimeout reset\n", state);
			if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			//ets_printf("going to state: %d \ttimeout reset\n", state);
			//ets_printf("----------------------------------------------\n");
		}
		innerState = !innerState;
		//ets_printf("level = %d\n",innerState);
		lastMilliseconds = milliseconds;
		lastInnerMilliseconds = milliseconds;
		if(innerState){
			if(state == IDLE){
				state = FIRSTONLY;
				innerFirst = true;
				//ets_printf("going to state: %d \tbreaching inner first\tentering\n", state);
				char msg[256];
				sprintf(msg,"going to state: %d \tbreaching inner first\tentering\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			}
			else if(state == FIRSTONLY && !innerFirst){
				state = BOTH;
				//ets_printf("going to state: %d \tbreaching inner\t\tgoing further\n", state);
				char msg[256];
				sprintf(msg,"going to state: %d \tbreaching inner\t\tgoing further\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			}
			else if(state == SECONDONLY && innerFirst){
				state = BOTH;
				//ets_printf("going to state: %d \tbreaching inner\t\tgoing back\n", state);
				char msg[256];
				sprintf(msg,"going to state: %d \tbreaching inner\t\tgoing back\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			}
		}
		else{
			if(state == BOTH && innerFirst){
				state = SECONDONLY;
				//ets_printf("going to state: %d \tunbreaching inner\tgoing further\n", state);
				char msg[256];
				sprintf(msg,"going to state: %d \tunbreaching inner\tgoing further\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			}
			else if(state == BOTH && !innerFirst){
				state = FIRSTONLY;
				//ets_printf("going to state: %d \tunbreaching inner\tgoing back\n", state);
				char msg[256];
				sprintf(msg,"going to state: %d \tunbreaching inner\tgoing back\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			}
			else if(state == SECONDONLY && !innerFirst){
				state = IDLE;
				count++;
				//ets_printf("going to state: %d \tunbreaching inner\tincreasing count\n", state);
				char msg[256];
				sprintf(msg,"going to state: %d \tunbreaching inner\tincreasing count\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
				//ets_printf("----------------------------------------------\n");
			}
			else if(state == FIRSTONLY && innerFirst){
				state = IDLE;
				//ets_printf("going to state: %d \tunbreaching inner\tgoing back\n", state);
				char msg[256];
				sprintf(msg,"going to state: %d \tunbreaching inner\tgoing back\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			}
		}
	}
}

void handleOuterBarrier()
{
	struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds

	if(milliseconds > lastOuterMilliseconds + debounceTime){
		if (milliseconds > lastMilliseconds + 1000){
			state = IDLE;
			outerState = false;
			innerState = false;
			char msg[256];
			sprintf(msg,"going to state: %d \ttimeout reset\n", state);
			if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			//ets_printf("going to state: %d \ttimeout reset\n", state);
			//ets_printf("----------------------------------------------\n");
		}
		
		outerState = !outerState;
		//ets_printf("level = %d\n",outerState);
		lastMilliseconds = milliseconds;
		lastOuterMilliseconds = milliseconds;
		if(outerState){
			if(state == IDLE){
				state = FIRSTONLY;
				innerFirst = false;
				//ets_printf("going to state: %d \tbreaching outer first\tentering\n", state);
				char msg[256];
				sprintf(msg, "going to state: %d \tbreaching outer first\tentering\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			}
			else if(state == FIRSTONLY && innerFirst){
				state = BOTH;
				//ets_printf("going to state: %d \tbreaching outer\t\tgoing further\n", state);
				char msg[256];
				sprintf(msg,"going to state: %d \tbreaching outer\t\tgoing further\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			}
			else if(state == SECONDONLY && !innerFirst){
				state = BOTH;
				//ets_printf("going to state: %d \tbreaching outer\t\tgoing back\n", state);
				char msg[256];
				sprintf(msg,"going to state: %d \tbreaching outer\t\tgoing back\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			}
		}
		else{
			if(state == BOTH && !innerFirst){
				state = SECONDONLY;
				//ets_printf("going to state: %d \tunbreaching outer\tgoing further\n", state);
				char msg[256];
				sprintf(msg,"going to state: %d \tunbreaching outer\tgoing further\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			}
			else if(state == BOTH && innerFirst){
				state = FIRSTONLY;
				//ets_printf("going to state: %d \tunbreaching outer\tgoing back\n", state);
				char msg[256];
				sprintf(msg,"going to state: %d \tunbreaching outer\tgoing back\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			}
			else if(state == SECONDONLY && innerFirst){
				state = IDLE;
				if (count > 0) count--;
				//ets_printf("going to state: %d \tunbreaching outer\tdecreasing count\n", state);
				char msg[256];
				sprintf(msg,"going to state: %d \tunbreaching outer\tdecreasing count\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
				//ets_printf("----------------------------------------------\n");
			}
			else if(state == FIRSTONLY && !innerFirst){
				state = IDLE;
				//ets_printf("going to state: %d \tunbreaching outer\tgoing back\n", state);
				char msg[256];
				sprintf(msg,"going to state: %d \tunbreaching outer\tgoing back\n", state);
				if(xQueueSendToBack(xQueueLog, ( void * )msg, (TickType_t)10 ) == pdPASS);
			}
		}
	}
}

void text(){
    char countStr[21]; //for x64 machines
    char predStr[21]; //for x64 machines
	sprintf(countStr, "%02d", count);
	sprintf(predStr, "%02d", prediction);
    //ESP_LOGI("OLED", "TRYING TO PRINT");
	time_t current_time;
	struct tm *time_info;
	char strftime_buf[9];  // space for "HH:MM:SS\0"

	time(&current_time);
	time_info = localtime(&current_time);
    strftime(strftime_buf, sizeof(strftime_buf), "%H:%M", time_info);
	ssd1306_clearScreen();
    ssd1306_printFixedN(0, 0, "G9", STYLE_NORMAL, 1);
    ssd1306_printFixedN(60, 0, strftime_buf, STYLE_NORMAL, 1);
    ssd1306_printFixedN(0, 35, countStr, STYLE_NORMAL, 2);
    ssd1306_printFixedN(75, 35, predStr, STYLE_NORMAL, 2);
}

void handleButton(){
	count++;
}

void vCounterTask (void *pvParameters){
	char *pcTaskName;
	pcTaskName = ( char * ) pvParameters;
	ets_printf(pcTaskName);
	ESP_ERROR_CHECK(gpio_set_intr_type(INNER_GPIO_PIN, GPIO_INTR_ANYEDGE));
	ESP_ERROR_CHECK(gpio_isr_handler_add(INNER_GPIO_PIN, handleInnerBarrier, (void*) INNER_GPIO_PIN));

	ESP_ERROR_CHECK(gpio_set_intr_type(OUTER_GPIO_PIN, GPIO_INTR_ANYEDGE));
	ESP_ERROR_CHECK(gpio_isr_handler_add(OUTER_GPIO_PIN, handleOuterBarrier,(void*) OUTER_GPIO_PIN)); 
	
	for(;;){
		vTaskDelay(10/portTICK_PERIOD_MS);
	}

}

void vButtonTask(){
	int Push_button_state = digitalRead(26);
	for(;;){
		// digitalRead function stores the Push button state 
		// in variable push_button_state
		// if condition checks if push button is pressed
		// if pressed LED will turn on otherwise remain off 
		if ( Push_button_state != digitalRead(26) && Push_button_state == LOW)
		{ 
			count++;
		}
		vTaskDelay(100/portTICK_PERIOD_MS);
	}

}

void vPublishLogTask(void *parameters){
	for(;;){
		time_t now = 0;
		struct tm *time_info;
		time(&now);
		time_info = localtime(&now);
		char msg[256];
		if (xQueueReceive(xQueueLog, &( msg ), ( TickType_t ) 500)  == pdPASS){
			char topic[256];
			sprintf(topic, "iotLab/%lu", now);
			esp_mqtt_client_publish(mqttlogClient, topic,msg , strlen(msg), 1,1);
		}
		
		vTaskDelay(500/portTICK_PERIOD_MS);
	}
}

void vPublishTask(void *parameters){
	
	for(;;){
		time_t now = 0;
		struct tm *time_info;
		time(&now);

		//Check if shortly after 00:00h, if yes: reset count
		char strftime_buf[6];  // space for "HH:MM\0"
		time_info = localtime(&now);
		strftime(strftime_buf, sizeof(strftime_buf), "%H:%M", time_info);

		if((now %(24*3600)) >= 0 && (now %(24*3600)) <= 400){
			count = 0;
		}

		//publish current count
		char msg[256];
		sprintf(msg, "{\"username\":\"group9\",\"device_id\":\"7\",\"count\":%d,\"timestamp\":%lu000}", count, now);
		esp_mqtt_client_publish(mqttClient, "11_7", msg, strlen(msg), 1,0);
		
		vTaskDelay(300000/portTICK_PERIOD_MS);
	}
}

void vRandomTask(void *parameters){
	ets_printf("Started random Task");
	for(;;){
		ets_printf("Random tick");
		vTaskDelay(60000/portTICK_PERIOD_MS);
	}
}

void showRoomState(){
	ets_printf("oled task running");
	for(;;){
		text();
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void IRAM_ATTR outISR(void* arg){
	//ets_printf("Interrupt OUT.\n");
}

static void IRAM_ATTR gpio_isr_handler(int pin, int edgetype) {
	//ets_printf("-------------------------HERE INTERRUPT----------------------------------\n");
}



void app_main(void){
	ets_printf("running app....\n");
        
	esp_log_level_set("BLINK", ESP_LOG_INFO);       
	
	ESP_ERROR_CHECK(gpio_set_direction(INNER_GPIO_PIN, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_set_direction(OUTER_GPIO_PIN, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_set_direction(READINNER_GPIO_PIN, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_set_direction(READOUTER_GPIO_PIN, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_set_direction(26, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_pulldown_en(INNER_GPIO_PIN));
	ESP_ERROR_CHECK(gpio_pulldown_en(OUTER_GPIO_PIN));
	ESP_ERROR_CHECK(esp_sleep_enable_gpio_wakeup());




	ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_EDGE)); //ESP_INTR_FLAG_EDGE

	/* Create a queue capable of containing 10 unsigned long values. */
    xQueueLog = xQueueCreate( 200, sizeof( char[256] ) );
    xQueueLeftIn = xQueueCreate( 10, sizeof( unsigned long ) );

    //void ssd1306_i2cInit_Embedded(int8_t scl, int8_t sda, uint8_t sa);


	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	initWifi();
	initSNTP();
	initMQTT();
	initLogMQTT();
	initDisplay();
    text();
	
	esp_mqtt_client_publish(mqttlogClient, "iotLab/startup",message , strlen(message), 1,1);

	xTaskCreate( vCounterTask, "Task 1", 1000, (void*)pcTextForTask1, 100, NULL);
	xTaskCreate( vPublishTask, "Task 2", 10000, (void*)pcTextForTask2, 100, NULL);
	xTaskCreate( showRoomState, "Task 3", 10000, NULL, 1, NULL);
	xTaskCreate( vPublishLogTask, "Task 5", 10000, NULL, 100, NULL);
	//xTaskCreate( vButtonTask, "Task 4", 1000, NULL, 50, NULL);


}
