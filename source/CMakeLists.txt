# Add your souce files here: 
add_executable(${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/main.c
    io/blink.c
    io/io_handler.c
    io/line_following.c
    io/barcodes.c
    networking/wifi.c
    sensors/interrupt.c
    sensors/encoder.c
    sensors/ultrasonic.c
    sensors/infrared.c
    motor/motor.c
)

# Don't think there is a need to change anything here
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/networking
)

# Add required libraries
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib 
    pico_multicore
    FreeRTOS-Kernel-Heap4
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_pwm
    )

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# If using the VS code extension "Run Project" command, 
# the location of the UF2 file might match what is expected. Change the code below to match it.
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

pico_add_extra_outputs(${PROJECT_NAME})