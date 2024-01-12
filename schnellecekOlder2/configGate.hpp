#ifndef CONFIG_GATE_HPP
#define CONFIG_GATE_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

class ConfigGate {
public:
    ConfigGate(const std::string& json_file_path);

    int parseConfig();

private:
    std::ifstream json_file;
    rapidjson::IStreamWrapper json_stream;
    rapidjson::Document data_dict;
    std::vector<std::string> list_front_cam;
    std::vector<std::string> list_side_cam;
    std::vector<std::string> list_gate;
};

#endif // CONFIG_GATE_HPP
