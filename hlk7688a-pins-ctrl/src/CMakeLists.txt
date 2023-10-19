cmake_minimum_required(VERSION 3.10)
project(mt76x8-pin-ctrl C)
include_directories("./include")

file(GLOB SOURCES "src/*.c")
add_executable(${PROJECT_NAME} ${SOURCES})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)