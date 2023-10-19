cmake_minimum_required(VERSION 3.10)

project(MT76x8_GPIO_Control)

add_executable(mt76x8_gpio mt76x8_gpio.c)

target_link_libraries(mt76x8_gpio m)

install(TARGETS mt76x8_gpio DESTINATION bin)
