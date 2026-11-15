# Host-side unit tests for the firmware's own logic. Standalone project:
# builds with the host compiler, not the Pico SDK.
#   cmake -S tests -B build-tests && cmake --build build-tests
#   ctest --test-dir build-tests --output-on-failure
cmake_minimum_required(VERSION 3.14)
project(macropad_tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.6.1
)
FetchContent_MakeAvailable(unity)

enable_testing()

set(ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
set(SANITIZE -fsanitize=address,undefined -fno-omit-frame-pointer)

# add_unit_test(<name> <sources...>): one executable per test file
function(add_unit_test name)
    add_executable(${name} ${name}.c ${ARGN})
    # stubs/ comes first so it shadows the Pico SDK / TinyUSB / lwIP headers
    target_include_directories(${name} PRIVATE
        stubs
        fakes
        ${ROOT}/include
        ${ROOT}/config
        ${ROOT}/lib/httpd
    )
    target_compile_options(${name} PRIVATE -Wall -Wextra -g ${SANITIZE})
    target_link_options(${name} PRIVATE ${SANITIZE})
    target_link_libraries(${name} PRIVATE unity)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_unit_test(test_hid_json ${ROOT}/hid_json.c)
add_unit_test(test_config ${ROOT}/config.c fakes/fake_flash.c)
add_unit_test(test_json_parser ${ROOT}/lib/httpd/json_parser.c ${ROOT}/hid_json.c)
add_unit_test(test_config_api ${ROOT}/lib/httpd/config_api.c ${ROOT}/lib/httpd/json_parser.c ${ROOT}/hid_json.c ${ROOT}/config.c fakes/fake_flash.c fakes/fake_lwip.c fakes/fake_log.c)
add_unit_test(test_hid_report ${ROOT}/hid_report.c fakes/fake_tusb.c)
add_unit_test(test_macro ${ROOT}/macro.c fakes/fake_time.c fakes/fake_hid_report.c fakes/fake_log.c)
add_unit_test(test_key_display ${ROOT}/key_display.c fakes/fake_oled.c fakes/fake_key.c)
add_unit_test(test_key ${ROOT}/key.c fakes/fake_gpio.c fakes/fake_time.c fakes/fake_hid_report.c fakes/fake_macro.c)
