# NOTE: not sure how I feel about this since it couples 
# svelte app to this library (albiet this isn't really a library and
# its a small amount of code)

# fsdata generation
set(WEB_DIR ${CMAKE_CURRENT_LIST_DIR}/../../web)
set(FSDATA ${CMAKE_CURRENT_LIST_DIR}/fsdata.c)

file(GLOB_RECURSE WEB_SOURCES CONFIGURE_DEPENDS
    ${WEB_DIR}/src/*
    ${WEB_DIR}/static/*
    ${WEB_DIR}/index.html
    ${WEB_DIR}/package.json
    ${WEB_DIR}/bun.lock
    ${WEB_DIR}/vite.config.*
    ${WEB_DIR}/svelte.config.*
    ${WEB_DIR}/tsconfig.json
)

add_custom_command(
    OUTPUT ${FSDATA}
    COMMAND bun run build
    COMMAND bun run generate-fsdata -- ${FSDATA}
    WORKING_DIRECTORY ${WEB_DIR}
    DEPENDS
        ${WEB_SOURCES}
        ${WEB_DIR}/scripts/generate-fsdata.ts
    COMMENT "Building web UI and generating fsdata.c"
)

# Produce httpd library
add_library(httpd
    fs.c
    ${FSDATA}
    config_api.c
    json_parser.c
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/httpd.c
)

target_include_directories(httpd PRIVATE
    .
    ${CMAKE_CURRENT_LIST_DIR}/../../include
)

target_link_libraries(httpd
    pico_stdlib
    lwip-port
)
