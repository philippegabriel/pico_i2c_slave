/*
 *
 * SPDX-License-Identifier: MIT
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "ah3_32bit.h"
#include "sine_wave.h"

// timer period in milliseconds
#define TIMER_PERIOD 10
static uint32_t counter = 0;
static uint32_t* counterBE = (uint32_t*) &(context.mem[0xa0]);
static uint32_t* sine_sample = (uint32_t*) &(context.mem[0xb0]);
static struct repeating_timer timer;
// Increase regmap @ 0xa0 every tick
static bool repeating_timer_callback(struct repeating_timer *t) {
    *counterBE = counter;
    // byte swap little endian -> big endian
    asm( "rev %0,%0" : "+r" (*counterBE) );
    // update sine function value 
    // counter mod SINE_WAVE_TABLE_LEN
    *sine_sample = sine_wave[counter & 0x7ff];
    counter++;
    return true;
}
bool timer_init() {
    printf("Initialising timer...\n");
    // Negative delay so means we will call repeating_timer_callback, and call it again
    return add_repeating_timer_ms(-TIMER_PERIOD, repeating_timer_callback, NULL, &timer);
}
