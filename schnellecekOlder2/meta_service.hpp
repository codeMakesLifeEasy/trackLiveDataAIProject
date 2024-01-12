#ifndef META_HPP
#define META_HPP

#include <string>
#include <curl/curl.h>
#include "occupiedStatusDetection.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"

class BlobService {
public:
    static const std::string storage_url;
    // You need to define the storage_url or any other required constants.
};

class Meta {
public:
    static const std::string truck_service_url;
    static const std::string camera_service_url;
    static const std::string gate_service_url;
    static const std::string event_service_url;
    static const std::string image_service_url;

    static CURLcode post(const std::string& payload, const std::string& url);
    static CURLcode put(const std::string& payload, const std::string& url);
    static CURLcode get(const std::string& url);

    static bool image(const std::string& file_name, const std::string& session_id, const std::string& gate_id,
                      const std::string& event_id, const std::string& camera_id, const std::string& type);

    static std::string event(const std::string& gate_id, const std::string& truck_id, const std::string& start_time);

    static int createtruck(const std::string& number, const std::string& status);
    static std::string postlocal(const std::string& payload, const std::string& url);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);

    static bool truck_update(const std::string& number, const std::string& status, const std::string& truck_id);

    static bool gate_update(const std::string& gate_id, const std::string& status);

    static bool event_update(const std::string& event_id, const std::string& truck_id, const std::string& end_time,
                             const std::string& status, const std::string& loading_start_time,
                             const std::string& loading_end_time);

    static bool camera();
    void test();
private:
    static rapidjson::Document document;
};

#endif // META_HPP
