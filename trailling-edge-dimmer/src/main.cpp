/**
   @brief this code is made in love by Ahmed ARIF
Embedded Devices & Telecom Engineer
   email for inquiries : arif193@gmail.com
*/
#include <iostream>
#include <cstring>
#include <vector>
#include <array>
#include <chrono>
#include <thread>

#include "mqtt.h"

/**
usefull cmds
i2cdetect -y 0
i2cset  -y 0 0x0b 0xEF
i2cget -y 0 0x0b
to config a demo test
mosquitto_pub -h broker.hivemq.com -t 'Device/DEVICE1/config' -m '{"version":1,"samplingrate":3600,"config":{"input":[{"id":0,"type":2,"map":0},{"id":1,"type":1,"map":0}]}}'
*/
Registers ioRegisters;
configStruct configstruct;

// Function to write a struct to a file
void writeToFile(const configStruct& s, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char*>(&s), sizeof(configStruct));
    file.close();
}

// Function to read a struct from a file
configStruct readFromFile(const std::string& filename) {
    configStruct s;
    std::ifstream file(filename, std::ios::binary);
    if(file.good()){
        file.read(reinterpret_cast<char*>(&s), sizeof(configStruct));
        file.close();
    }else{
        throw "no config file found, this might be the first execution of the deamon";
    }

    return s;
}

I2CDevice device;
std::mutex i2c_mutex;
std::mutex mqtt_mutex;

int helper_i2c_dimmer_init(I2CDevice &ctx){
    const std::lock_guard<std::mutex> lock(i2c_mutex);
    if ((ctx.bus = i2c_open("/dev/i2c-0")) == -1) {

        perror("Open i2c bus error");
        return -1;
    }

    ctx.addr = 0x3b & 0x3ff;
    ctx.tenbit = 0;
    ctx.delay = 10;
    ctx.flags = 0;
    ctx.page_bytes = I2C_CONFIG_BYTES_PAGE_SIZE;
    /* Set this to zero, and using i2c_ioctl_xxxx API will ignore chip internal address */
    ctx.iaddr_bytes = 0;
    return 0;
}

int helper_i2c_dimmer_close(I2CDevice &ctx){
    const std::lock_guard<std::mutex> lock(i2c_mutex);
    i2c_close(ctx.bus);
    return 0;
}

int helper_i2c_dimmer_set(I2CDevice &ctx, const Registers &data){
    const std::lock_guard<std::mutex> lock(i2c_mutex);
    ssize_t ret;
    ret = i2c_ioctl_write(&ctx, 0x0, data.data(), data.size());
    if (ret == -1 || (size_t)ret != data.size())
    {
        return -1;
    }
    return 0;
}
int helper_i2c_dimmer_get(I2CDevice &ctx, Registers &data){
    const std::lock_guard<std::mutex> lock(i2c_mutex);
    uint8_t buff[I2C_CONFIG_BYTES_PAGE_SIZE];
    ssize_t ret = i2c_ioctl_read(&ctx, 0x0, buff, I2C_CONFIG_BYTES_PAGE_SIZE);

    if (ret == -1 || (size_t)ret != I2C_CONFIG_BYTES_PAGE_SIZE)
    {
        return -1;
    }
    for (int i = 0; i < I2C_CONFIG_BYTES_PAGE_SIZE; i++) {
        data[i]=buff[i];
    }
    return 0;
}
void thread_i2c_routine(int tid) {
    std::cout << "thread i2c started"<< std::endl;
    helper_i2c_dimmer_init(device);
    helper_i2c_dimmer_set(device, configstruct.configRegisters);

    while (1)
    {
        helper_i2c_dimmer_get(device, ioRegisters);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    //TODO: this function part will never be achieved, fix it
    helper_i2c_dimmer_close(device);
}
void thread_publish_io_routine(mqtt_client *iot_client) {
    std::cout << "thread io started 2"<< std::endl;
    //    int sampling_rate = 1000;
    //    {
    //        const std::lock_guard<std::mutex> lock(mqtt_mutex);
    //        configstruct.sampling_rate = sampling_rate;
    //    }
    //    {
    //        const std::lock_guard<std::mutex> lock(mqtt_mutex);
    //        configstruct.sampling_rate = 1000;
    //    }
    MqttPublisher publisher(CLIENT_ID, BROKER_ADDRESS, MQTT_PORT);
    while (true) {
        Json::Value root;
        Json::Value inputs(Json::arrayValue);
        Json::Value outputs(Json::arrayValue);
        for (int i = 0; i < I2C_CONFIG_BYTES_PAGE_SIZE; i++) {
            inputs.append(std::to_string(ioRegisters[i]));
        }
        root["inputs"] = inputs;
        root["outputs"] = outputs;
        Json::FastWriter writer;
        std::string json_string = writer.write(root);
        iot_client->publishMessage(PUBLISH_IO_TOPIC, json_string);
        //std::cout << "message" <<root<< std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        //const std::lock_guard<std::mutex> lock(mqtt_mutex);
    }
    publisher.disconnect();
}

void thread_mqtt_routine(int tid) {
    std::cout << "thread mqtt started"<< std::endl;
    class mqtt_client *iot_client;
    std::thread thread_publish_io;
    mosqpp::lib_init();

    iot_client = new mqtt_client(CLIENT_ID, BROKER_ADDRESS, MQTT_PORT);
    auto ret = iot_client->subscribe(NULL, MQTT_TOPIC);
    //iot_client->threaded_set(true);
    thread_publish_io = std::thread(thread_publish_io_routine, iot_client);
    while (1) {
        int ret = iot_client->loop();
        if(ret){
            iot_client->reconnect();
            iot_client->subscribe(NULL, MQTT_TOPIC);
        }
    }
    //    while (true)
    //    {
    //        if (!iot_client->is_connected())
    //        {
    //            iot_client->reconnect();
    //        }

    //        int ret = client.loop();
    //        if (ret != MOSQ_ERR_SUCCESS)
    //        {
    //            std::cout << "Error in client loop. Return code: " << ret << std::endl;
    //            client.reconnect();
    //        }
    //    }
    thread_publish_io.join();
    mosqpp::lib_cleanup();
}
int main()
{
    std::cout<<"tedd boot"<<std::endl;
    try {
        configstruct =  readFromFile(CONFIG_FILE);
    }  catch (...) {
        std::cout<<"no config file found in " CONFIG_FILE ", this might be the first execution of the deamon"<<std::endl;
    }
#ifdef __mips__
    auto thread_i2c = std::thread(thread_i2c_routine, 1);
#endif
    auto thread_mqtt = std::thread(thread_mqtt_routine, 1);
    while (1)
    {
    }
#ifdef __mips__
    //thread_i2c.join();
#endif

    thread_mqtt.join();
    return 0;

}

