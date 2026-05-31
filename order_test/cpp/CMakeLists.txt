cmake_minimum_required(VERSION 3.15)
project(OrderServiceTest VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

# 【修正项 1】：路径从回退 1 级 (../proto) 改为回退 2 级 (../../proto)
include_directories(${CMAKE_SOURCE_DIR}/../../proto)

set(SOURCES
    server.cpp
    # 【修正项 2】：修正数据序列化源码文件的相对路径
    ../../proto/order.pb.cc
    # 【修正项 3】：强制补齐 gRPC 通信存根源码文件，否则引发 LNK2019 链接错误
    ../../proto/order.grpc.pb.cc
)

add_executable(order_server_test ${SOURCES})

target_link_libraries(order_server_test
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

target_include_directories(order_server_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    # 【修正项 4】：同步修正 Target 的头文件包含路径
    ${CMAKE_SOURCE_DIR}/../../proto
)