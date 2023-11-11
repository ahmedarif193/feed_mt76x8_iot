cmake_minimum_required(VERSION 2.6)
project(ted-spi-utils)

# Set the build type to Release
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(PkgConfig REQUIRED)

link_libraries(-lpthread)

add_executable(ted-spi-utils main.cpp cs5460.cpp arduino_wrapper.cpp)
target_link_libraries(ted-spi-utils ${MQTT_LIBRARIES} ${JSON_LIBRARIES} ${GPIOD_LIBRARIES})

INSTALL(TARGETS ted-spi-utils RUNTIME DESTINATION /usr/bin/)
