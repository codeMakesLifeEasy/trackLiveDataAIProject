#include "meta_service.hpp"
#include "occupiedStatusDetection.hpp"
#include "rapidjson/document.h"
#include "loadStatus.hpp"
#include "meta_service.hpp"
#include <iostream>

const std::string BlobService::storage_url = "your_storage_url_here";  // Replace with the actual storage URL
rapidjson::Document Meta::document;


const std::string Meta::truck_service_url = "http://localhost:5001/api/server/v1/truck";
const std::string Meta::camera_service_url = "http://localhost:5001/api/server/v1/camera";
const std::string Meta::gate_service_url = "http://localhost:5001/api/server/v1/gate";
const std::string Meta::event_service_url = "http://localhost:5001/api/server/v1/event";
const std::string Meta::image_service_url = "http://localhost:5001/api/server/v1/image";


/* const std::string Meta::truck_service_url =  "http://10.100.2.5:5000/api/server/v1/truck";
const std::string Meta::camera_service_url = "http://10.100.2.5:5000/api/server/v1/camera";
const std::string Meta::gate_service_url =   "http://10.100.2.5:5000/api/server/v1/gate";
const std::string Meta::event_service_url =  "http://10.100.2.5:5000/api/server/v1/event";
const std::string Meta::image_service_url =  "http://10.100.2.5:5000/api/server/v1/image"; */

CURLcode Meta::post(const std::string& payload, const std::string& url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        return res;
    }

    return CURLE_FAILED_INIT;
}
void Meta::test()
{

}
CURLcode Meta::put(const std::string& payload, const std::string& url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        return res;
    }

    return CURLE_FAILED_INIT;
}

CURLcode Meta::get(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        CURLcode res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        return res;
    }

    return CURLE_FAILED_INIT;
}

bool Meta::image(const std::string& file_name, const std::string& session_id, const std::string& gate_id,
                 const std::string& event_id, const std::string& camera_id, const std::string& type) {
    std::string payload = R"({
        "file_name": ")" + file_name + R"(",
        "url": ")" + BlobService::storage_url + R"(/schnellecke/)" + session_id + R"(/)" + file_name + R"(",
        "gate_id": ")" + gate_id + R"(",
        "event_id": ")" + event_id + R"(",
        "camera_id": ")" + camera_id + R"(",
        "type": ")" + type + R"("
    })";

    CURLcode res = post(payload, image_service_url);

    if (res == CURLE_OK) {
        std::cout << "Image created successfully!\n";
        return true;
    }

    return false;
}

std::string Meta::event(const std::string& gate_id, const std::string& truck_id, const std::string& start_time) {
    std::string payload = R"({
        "truck_id": ")" + truck_id + R"(",
        "gate_id": ")" + gate_id + R"(",
        "start_time": ")" + start_time + R"("
    })";

    std::string response = postlocal(payload, event_service_url); // post(payload, event_service_url);
    std::string idValue;
    int temp;
    document.Parse(response.c_str());
    if (document.HasParseError() || !document.IsObject()) {
        return idValue;
    }
     if (document.HasMember("id") && document["id"].IsInt()) {
        temp = document["id"].GetInt();
        idValue = std::to_string(temp);
    }

    std::cout << "Response event : " << response << std::endl;
    if (!response.empty()) {
        std::cout << "<Event created successfully!\n";
        return idValue;
    }
    return idValue;
}

int Meta::createtruck(const std::string& number, const std::string& status) {
    std::string payload = R"({
        "plate_number": ")" + number + R"(",
        "truck_status": ")" + status + R"("
    })";
    int idValue;
    std::string response = postlocal(payload, truck_service_url);
    document.Parse(response.c_str());
    if (document.HasParseError() || !document.IsObject()) {
        return idValue;
    }
     if (document.HasMember("id") && document["id"].IsInt()) {
        idValue = document["id"].GetInt();
    }

    std::cout << "Response: " << response << std::endl;

    if (!response.empty()) {
        std::cout << "Truck created successfully!\n";
        return idValue;
    }
    return idValue;
}

std::string Meta::postlocal(const std::string& payload, const std::string& url) {
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    std::string response;

    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        /* curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
         */
        //CURLcode res = curl_easy_perform(curl);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();

    if (res != CURLE_OK) {
        std::cerr << "Failed to perform HTTP request. CURLcode: " << res << "\n";
    }

    return response;
}

size_t Meta::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append(static_cast<char*>(contents), total_size);
    return total_size;
}

/* 
bool Meta::createtruck(const std::string& number, const std::string& status) {
    std::string payload = R"({
        "plate_number": ")" + number + R"(",
        "truck_status": ")" + status + R"("
    })";

    std::string response = post(payload, truck_service_url);
    std::cout << "Response: " << response << std::endl;
    CURLcode res = getLastCurlCode();
    if (res == CURLE_OK) {
        std::cout << "Truck created successfully!\n";
        return true;
    }

    return false;
} */

bool Meta::truck_update(const std::string& number, const std::string& status, const std::string& truck_id) {
    std::string payload = R"({
        "plate_number": ")" + number + R"(",
        "truck_status": ")" + status + R"("
    })";

    CURLcode res = put(payload, truck_service_url + "/" + truck_id);

    if (res == CURLE_OK) {
        std::cout << "Truck updated successfully!\n";
        return true;
    }

    return false;
}

bool Meta::gate_update(const std::string& gate_id, const std::string& status) {
    std::string payload = R"({
        "status": ")" + status + R"("
    })";
    CURLcode res = put(payload, gate_service_url + "/" + gate_id);

    if (res == CURLE_OK) {
        std::cout << "Gate updated successfully!\n";
        return true;
    }

    return false;
}

bool Meta::event_update(const std::string& event_id, const std::string& truck_id, const std::string& end_time,
                        const std::string& status, const std::string& loading_start_time,
                        const std::string& loading_end_time) {
    std::string payload = R"({
        "truck_id": ")" + truck_id + R"(",
        "end_time": ")" + end_time + R"(",
        "status": ")" + status + R"(",
        "loading_start_time": ")" + loading_start_time + R"(",
        "loading_end_time": ")" + loading_end_time + R"("
    })";

    std::cout<<"PAYLOAD : " << payload<<std::endl;

    CURLcode res = put(payload, event_service_url + "/" + event_id);

    if (res == CURLE_OK) {
        std::cout << "Event updated successfully!\n";
        return true;
    }

    return false;
}

bool Meta::camera() {
    CURLcode res = get(camera_service_url);

    if (res == CURLE_OK) {
        std::cout << "Camera IDs retrieved successfully!\n";
        return true;
    }

    return false;
}
