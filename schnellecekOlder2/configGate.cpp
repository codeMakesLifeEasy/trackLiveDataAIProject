#include "configGate.hpp"



ConfigGate::ConfigGate(const std::string& json_file_path) : json_file(json_file_path), json_stream(json_file) {
    if (!json_file.is_open()) {
        std::cerr << "Error opening JSON file: " << json_file_path << std::endl;
        // Handle the error appropriately, e.g., throw an exception or set an error flag.
    }
}

int ConfigGate::parseConfig() {
    data_dict.ParseStream(json_stream);

    if (data_dict.HasParseError()) {
        // Handle parse error appropriately
        return 1;
    }

    if (data_dict.IsObject()) {
        for (auto& gate : data_dict.GetObject()) {
            const std::string& gateID = gate.name.GetString();
            const std::string front_cam = gate.value["front"].GetString();
            const std::string side_cam = gate.value["side"].GetString();

            list_front_cam.push_back(front_cam);
            list_side_cam.push_back(side_cam);
            list_gate.push_back("cam_" + gateID);

            std::cout << " front_cam: " << front_cam << std::endl;
            std::cout << " side_cam " << side_cam << std::endl;
            std::cout << "gateID : " << gateID << std::endl;
        }
    } else {
        std::cerr << "Root of JSON is not an object." << std::endl;
        return 1;
    }

    // Return success or appropriate error code
    return 0;
}
