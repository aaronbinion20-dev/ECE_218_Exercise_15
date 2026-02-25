#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "driver/gpio.h"


#define LOOP_DELAY_MS 500     // Sampling time (ms)
#define ECHO GPIO_NUM_12
#define TRIG GPIO_NUM_11
#define OUT_OF_RANGE_SHORT 116
#define OUT_OF_RANGE_LONG 23200
int64_t start_time = 0;
int64_t end_time = 0 ;

esp_timer_handle_t oneshot_timer;   // One-shot timer handle
uint64_t echo_pulse_time = 0;       // Pulse time calculated in echo ISR

void IRAM_ATTR oneshot_timer_handler(void* arg) {
    gpio_set_level(TRIG, 0);
}

void IRAM_ATTR echo_isr_handler(void*arg) {
    if (gpio_get_level(ECHO)) {
        start_time = esp_timer_get_time();
    }
    else {
        end_time = esp_timer_get_time();
        echo_pulse_time = end_time - start_time;
    } 
}

void hc_sr04_init() {
gpio_reset_pin(TRIG);
gpio_set_direction(TRIG, GPIO_MODE_OUTPUT);
gpio_set_level(TRIG, 0);

gpio_reset_pin(ECHO);
gpio_set_direction(ECHO, GPIO_MODE_INPUT);
gpio_set_intr_type(ECHO, GPIO_INTR_ANYEDGE);

const esp_timer_create_args_t oneshot_timer_args = {
    .callback = &oneshot_timer_handler,
    .name = "one-shot"
};

esp_timer_create(&oneshot_timer_args, &oneshot_timer);

gpio_install_isr_service(0); 
gpio_intr_enable(ECHO);
gpio_isr_handler_add(ECHO, echo_isr_handler, NULL); 
}

void app_main(void)
{
    hc_sr04_init();
    float distance;
    while(1)
    {
        gpio_set_level(TRIG, 1);
        esp_timer_start_once(oneshot_timer, 10);
        vTaskDelay(4 / portTICK_PERIOD_MS);
        if (echo_pulse_time > OUT_OF_RANGE_LONG) {
    printf("Object too far.\n");
}
else if (echo_pulse_time < OUT_OF_RANGE_SHORT) {
    printf("Object too close.\n");
}
else {
    distance = echo_pulse_time / 58.3;
    printf("%.2f cm\n", distance);
    }  
    vTaskDelay(LOOP_DELAY_MS / portTICK_PERIOD_MS);
}
}