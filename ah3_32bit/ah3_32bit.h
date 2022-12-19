/*
 *
 * SPDX-License-Identifier: MIT
 */
#include "pico/sem.h"


// The slave implements a 256 byte memory. To write a series of bytes, the master first
// writes the memory address, followed by the data. The address is automatically incremented
// for each byte transferred, looping back to 0 upon reaching the end. Reading is done
// sequentially from the current memory address.
extern struct
{
// mem is 32 bit big endian
    uint8_t mem[256];
    uint32_t mem_address;
    uint8_t inx;
    uint32_t IRQstatus;
} context;
// semaphore to signal main to output debug info
extern struct semaphore irq_triggered;

