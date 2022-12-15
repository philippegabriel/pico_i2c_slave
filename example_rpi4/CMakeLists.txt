add_executable(example_rpi4 example_rpi4.c)

pico_enable_stdio_uart(example_rpi4 0)
pico_enable_stdio_usb(example_rpi4 1)

pico_add_extra_outputs(example_rpi4)

target_compile_options(example_rpi4 PRIVATE -Wall)

target_link_libraries(example_rpi4 i2c_slave pico_stdlib)
