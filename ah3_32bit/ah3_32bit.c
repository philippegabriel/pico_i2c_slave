/*
 *
 * SPDX-License-Identifier: MIT
 */

#include <i2c_fifo.h>
#include <i2c_slave.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico/sem.h"
#include "ah3_32bit.h"
#include "i2c_slave_handler.h"
#include "timer_handler.h"


context_struct context;
#ifdef DEBUG
// semaphore to signal main to output debug info
struct semaphore irq_triggered;
#endif

int main() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
    stdio_init_all();
    puts("\nI2C slave example");
#ifdef DEBUG
    sem_init(&irq_triggered, 0, 1);
#endif    
    setup_slave();
    timer_init();
    sleep_ms(3000);	
    gpio_put(LED_PIN, 0);
    printf("I2c Slave running...\n");
    while (true) {
#ifdef DEBUG	    
	sem_acquire_blocking(&irq_triggered);
        printf("IRQ triggered...%#010lx,%u,%#010lx\n", context.IRQstatus, context.inx, context.mem_address);
#else
        tight_loop_contents();
#endif	
    }
    return 0;
}
