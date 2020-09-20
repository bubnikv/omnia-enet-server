cmake_minimum_required(VERSION 3.0)
project(omnia-enet-server)

add_subdirectory(enet)
#find_package(enet)

add_executable(omnia-enet-server omnia-enet-server.cpp cat.cpp cat.h enet)
target_include_directories(omnia-enet-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/enet/include)
target_compile_options(omnia-enet-server PUBLIC -Wall -Wfloat-conversion)
target_link_libraries(omnia-enet-server PUBLIC -lusb-1.0 ${CMAKE_CURRENT_BINARY_DIR}/enet/libenet.a)

