#include <iostream>
//#include <opencv2/opencv.hpp>
//#include ""
#include <chrono>
#include <thread>
#include <filesystem>
#include <vector>

class OCR {
    // Implement OCR class if needed
};

class YOLOv8Detector {
    // Implement YOLOv8Detector class if needed
};

class Trigger {
    // Implement Trigger class if needed
};

class BlobService {
    // Implement BlobService class if needed
};

class Meta {
    // Implement Meta class if needed
};

class Camera {
public:
    Camera(int camera_index_front, int camera_index_side, YOLOv8Detector detector, OCR ocr, std::string gate_id, std::string backup_path, std::string data_folder);
    void start();
    void stop();

private:
    void stop_cameras();
    std::string find_latest_jpg_file(const std::string& directory);
    void upload_items_for_ongoing_session(std::string gate_id, std::string session_id, int count, std::string event_id, std::string front_cam_id_cloud);
    void process_data();

    YOLOv8Detector detector;
    Trigger trigger;
    std::thread process;
    std::string data_folder;
    int camera_index;
    int camera_index_side;
    std::string front_cam_folder;
    std::string side_cam_folder;
    std::string gate_id;
    std::string backup_path;
    std::string backup_dir;
    std::string front_cam_id_cloud;
    std::string side_cam_id_cloud;
    std::string event_id;
};

Camera::Camera(int camera_index_front, int camera_index_side, YOLOv8Detector detector, OCR ocr, std::string gate_id, std::string backup_path, std::string data_folder)
    : detector(detector), trigger(ocr, detector), camera_index(camera_index_front), camera_index_side(camera_index_side),
      front_cam_folder(data_folder + "/" + std::to_string(camera_index_front)), side_cam_folder(data_folder + "/" + std::to_string(camera_index_side)),
      gate_id(gate_id), backup_path(backup_path), data_folder(data_folder) {
    // Initialization code here
}

void Camera::start() {
    process = std::thread(&Camera::process_data, this);
}

void Camera::stop() {
    if (process.joinable()) {
        stop_cameras();
        process.join();
    }
}

void Camera::stop_cameras() {
    if (!event_id.empty()) {
        std::cout << "Ending session with event ID: " << event_id << std::endl;
        // Meta::event_update(/* parameters */);
    }
    // Implement the rest of the stop_cameras method
}

std::string Camera::find_latest_jpg_file(const std::string& directory) {
    // Implement the find_latest_jpg_file method
    // Return the path to the latest jpg file
    return "";
}

void Camera::upload_items_for_ongoing_session(cv::Mat plate_image, std::string gate_id, std::string session_id, int count, cv::Mat event, std::string event_id, std::string front_cam_id_cloud) {
    // Implement the upload_items_for_ongoing_session method
}

void Camera::process_data() {
    // Implement the process_data method
}

class ProcessManager {
public:
    ProcessManager(std::vector<int> camera_indices_front, std::vector<int> camera_indices_side, YOLOv8Detector detector, OCR ocr, std::vector<std::string> gate_list, std::string backup_folder, std::string data_folder);
    void start_all();
    void stop_all();

private:
    std::vector<Camera> cameras;
};

ProcessManager::ProcessManager(std::vector<int> camera_indices_front, std::vector<int> camera_indices_side, YOLOv8Detector detector, OCR ocr, std::vector<std::string> gate_list, std::string backup_folder, std::string data_folder) {
    for (size_t i = 0; i < camera_indices_front.size(); ++i) {
        cameras.emplace_back(camera_indices_front[i], camera_indices_side[i], detector, ocr, gate_list[i], backup_folder, data_folder);
    }
}

void ProcessManager::start_all() {
    for (auto& camera : cameras) {
        camera.start();
    }
}

void ProcessManager::stop_all() {
    for (auto& camera : cameras) {
        camera.stop();
    }
}

int main() {
    // Initialization code for YOLOv8Detector, OCR, and other components

    // Specify camera indices
    std::vector<int> list_front_cam;
    std::vector<int> list_side_cam;
    std::vector<std::string> list_gate;

    // Populate camera indices and gate list

    // Create a process manager
    ProcessManager manager(list_front_cam, list_side_cam, /* pass YOLOv8Detector, OCR, and other necessary parameters */);

    try {
        manager.start_all();

        // Uncomment the following lines if you want to wait for user input to stop the program
        // std::cout << "Press q to stop..." << std::endl;
        // char user_input;
        // std::cin >> user_input;
        // if (user_input == 'q') {
        //     manager.stop_all();
        // }
        
        // Simulate a running program by sleeping
        std::this_thread::sleep_for(std::chrono::minutes(1));

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
    }

    // Stop all cameras when the program ends
    manager.stop_all();

    return 0;
}
