/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define BOARD pico

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/sync.h"
#include "ws2812.h"
#include "fraise.h"

#define WS2812_PIN 26
#define IS_RGBW false
#define NUM_PIXELS 150

#define PIN_A 0
#define PIN_B 1
uint16_t count, count_old;

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
#define LED_NO_FRAISE_COUNT 66
#define LED_NO_FRAISE_MOD 3
uint8_t led_period = LED_NO_FRAISE_COUNT;
uint8_t led_mod = LED_NO_FRAISE_MOD;
uint8_t led_watchdog_count = 0;
absolute_time_t debouce_timer[16];
bool state[16];

uint16_t lastcount;
absolute_time_t lasttime, last_irq_time;
float speed_measured, speed_lp1, speed_lp2;
float lp_f = 0.66;

float speed_process()
{
    uint32_t status = save_and_disable_interrupts();
    int16_t dx = count - lastcount;
    lastcount = count;
    int dt = absolute_time_diff_us(lasttime, last_irq_time);
    lasttime = last_irq_time;
    restore_interrupts(status);
    float speed;
    if(!dt) speed = 0;
    else speed = (200000.0 * dx) / dt;
    //return speed;
    speed_lp1 += (speed - speed_lp1) * lp_f;
    speed_lp2 += (speed_lp1 - speed_lp2) * lp_f;
    return speed_lp2;
}

void gpio_handler(uint gpio, uint32_t events)
{
    bool on = gpio_get(gpio);
    //bool final = false;

    if(gpio == PIN_A) {
        if(on) {
            if(gpio_get(PIN_B)) count++; else count--;
        } else {
            if(!gpio_get(PIN_B)) count++; else count--;
        }
        //printf("C %d\n", count);
    } else if(gpio == PIN_B) {
        if(on) {
            if(!gpio_get(PIN_A)) count++; else count--;
        } else {
            if(gpio_get(PIN_A)) count++; else count--;
        }
        //printf("C %d\n", count);
    } else return;
    last_irq_time = get_absolute_time();
}

repeating_timer_t tick_timer, led_timer;

bool tick_callback(repeating_timer_t *rt)
{
    static int led_count = 0;
    static int16_t last_speed;
    int16_t speed = (int16_t)speed_process();
    bool send = false;
    if(speed != last_speed) {
        //printf("S %d\n", speed);
        last_speed = speed;
        send = true;
    }

    if(count_old != count) {
        send = true;
        count_old = count;
        //printf("C %d\n", count);
        gpio_put(LED_PIN, (count / 100) % 2);
    } else {
        led_count++;
        gpio_put(LED_PIN, ((led_count / led_period) % led_mod) == 0);
        if(led_watchdog_count > 0) led_watchdog_count--;
        else {
            led_period = LED_NO_FRAISE_COUNT;
            led_mod = LED_NO_FRAISE_MOD;
        }
    }
    
    if(send) {
        fraise_put_init();
        fraise_put_uint8(1);
        fraise_put_uint16(count);
        fraise_put_int16(speed);
        fraise_put_send();
    }
    return true;
}

bool led_callback(repeating_timer_t *rt)
{
    //static bool led;
    //gpio_put(LED_PIN, led = !led);
    return true;
}

void setup() {
    //ws2812_setup(WS2812_PIN, IS_RGBW);
    add_repeating_timer_ms(20, tick_callback, NULL, &tick_timer);

    for(int i = 0; i < 16; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_up(i);
        gpio_set_irq_enabled_with_callback(i, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, gpio_handler);
    }
    add_repeating_timer_ms(250, led_callback, NULL, &led_timer);
}

void fraise_receivebytes(const char* data, uint8_t len) {
	uint8_t command = fraise_get_uint8();
	if(command == 1) {
	    led_period = data[1];
	    led_mod = data[2];
	    led_watchdog_count = 120;
    }
    else if(command == 2) lp_f = fraise_get_uint16() / 65535.0;
}

void fraise_receivechars(const char *data, uint8_t len){
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
}

