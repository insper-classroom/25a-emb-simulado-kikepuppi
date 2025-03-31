/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/util/datetime.h"
#include <string.h>
#include "hardware/rtc.h"

const int TRIGGER_1 = 13;
const int ECHO_1 = 12;

volatile uint32_t t0_1;
volatile uint32_t tf_1;
volatile int erro_1 = 0;

const int TRIGGER_2 = 19;
const int ECHO_2 = 18;

volatile uint32_t t0_2;
volatile uint32_t tf_2;
volatile int erro_2 = 0;

volatile alarm_id_t alarm_1;
volatile alarm_id_t alarm_2;
 
 void echo_callback(uint gpio, uint32_t events){
    // if (gpio_get(ECHO_1)) {
    //     t0_1 = to_us_since_boot(get_absolute_time());
    // } else {
    //     tf_1 = to_us_since_boot(get_absolute_time());
    // }

    // if (gpio_get(ECHO_2)) {
    //     t0_2 = to_us_since_boot(get_absolute_time());
    // } else {
    //     tf_2 = to_us_since_boot(get_absolute_time());
    // }
    
    if (gpio == ECHO_1) {
        if (events & GPIO_IRQ_EDGE_RISE) {
            t0_1 = get_absolute_time();
        }
        else if (events & GPIO_IRQ_EDGE_FALL) {
            tf_1 = get_absolute_time();
            cancel_alarm(alarm_1);
        }
    }

    if (gpio == ECHO_2) {
        if (events & GPIO_IRQ_EDGE_RISE) {
            t0_2 = get_absolute_time();
        }
        else if (events & GPIO_IRQ_EDGE_FALL) {
            tf_2 = get_absolute_time();
            cancel_alarm(alarm_2);
        }
    }
 }

 
 void init_elements(){
    gpio_init(TRIGGER_1);
    gpio_set_dir(TRIGGER_1, GPIO_OUT);

    gpio_init(ECHO_1);
    gpio_set_dir(ECHO_1, GPIO_IN);

    gpio_set_irq_enabled_with_callback(ECHO_1, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &echo_callback);

    gpio_init(TRIGGER_2);
    gpio_set_dir(TRIGGER_2, GPIO_OUT);

    gpio_init(ECHO_2);
    gpio_set_dir(ECHO_2, GPIO_IN);

    gpio_set_irq_enabled_with_callback(ECHO_2, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &echo_callback);
 }
 
 int64_t alarm_callback_1(alarm_id_t id, void *user_data) {
    erro_1 = 1;
    return 0;
 }

 int64_t alarm_callback_2(alarm_id_t id, void *user_data) {
    erro_2 = 1;
    return 0;
 }
  
 int main() {
     stdio_init_all();
 
     init_elements();
 
     while (true) {

        sleep_ms(100);
        gpio_put(TRIGGER_1, 1);
        gpio_put(TRIGGER_2, 1);
        sleep_us(10);
        gpio_put(TRIGGER_1, 0);
        gpio_put(TRIGGER_2, 0);

        alarm_1 = add_alarm_in_ms(500, alarm_callback_1, NULL, false);
        alarm_2 = add_alarm_in_ms(500, alarm_callback_2, NULL, false);

        while(tf_1 == 0 && erro_1 == 0 && tf_2 == 0 && erro_2 == 0){}
        
        if(erro_1 == 1){
            printf("Sensor 1 - dist: falha\n");
        } else {
            cancel_alarm(alarm_1);
            int distancia_1 = (int) (((tf_1 - t0_1) * 0.0343) / 2.0);
            printf("Sensor 1 - dist: %d cm\n", distancia_1);
        }

        if(erro_2 == 1){
            printf("Sensor 2 - dist: falha\n");
        } else {
            cancel_alarm(alarm_2);
            int distancia_2 = (int) (((tf_2 - t0_2) * 0.0343) / 2.0);
            printf("Sensor 2 - dist: %d cm\n", distancia_2);
        }

        erro_1 = 0;
        tf_1 = 0;
        t0_1 = 0;

        erro_2 = 0;
        tf_2 = 0;
        t0_2 = 0;
 
    }
}