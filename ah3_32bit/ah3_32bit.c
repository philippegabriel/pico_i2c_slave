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
#include "i2c_slave_handler.h"
#include "timer_handler.h"


// The slave implements a 256 byte memory. To write a series of bytes, the master first
// writes the memory address, followed by the data. The address is automatically incremented
// for each byte transferred, looping back to 0 upon reaching the end. Reading is done
// sequentially from the current memory address.
struct
{
// mem is 32 bit big endian
    uint8_t mem[256];
    uint32_t mem_address;
    uint8_t inx;
    uint32_t IRQstatus;
} context;
// semaphore to signal main to output debug info
struct semaphore irq_triggered;


int main() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
    stdio_init_all();
    puts("\nI2C slave example");
    sem_init(&irq_triggered, 0, 1);
    setup_slave();
    timer_init();
    sleep_ms(3000);	
    gpio_put(LED_PIN, 0);
    printf("I2c Slave running...\n");
    while (true) {
	sem_acquire_blocking(&irq_triggered);
        printf("IRQ triggered...%#010lx,%u,%#010lx\n", context.IRQstatus, context.inx, context.mem_address);
    }
    return 0;
}
