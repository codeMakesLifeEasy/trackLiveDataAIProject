#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "rapidjson/document.h"
const std::string jsonFilePath = "gate.json";

class CameraDataLoader {
public:
    void loadJsonData(const std::string& jsonFilePath) {
        std::ifstream jsonFile(jsonFilePath);
        if (!jsonFile.is_open()) {
            std::cerr << "Error opening JSON file: " << jsonFilePath << std::endl;
            return;
        }

        std::stringstream buffer;
        buffer << jsonFile.rdbuf();
        std::string jsonDataStr = buffer.str();

        document.Parse(jsonDataStr.c_str());

        if (document.HasParseError() || !document.IsObject()) {
            std::cerr << "Error parsing JSON file: " << jsonFilePath << std::endl;
            return;
        }

        // Clear the existing data in the json_data map
        json_data.clear();

        // Copy the content of the GenericObject into the std::unordered_map
        for (auto& entry : document.GetObject()) {
            std::string name = entry.name.GetString();
            json_data[name] = entry.value; // No need to clone since it already holds a copy
        }
    }

    void printCameraData() const {
        std::cout << "Content of json_data map:" << std::endl;
        for (const auto& entry : json_data) {
            std::cout << "Key: " << entry.first << ", Type: " << entry.second["type"].GetString()
                      << ", Gate ID: " << entry.second["gate_id"].GetString() << std::endl;
        }
    }

    // Other functions that need access to json_data can be added here

private:
    rapidjson::Document document;
    std::unordered_map<std::string, rapidjson::Value> json_data;
};

int main() {
    CameraDataLoader dataLoader;
    const std::string jsonFilePath = "gate.json";

    dataLoader.loadJsonData(jsonFilePath);
    dataLoader.printCameraData();

    // You can now call other functions in CameraDataLoader that use json_data

    return 0;
}
