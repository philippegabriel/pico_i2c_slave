/*
 *
 * SPDX-License-Identifier: MIT
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "ah3_32bit.h"

static uint32_t counter = 0;
static uint32_t* counterBE = (uint32_t*) &(context.mem[0xa0]);
static struct repeating_timer timer;
// Increase regmap @ 0xa0 every tick
static bool repeating_timer_callback(struct repeating_timer *t) {
    *counterBE = counter;
    // byte swap little endian -> big endian
    asm( "rev %0,%0" : "+r" (*counterBE) );
    counter++;
    return true;
}
bool timer_init() {
    printf("Initialising timer...\n");
    // Negative delay so means we will call repeating_timer_callback, and call it again
    // 500ms later regardless of how long the callback took to execute
    return add_repeating_timer_ms(-2000, repeating_timer_callback, NULL, &timer);
}
