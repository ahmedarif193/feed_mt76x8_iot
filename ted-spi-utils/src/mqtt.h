#ifndef MQTT_H
#define MQTT_H

#define DEBUG

#include <iostream>
#include <string>
#include <mutex>
#include <fstream>
#include <string>
#include <functional>
#include <fstream>

#include <mosquittopp.h>
#include "json/json.h"
#include "i2c/i2c.h"

#define CLIENT_ID "CLIENT_ID"
#define BROKER_ADDRESS "broker.hivemq.com"
#define MQTT_PORT 1883
#define DEFAULT_KEEP_ALIVE 60

#define DEVICE_ID "DEVICE1"
#define MQTT_TOPIC "Device/" DEVICE_ID "/#"
#define PUBLISH_IO_TOPIC "Device/" DEVICE_ID "/io"
#define PUBLISH_COUNTER_TOPIC "Device/" DEVICE_ID "/counter"

#define CONFIG_FILE "/tmp/teddconfig.bin"


#define I2C_CONFIG_BYTES_PAGE_SIZE 8
using Registers = std::array<uint8_t,I2C_CONFIG_BYTES_PAGE_SIZE>;
typedef struct __attribute__((__packed__))  configStruct {
  Registers configRegisters;
  int sampling_rate;
}configStruct;

#define I2C_CONFIG_REGISTER_SET 0x52
#define I2C_OUTPUT_REGISTERS_SET 0x53
extern  I2CDevice device;
extern int sampling_rate;
extern Registers ioRegisters;
extern configStruct configstruct;

extern std::mutex i2c_mutex;
extern std::mutex mqtt_mutex;

// Function to write a struct to a file
void writeToFile(const configStruct& s, const std::string& filename);

// Function to read a struct from a file
configStruct readFromFile(const std::string& filename);

int helper_i2c_dimmer_set(I2CDevice &ctx, const Registers &data);

class mqtt_client : public mosqpp::mosquittopp
{
public:
    mqtt_client (const char *id, const char *host, int port);
    ~mqtt_client();

//    void publishMessage(std::string topic, std::string value);
    void on_connect(int rc);
    void on_message(const struct mosquitto_message *message);
    void on_subscribe(int mid, int qos_count, const int *granted_qos);
    void publishMessage(const std::string &topic, const std::string &message) {
      int ret = publish(NULL, topic.c_str(), message.size(), message.c_str());
      if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Error publishing message: " << ret << std::endl;
      }
    }
};

class MqttPublisher : public mosqpp::mosquittopp {
public:
  MqttPublisher(const char *id, const char *host, int port) : mosqpp::mosquittopp(id) {
    connect(host, port, 60);
  }

  void publishMessage(const std::string &topic, const std::string &message) {
    int ret = publish(NULL, topic.c_str(), message.size(), message.c_str());
    if (ret != MOSQ_ERR_SUCCESS) {
      std::cerr << "Error publishing message: " << ret << std::endl;
    }
  }
};

#endif //MQTT_H
