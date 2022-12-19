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

static const uint I2C_SLAVE_ADDRESS = 0x17;
static const uint I2C_BAUDRATE = 100000; // 100 kHz

// For this example, we run both the master and slave from the same board.
// You'll need to wire pin GP4 to GP6 (SDA), and pin GP5 to GP7 (SCL).
static const uint I2C_SLAVE_SDA_PIN = PICO_DEFAULT_I2C_SDA_PIN; // 4
static const uint I2C_SLAVE_SCL_PIN = PICO_DEFAULT_I2C_SCL_PIN; // 5

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event, uint32_t status) {
    uint32_t* timew = (uint32_t*) &(context.mem[0xf0]);
    context.IRQstatus = status;
    sem_release(&irq_triggered);
    switch (event) {
    case I2C_SLAVE_RECEIVE:
	if (context.inx > 0) {
        // First 4 bytes is the register address in big endian
	    context.inx--;
	    ((uint8_t*) &(context.mem_address))[context.inx] = i2c_read_byte(i2c);
	// atm, we only implement a 0xff lenght regmap, so we truncate the address before use
	    if(context.inx == 0) {
                context.mem_address &= 0x000000ff;
	    }
	} else {
        // If we keep receiving bytes after the register address
	// we're in slave receiver mode
        context.mem[context.mem_address] = i2c_read_byte(i2c);
        context.mem_address++;
	}
        break;
    case I2C_SLAVE_REQUEST:
	// Slave transmitter mode
	if(context.mem_address == 0xf0){
        // Update memory mapped system tick
	    *timew =  time_us_32();
	// byte swap little endian -> big endian
	    asm( "rev %0,%0" : "+r" (*timew) );
	}
        // load from memory
        i2c_write_byte(i2c, context.mem[context.mem_address]);
        context.mem_address++;
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        context.inx=4;
        break;
    default:
        break;
    }
}

void setup_slave() {
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);

    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(i2c0, I2C_BAUDRATE);
    // configure I2C0 for slave mode
    i2c_slave_init(i2c0, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
    context.inx=4;
}
