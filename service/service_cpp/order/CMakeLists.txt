cmake_minimum_required(VERSION 3.15)
project(OrderService VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(Protobuf REQUIRED IMPORTED_TARGET protobuf)
pkg_check_modules(gRPC REQUIRED IMPORTED_TARGET grpc++)
pkg_check_modules(SPDLOG REQUIRED IMPORTED_TARGET spdlog)

find_library(HIREDIS hiredis REQUIRED)
find_library(MYSQL_CLIENT mysqlclient REQUIRED)

set(PROTO_DIR ${CMAKE_SOURCE_DIR}/../../../proto)

# 源文件列表（不需要列出头文件，CMake会根据#include自动找到）
set(ORDER_SOURCES
    main.cpp
    ${PROTO_DIR}/order.pb.cc
    ${PROTO_DIR}/order.grpc.pb.cc
)

# 创建可执行文件（头文件由target_include_directories指定，CMake自动解析）
add_executable(order_service ${ORDER_SOURCES})

target_include_directories(order_service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${PROTO_DIR}
    /usr/include/hiredis
    /usr/include/mysql
)

target_link_options(order_service PRIVATE
    -pthread
)

target_link_libraries(order_service PRIVATE
    PkgConfig::Protobuf
    PkgConfig::gRPC
    PkgConfig::SPDLOG
    Threads::Threads
    ${HIREDIS}
    ${MYSQL_CLIENT}
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(order_service PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -Wno-pedantic
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(order_service PRIVATE
        /W4
        /wd4100
    )
endif()

set_target_properties(order_service PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

install(TARGETS order_service
    RUNTIME DESTINATION bin
)
