cmake_minimum_required(VERSION 2.6)
project(trailling-edge-dimmer)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MQTT REQUIRED libmosquittopp)
pkg_check_modules(JSON REQUIRED jsoncpp)

link_libraries(-li2c-amaork -lpthread)

add_executable(tedd main.cpp mqtt.cpp)
target_link_libraries(tedd ${MQTT_LIBRARIES} ${JSON_LIBRARIES})

INSTALL(TARGETS tedd RUNTIME DESTINATION /usr/bin/)
