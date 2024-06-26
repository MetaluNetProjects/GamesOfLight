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
#include "ws2812.h"
#include "fraise.h"

#define WS2812_PIN 26
#define IS_RGBW false
#define NUM_PIXELS 150

#define PIEZO_PIN 1
const uint LED_PIN = PICO_DEFAULT_LED_PIN;

absolute_time_t debouce_timer[16];
bool state[16];

void piezo_handler(uint gpio, uint32_t events)
{
    bool on = !gpio_get(gpio);
    bool final = false;

    if(!on) {
        debouce_timer[gpio] = make_timeout_time_ms(40);
    }
    else if(time_reached(debouce_timer[gpio])) {
        printf("P %d\n", gpio);
        debouce_timer[gpio] = at_the_end_of_time;
        final = true;
    }

    if(state[gpio] != final) {
        state[gpio] = final;
        printf("S %d %d\n", gpio, final);
    }
}

repeating_timer_t tick_timer;

bool tick_callback(repeating_timer_t *rt)
{
    static bool led;
    gpio_put(LED_PIN, led = !led);
    return true;
}

void setup() {
    //ws2812_setup(WS2812_PIN, IS_RGBW);
    //add_repeating_timer_ms(5, tick_callback, NULL, &tick_timer);

    for(int i = 0; i < 16; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_up(i);
        gpio_set_irq_enabled_with_callback(i, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, piezo_handler);
    }
    add_repeating_timer_ms(250, tick_callback, NULL, &tick_timer);
}

void fraise_receivebytes(const char* data, uint8_t len) {
	//if(data[0] == 1) ledPeriod = (int)data[1] * 10;
	/*char command = data[0];
	switch(command) {
		case 10: set_all(data[1] != 0); break;
		case 20: set_pixel((int)data[1], data[2], data[3], data[4]); break;
		case 21:
			pattern_play = data[1] != 0;
			pattern_rnd = data[2] != 0;
			pat = data[3];
			dir = data[4] != 0 ? 1 : -1;
			break;
		case 30: rotate_speed = (int32_t)((data[1] << 24) | (data[2]  << 16) | (data[3] << 8) | data[4]); break;
	}*/
}

void fraise_receivechars(const char *data, uint8_t len){
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
}

