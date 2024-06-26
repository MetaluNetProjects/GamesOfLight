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

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

absolute_time_t debouce_timer[16];
bool state[16];
bool last_state[16];

void buttons_handler(uint gpio, uint32_t events)
{
    bool on = !gpio_get(gpio);

    debouce_timer[gpio] = make_timeout_time_ms(20);
    state[gpio] = on;
}

repeating_timer_t tick_timer;

bool tick_callback(repeating_timer_t *rt)
{
    static bool led;
    gpio_put(LED_PIN, led = !led);
    for(int i = 0; i < 16; i++) {
        if(time_reached(debouce_timer[i]) && last_state[i] != state[i]) {
            last_state[i] = state[i];
            printf("S %d %d\n", i, state[i]);
        }
    };
    return true;
}

void setup() {
    for(int i = 0; i < 16; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_up(i);
        gpio_set_irq_enabled_with_callback(i, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, buttons_handler);
    }
    add_repeating_timer_ms(10, tick_callback, NULL, &tick_timer);
}

void fraise_receivebytes(const char* data, uint8_t len) {
}

void fraise_receivechars(const char *data, uint8_t len){
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
}

