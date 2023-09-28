#include "mqtt.h"


std::vector<std::string> split(const std::string& s, char seperator)
{
    std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );

        output.push_back(substring);

        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

    return output;
}

mqtt_client::mqtt_client(const char *id, const char *host, int port) : mosquittopp(id)
{
    int keepalive = DEFAULT_KEEP_ALIVE;
    connect(host, port, keepalive);
}

mqtt_client::~mqtt_client()
{
}

void mqtt_client::on_connect(int rc)
{
    if (!rc)
    {
#ifdef DEBUG
        std::cout << "Connected - code " << rc << std::endl;
#endif
    }
}

void mqtt_client::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
#ifdef DEBUG
    std::cout << "Subscription succeeded." << std::endl;
#endif
}

#include <bitset>

void printBinary(int n) {
    std::bitset<8> binary(n);
    std::cout << binary << std::endl;
}
int parseJSON(std::string json_string){
    std::cout << "parsing config payload"<< std::endl;

    Registers inputConf;
    std::fill(std::begin(inputConf), std::end(inputConf), 0);
    Json::Value root;
     Json::CharReaderBuilder builder;
     JSONCPP_STRING err;
     const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
     if (!reader->parse(json_string.c_str(), json_string.c_str() + json_string.length(), &root, &err)) {
       std::cerr << "Error parsing JSON: " << err << std::endl;
       return 1;
     }
     int version = root["version"].asInt();
     int sampling_rate = root["samplingrate"].asInt();

     const Json::Value input_array = root["config"]["input"];
     if (!input_array.isArray()) {
       std::cerr << "Error: 'input' is not an array." << std::endl;
       return 1;
     }
     inputConf[0]=I2C_CONFIG_REGISTER_SET;
     for (const auto &input : input_array) {
       int id = input["id"].asInt();
       int type = input["type"].asInt();
       int map = input["map"].asInt();
       inputConf[id+1] |= ((uint8_t)type)<<3;
       printBinary((int)inputConf[id+1]);
       inputConf[id+1] |= (uint8_t)map;
       printBinary((int)inputConf[id+1]);

       std::cout << "Input ID: " << id << ", Type: " << type << ", Map: " << map <<"=>"<<(int)inputConf[id+1]<< std::endl;
     }
    configstruct.configRegisters = inputConf;
    {
        const std::lock_guard<std::mutex> lock(mqtt_mutex);
        configstruct.sampling_rate = sampling_rate;
    }
    writeToFile(configstruct,CONFIG_FILE);
#ifdef __mips__
    return helper_i2c_dimmer_set(device, inputConf);
#endif
     return 0;
}
//void mqtt_client::publishMessage(const std::string &topic, const std::string &message) {
//  int ret = publish(NULL, topic.c_str(), message.size(), message.c_str());
//  if (ret != MOSQ_ERR_SUCCESS) {
//    std::cerr << "Error publishing message: " << ret << std::endl;
//  }
//}
void mqtt_client::on_message(const struct mosquitto_message *message)
{
    std::string topic(message->topic);
    std::string payload((char*)message->payload);
    if(topic != PUBLISH_IO_TOPIC && topic != PUBLISH_IO_TOPIC)
    {
        std::cout << "new upcoming message on topic "<<message->topic << std:: endl;
        auto splits = split(topic,'/');
        if(splits.size()<3){
            goto on_message_error_topic;
        }
        if(splits[2] == "config"){
            parseJSON(payload);
            //TODO : check return value and string error also;
        }
        if(splits[2] == "set"){
            if(splits.size()<4){
                goto on_message_error_topic;
            }
            int outputPin=-1;
            int outputValue=-1;
            try {
                outputPin = std::stoi(splits[3]);
                outputValue = std::stoi(payload);
            } catch (...) {
                goto on_message_error_topic;
            }

            if(outputPin<0 || 8 < outputPin || outputValue<0 || 255 < outputValue){
                goto on_message_error_topic;
            }
            std::cout << "set the physical pin : "<< outputPin << " with the value : "<<outputValue<<std:: endl;
            Registers inputConf;
            inputConf[0]=I2C_OUTPUT_REGISTERS_SET;
            inputConf[1]=outputValue;
            inputConf[2]=outputPin;
            int ret = helper_i2c_dimmer_set(device, inputConf);
        }
    }
    return;
on_message_error_topic:
    std::cerr << "topic format is not OK : " <<topic<<std:: endl;
    return;
}
