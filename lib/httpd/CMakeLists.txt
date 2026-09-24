# NOTE: not sure how I feel about this since it couples 
# svelte app to this library (albiet this isn't really a library and
# its a small amount of code)

# fsdata generation
set(WEB_DIR ${CMAKE_CURRENT_LIST_DIR}/../../web)
set(FSDATA ${CMAKE_CURRENT_LIST_DIR}/fsdata.c)

add_custom_command(
    OUTPUT ${FSDATA}
    COMMAND bun run build
    COMMAND bun run generate-fsdata
    WORKING_DIRECTORY ${WEB_DIR}
    DEPENDS
        ${WEB_DIR}/package.json
        ${WEB_DIR}/bun.lock
        ${WEB_DIR}/scripts/generate-fsdata.ts
    COMMENT "Building web UI and generating fsdata.c"
)

# produce httpd library
add_library(httpd
    fs.c
    ${FSDATA}
    config_api.c
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/httpd.c
)

target_include_directories(httpd INTERFACE
    .
)

target_link_libraries(httpd
    pico_stdlib
    lwip-port
    pico_lwip
)
