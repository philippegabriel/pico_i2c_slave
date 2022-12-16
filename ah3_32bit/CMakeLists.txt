add_executable(ah3_32bit ah3_32bit.c)

pico_enable_stdio_uart(ah3_32bit 0)
pico_enable_stdio_usb(ah3_32bit 1)

pico_add_extra_outputs(ah3_32bit)

target_compile_options(ah3_32bit PRIVATE -Wall)

target_link_libraries(ah3_32bit i2c_slave pico_stdlib)
