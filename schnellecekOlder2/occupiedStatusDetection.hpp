// TruckOccupationChecker.hpp

#ifndef TRUCK_OCCUPATION_CHECKER_HPP
#define TRUCK_OCCUPATION_CHECKER_HPP
/**/
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <condition_variable>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"

struct BoundingBox {
    double x, y, width, height;
};
struct GateData {

    int frontConditionCounter;
    int conditionCounter;
    int frontFlag;
    int flag;

    int loadStatusFlag;
    int loadStatusFlagTemp;
    int forkliftIndexNumber;
    long int truckIdRespose;

    std::string eventIdRespose;
    std::string firstTimestamp;
    std::string lastTimestamp;
    std::string licenceNumber;
    std::string truckIdNumber;
    std::string frontTrackId;
    std::string loadingStartTime;
    std::string loadingEndTime;
   
    bool occupiFlag;
    bool unoccupiFlag;
    bool startLoadFlag;
    bool startLoadAfterOccupied;
    bool ForkliftTrackStatus;
    
};
//class Meta; 
class TruckOccupationChecker {
public:

    TruckOccupationChecker() = default;
    TruckOccupationChecker(const BoundingBox& roi, const BoundingBox& truckBoundingBox);

    void processContinuousData(const std::string& frameData);
    void loadJsonData();

    void frontCamTruckandLicense(const std::string& licenseNumber, const std::string& truckIndex);
    void frontCamOccupiedStatus(const std::string& licenseNumber, const std::string&, GateData& gateData);

    double calculateIoU(const BoundingBox& box1, const BoundingBox& box2);
    double calculateIoMin(const BoundingBox& box1, const BoundingBox& box2);

    void processSideCamera(const rapidjson::Value& objectClassesArray,
                                               const rapidjson::Value& BboxesArray,
                                               const rapidjson::Value& objectidsArray,
                                               const rapidjson::Value& timeStamp);
    
    void processFrontCamera(const rapidjson::Value& objectClassesArray,
                                                const rapidjson::Value& objectidsArray,
                                                const rapidjson::Value& licensePlateArray);
    
    void processGateData(const std::string& frameData,const rapidjson::Value& objectClassesArray,
                                             const rapidjson::Value& BboxesArray,
                                             const rapidjson::Value& objectidsArray,
                                             const rapidjson::Value& timeStamp,
                                             LoadStatusChecker& loadStatus);

    void processFrameData(const std::string& frameData, const std::string& idNumber, const std::string timeStamp);
    void processBboxes(const std::string& FrameData, const rapidjson::Value& bboxesArray, const rapidjson::Value& ,rapidjson::SizeType truckIndex,const std::string timeStamp);
    void handleGate(std::string& Timestamp, const std::string& indexNumber, GateData& sideGateData);

    void detectFirstAndLastForklift(const std::string &timeStamp, GateData& );

    void updateBoundingBox(const BoundingBox& newTruckBoundingBox) {
        truckBoundingBox = newTruckBoundingBox;
    }
    void finalstatus();
    void occupied(GateData& );
    void unoccupied(GateData& );
    ~TruckOccupationChecker();


     
private:
    BoundingBox roi = {80.0, 320.0, 1920.00, 1080.00};
    BoundingBox truckBoundingBox;
    std::string objectClassType;
    std::string timeStamp;
    std::string gateId;
    static rapidjson::Document document;
    static bool loadFlag;
    static int frameIndex;
    static std::string firstForkliftTimestamp;
    static std::string lastForkliftTimestamp;
    static std::string TruckFirstTimestamp;
    static std::string TruckLastTimestamp;
    std::chrono::steady_clock::time_point lastCallTime;
    const int timerDurationInSeconds = 5;
};

#endif // TRUCK_OCCUPATION_CHECKER_HPP

/*

class TruckOccupationChecker {
private:
    BoundingBox truckBoundingBox;
    BoundingBox roi;// = {0, 0, 1080, 720};
    std::string objectClassType;
    std::vector<int> objectClassIds;
    std::string timeStamp;
    double threshold;
    bool ocrResult;
    std::string gateId;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> dataMap;

public:
    TruckOccupationChecker();
    
    TruckOccupationChecker(
        const BoundingBox& roi,
        const BoundingBox& truckBoundingBox,
        const std::string& objectClassType,
        const std::string timestamp,
        const std::string& gateId
    );

    void processData(const std::unordered_map<std::string, std::string>& keyValueData);

    double calculateIoU(const BoundingBox& box1, const BoundingBox& box2);

    double calculateIoMin(const BoundingBox& box1, const BoundingBox& box2);

    bool checkTruckOccupationStatus();

    void processFrameData(const std::string& FrameData, int IdNumber);

    //void updateBoundingBox(const std::string& keyValueData);

    void processContinuousData(const std::string& continuousJsonData);

    void processBboxes(const std::string& FrameData, const rapidjson::Value& bboxesArray, const rapidjson::Value& ,rapidjson::SizeType truckIndex,const std::string timeStamp);

    void updateBoundingBox(const BoundingBox& newTruckBoundingBox,const BoundingBox& newROI, const std::string newtimeStamp) {
        truckBoundingBox = newTruckBoundingBox;
        roi = newROI;
        timeStamp = newtimeStamp;
    }
    void detectFirstAndLastForklift();
};


*/