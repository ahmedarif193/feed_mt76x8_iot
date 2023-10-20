cmake_minimum_required(VERSION 2.6)
project(trailling-edge-dimmer)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MQTT REQUIRED libmosquittopp)
pkg_check_modules(JSON REQUIRED jsoncpp)
pkg_check_modules(GPIOD REQUIRED libgpiod)

link_libraries(-lpthread)

include_directories(${MQTT_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS} ${GPIOD_INCLUDE_DIRS})
link_directories(${MQTT_LIBRARY_DIRS} ${JSON_LIBRARY_DIRS} ${GPIOD_LIBRARY_DIRS})

add_executable(tedd main.cpp)
target_link_libraries(tedd ${MQTT_LIBRARIES} ${JSON_LIBRARIES} ${GPIOD_LIBRARIES})

INSTALL(TARGETS tedd RUNTIME DESTINATION /usr/bin/)
