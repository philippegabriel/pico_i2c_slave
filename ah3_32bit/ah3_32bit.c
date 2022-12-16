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

static const uint I2C_SLAVE_ADDRESS = 0x17;
static const uint I2C_BAUDRATE = 100000; // 100 kHz

// For this example, we run both the master and slave from the same board.
// You'll need to wire pin GP4 to GP6 (SDA), and pin GP5 to GP7 (SCL).
static const uint I2C_SLAVE_SDA_PIN = PICO_DEFAULT_I2C_SDA_PIN; // 4
static const uint I2C_SLAVE_SCL_PIN = PICO_DEFAULT_I2C_SCL_PIN; // 5

// The slave implements a 256 byte memory. To write a series of bytes, the master first
// writes the memory address, followed by the data. The address is automatically incremented
// for each byte transferred, looping back to 0 upon reaching the end. Reading is done
// sequentially from the current memory address.
static struct
{
// mem is 32 bit big endian
    uint8_t mem[256];
    uint8_t mem_address;
    uint8_t inx;
} context;
// semaphore to signal main to output debug info
static struct semaphore irq_triggered;
uint32_t IRQstatus;


// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event, uint32_t status) {
    uint8_t timeb[4];
    IRQstatus = status;
    sem_release(&irq_triggered);
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
	if (context.inx == 3) {
        // LSB of 32 address
	    context.mem_address = i2c_read_byte(i2c);
	    context.inx++;
	    break;
	}
	if (context.inx < 3) {
        // bytes [1:3] of address, we cheat and discard them
            i2c_read_byte(i2c);
            context.inx++;
	    break;
	}
        // save into memory
        context.mem[context.mem_address] = i2c_read_byte(i2c);
        context.mem_address++;
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data
	if(context.mem_address == 0xf0){
        // Update memory mapped system tick
	    *((uint32_t*) timeb) =  time_us_32();
	//TODO: find some system/ARM intrinsyc to do the byte swap
	    context.mem[0xf0] = timeb[3];
	    context.mem[0xf1] = timeb[2];
	    context.mem[0xf2] = timeb[1];
	    context.mem[0xf3] = timeb[0];
	}
        // load from memory
        i2c_write_byte(i2c, context.mem[context.mem_address]);
        context.mem_address++;
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        context.inx=0;
        break;
    default:
        break;
    }
}

static void setup_slave() {
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);

    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(i2c0, I2C_BAUDRATE);
    // configure I2C0 for slave mode
    i2c_slave_init(i2c0, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
    context.inx=0;
}
int main() {
    stdio_init_all();
    sleep_ms(3000);	
    sem_init(&irq_triggered, 0, 1);
    puts("\nI2C slave example");
    setup_slave();
    printf("I2c Slave running...");
    while (true) {
	sem_acquire_blocking(&irq_triggered);
        printf("IRQ triggered...%#010lx,%u,%u\n", IRQstatus, context.inx, context.mem_address);
    }
    return 0;
}
