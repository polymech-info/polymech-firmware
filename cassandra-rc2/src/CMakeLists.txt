file(GLOB_RECURSE app_sources "*.cpp" "*.c")

set(INCLUDE_DIRS
    .
    "components"
    "pid"
    "proto"
    "../lib/polymech-base/src"
    "../lib/polymech-base/src/components"
    "../lib/polymech-base/src/helpers"
    "../lib/polymech-base/src/modbus"
    "../lib/polymech-base/src/net"
    "../lib/polymech-base/src/profiles"
)

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    REQUIRES arduino
) 