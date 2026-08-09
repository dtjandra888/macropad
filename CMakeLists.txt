cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD waveshare_rp2040_zero)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(macro C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)

add_executable(${PROJECT_NAME})

pico_generate_pio_header(${PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated
)


target_sources(${PROJECT_NAME}
    PRIVATE
        main.c
        font5x7.c
        oled.c
        ws2812.c
        logger.c
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include
        ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        pico_stdlib
        hardware_pio
        hardware_spi
        # tinyusb_device
        # tunyusb_board
)

pico_add_extra_outputs(${PROJECT_NAME})
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
