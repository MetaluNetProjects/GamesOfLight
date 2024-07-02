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
#include "fraise.h"

#define PIEZO_PIN 1
const uint LED_PIN = PICO_DEFAULT_LED_PIN;

absolute_time_t debouce_timer[16];
bool state[16];
bool last_state[16];
uint16_t encoder, last_encoder;

void piezo_handler(uint gpio, uint32_t events)
{
    bool on = !gpio_get(gpio);
    bool final = false;

    if(gpio == 0 || gpio == 1) {
        if(on) {
            if(gpio == 0) {
                if(gpio_get(1)) encoder++;
                else encoder--;
            } else {
                if(!gpio_get(0)) encoder++;
                else encoder--;
            }
        } else {
            if(gpio == 0) {
                if(!gpio_get(1)) encoder++;
                else encoder--;
            } else {
                if(gpio_get(0)) encoder++;
                else encoder--;
            }
        }
        //printf("E %d\n", encoder);
        return;
    }

    debouce_timer[gpio] = make_timeout_time_ms(10);
    //if(state[gpio] != on) {
        state[gpio] = on;
        //printf("S %d %d\n", gpio, on);
    //}
}

repeating_timer_t tick_timer;

bool tick_callback(repeating_timer_t *rt)
{
    static bool led;
    gpio_put(LED_PIN, led = !led);
    for(int i = 2; i < 16; i++) {
        if(time_reached(debouce_timer[i]) && last_state[i] != state[i]) {
            last_state[i] = state[i];
            printf("S %d %d\n", i, state[i]);
        }
    };
    if(last_encoder != encoder) {
        printf("E %d\n", encoder);
        last_encoder = encoder;
    }
    return true;
}

void setup() {
    for(int i = 0; i < 16; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_up(i);
        gpio_set_irq_enabled_with_callback(i, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, piezo_handler);
    }
    add_repeating_timer_ms(5, tick_callback, NULL, &tick_timer);
}

void fraise_receivebytes(const char* data, uint8_t len) {
}

void fraise_receivechars(const char *data, uint8_t len){
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
}

