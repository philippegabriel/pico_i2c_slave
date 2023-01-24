#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "ah3_32bit.h"
#include "sine_wave.h"

void init_dma(){
    int ctrl_chan = dma_claim_unused_channel(true);
    int data_chan = dma_claim_unused_channel(true);

    // Setup the control channel
    dma_channel_config ctrl_chan_cfg = dma_channel_get_default_config(ctrl_chan);   // default configs
    channel_config_set_transfer_data_size(&ctrl_chan_cfg, DMA_SIZE_32);             // 32-bit txfers
    channel_config_set_read_increment(&ctrl_chan_cfg, false);                       // no read incrementing
    channel_config_set_write_increment(&ctrl_chan_cfg, false);                      // no write incrementing
    channel_config_set_chain_to(&ctrl_chan_cfg, data_chan);                         // chain to data channel

    uint32_t * address_pointer = sine_wave;
    dma_channel_configure(
        ctrl_chan,                          // Channel to be configured
        &ctrl_chan_cfg,                                 // The configuration we just created
        &dma_hw->ch[data_chan].read_addr,   // Write address (data channel read address)
        &address_pointer,                         // Read address (POINTER TO AN ADDRESS)
        1,                                  // Number of transfers
        false                               // Don't start immediately
    );

    // Setup the data channel
    dma_channel_config data_chan_cfg = dma_channel_get_default_config(data_chan);  // Default configs
    channel_config_set_transfer_data_size(&data_chan_cfg, DMA_SIZE_32);            // 32-bit txfers
    channel_config_set_read_increment(&data_chan_cfg, true);                       // yes read incrementing
    channel_config_set_write_increment(&data_chan_cfg, false);                     // no write incrementing
    // (X/Y)*sys_clk, where X is the first 16 bytes and Y is the second
    // sys_clk is 125 MHz unless changed in code. Configured to ~44 kHz
    dma_timer_set_fraction(0, 1, 0xffff) ;
    // 0x3b means timer0 (see SDK manual)
    channel_config_set_dreq(&data_chan_cfg, 0x3b);                                 // DREQ paced by timer 0
    // chain to the controller DMA channel
    channel_config_set_chain_to(&data_chan_cfg, ctrl_chan);                        // Chain to control channel


    dma_channel_configure(
        data_chan,                  // Channel to be configured
        &data_chan_cfg,                        // The configuration we just created
        memptr(0xb0),               // write address 
        sine_wave,                   // The initial read address
        SINE_WAVE_TABLE_LEN,         // Number of transfers
        false                       // Don't start immediately.
    );

    dma_channel_start(ctrl_chan);
}	
