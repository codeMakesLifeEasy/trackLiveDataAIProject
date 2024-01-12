/* 

#include <iostream>
#include <string>
#include <unordered_map>
#include <random>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <thread>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstring>

#include "occupiedStatusDetection.hpp"
#include "loadStatus.hpp"
#include "meta_service.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"

#include "loggerConfig.hpp"

rapidjson::Document TruckOccupationChecker::document;
bool TruckOccupationChecker::loadFlag = false;
int TruckOccupationChecker::frameIndex = 0;

std::string  TruckOccupationChecker::TruckFirstTimestamp = "";
std::string  TruckOccupationChecker::TruckLastTimestamp = "";
std::string  TruckOccupationChecker::firstForkliftTimestamp = "";
std::string  TruckOccupationChecker::lastForkliftTimestamp = "";

static int ForkliftIndexNumber = -1;

const size_t counterSize = 30;

const size_t flagSize = 10;
const size_t numberofGates = 19;
static bool initialized = false;

std::vector<GateData> gateDataArray(numberofGates);
std::vector<std::string> occupiedGateList;

static int TruckIndexNumber = -1;
std::unordered_map<std::string, rapidjson::Value> json_data;
const std::string jsonFilePath = "gate.json";

extern Meta meta;

std::string gate_id = "";
std::string cam_id = "";
std::string front_cam = "";
std::string side_cam = ""; 

static bool isDataReady = false;
rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator;


TruckOccupationChecker::TruckOccupationChecker(
        const BoundingBox& roi,
        const BoundingBox& truckBoundingBox
    ) : roi(roi),
        truckBoundingBox(truckBoundingBox){
        }


double TruckOccupationChecker::calculateIoU(const BoundingBox& box1, const BoundingBox& box2) {
    double xA = std::max(box1.x, box2.x);
    double yA = std::max(box1.y, box2.y);
    double xB = std::min(box1.x + box1.width, box2.x + box2.width);
    double yB = std::min(box1.y + box1.height, box2.y + box2.height);

    double intersectionArea = std::max(0.0, xB - xA) * std::max(0.0, yB - yA);
    double unionArea = box1.width * box1.height + box2.width * box2.height - intersectionArea;

    return static_cast<double>(intersectionArea) / unionArea;
}

double TruckOccupationChecker::calculateIoMin(const BoundingBox& box1, const BoundingBox& box2) {
    double intersectionArea = std::max(0.0, std::min(box1.x + box1.width, box2.x + box2.width) - std::max(box1.x, box2.x)) *
                            std::max(0.0, std::min(box1.y + box1.height, box2.y + box2.height) - std::max(box1.y, box2.y));
        return static_cast<double>(intersectionArea) / (box2.width * box2.height);
}

void TruckOccupationChecker::frontCamOccupiedStatus(const std::string& licenseNumber, const std::string& truckId, GateData& frontGateData)
{   
    if (frontGateData.frontConditionCounter >= counterSize && (!licenseNumber.empty() || !truckId.empty()) ) {
        if(frontGateData.licenceNumber.empty() && frontGateData.frontTrackId.empty()){

            frontGateData.licenceNumber = licenseNumber;
            frontGateData.frontTrackId = truckId;
            if (frontGateData.occupiFlag) {
            }
        }
        if((!frontGateData.licenceNumber.empty() && frontGateData.licenceNumber != licenseNumber) || (!frontGateData.frontTrackId.empty() && frontGateData.frontTrackId != truckId))
        {  
            --frontGateData.frontFlag;
            if(frontGateData.frontFlag == 0) {
                frontGateData.frontConditionCounter = 0;
                logger.log(INFO_LOG,"*****  front camera enter to unoccupied    ****");
                frontGateData.frontFlag = flagSize;
            }
        }
    }
    else {
        // Increment conditionCounter
        ++frontGateData.frontConditionCounter;
    }
}

void TruckOccupationChecker::unoccupied(GateData& gateData)
{
    if(gateData.licenceNumber.empty() && gateData.frontTrackId.empty() && gateData.truckIdNumber.empty()){
        logger.log(INFO_LOG," gate_id : " + gate_id );
        logger.log(INFO_LOG,"/n** Unoccupied done /n");

        std::string occupiedStatus = "UNOCCUPIED";

        auto it = std::find(occupiedGateList.begin(), occupiedGateList.end(), gate_id);
        if (it != occupiedGateList.end()) {
            occupiedGateList.erase(it);
        }

        meta.event_update( gateData.eventIdRespose, std::to_string(gateData.truckIdRespose), "",
                                                     "LOADING FINISHED",gateData.loadingStartTime, gateData.loadingEndTime);

        meta.event_update( gateData.eventIdRespose, std::to_string(gateData.truckIdRespose), gateData.lastTimestamp,
                                                    occupiedStatus,gateData.loadingStartTime, gateData.loadingEndTime);

        gateData.startLoadFlag = false;
        gateData.ForkliftTrackStatus = false;
        gateData.unoccupiFlag = false;
        gateData.occupiFlag = true;   
        finalstatus();
    }         
}
void TruckOccupationChecker::occupied(GateData& gateData)
{
    if(!gateData.truckIdNumber.empty() && (!gateData.frontTrackId.empty() || !gateData.licenceNumber.empty() )){

        logger.log(INFO_LOG,"************************* changed to occupied state for the below datasets     *********************");
        logger.log(INFO_LOG," gateData.licenceNumber : " +gateData.licenceNumber );
        logger.log(INFO_LOG," gateData.frontTrackId : "+ gateData.frontTrackId); 
        logger.log(INFO_LOG," gateData.truckIdNumber : "+ gateData.truckIdNumber); 
        logger.log(INFO_LOG," gate_id : "+ gate_id);

        occupiedGateList.push_back(gate_id);

        std::string status = "OCCUPIED";
        std::string plateNumber = "UNKNOWN";

        if(gateData.licenceNumber.empty())
        {
            logger.log(INFO_LOG," There is not OCR, so truck plateNumber updated to UNKNOWN ");
            gateData.truckIdRespose = meta.createtruck(plateNumber, status);
        }
        else
        {
            logger.log(INFO_LOG,"OCR found, This truck plateNumber is updated to createTruck : "+ gateData.licenceNumber);
            gateData.truckIdRespose = meta.createtruck(gateData.licenceNumber, status);
            logger.log(INFO_LOG,"gateData.truckIdRespose : "+ gateData.truckIdRespose );
        }
        bool gateUpdateResponse = meta.gate_update(gate_id,status);

        gateData.eventIdRespose = meta.event(gate_id, std::to_string(gateData.truckIdRespose), gateData.firstTimestamp);

        logger.log(INFO_LOG,"eventApiRespose : " + gateData.eventIdRespose);
        meta.event_update( gateData.eventIdRespose, std::to_string(gateData.truckIdRespose), "",
                                                    status, "", "");
        
        logger.log(INFO_LOG,"**************************************    ***********************************");
        gateData.startLoadFlag = true;
        gateData.startLoadAfterOccupied = true;
        gateData.unoccupiFlag = true;
        gateData.occupiFlag = false;
    }
}

void TruckOccupationChecker::handleGate(std::string& Timestamp, const std::string& TruckIdNumber, GateData& sideGateData) {

    // double ioMin = calculateIoMin(roi, truckBoundingBox);
    // double ioU = calculateIoU(roi, truckBoundingBox);

    sideGateData.lastUpdateTime = std::chrono::steady_clock::now(); 
    
    double truckX1 = truckBoundingBox.x;
    double truckY1 = truckBoundingBox.y;
    double truckX2 = truckBoundingBox.width;
    double truckY2 = truckBoundingBox.height;
    double truckAvailablePercentage = (((truckX2 - truckX1) * (truckY2 - truckY1)) / (1920 * 1080)) * 100; // gate_id

    std::tm timeInfo = {};
    std::istringstream iss(Timestamp);
    iss >> std::get_time(&timeInfo, "%Y-%m-%dT%H:%M:%S:");
    std::ostringstream formattedTimestamp;
    formattedTimestamp << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S.000");

    if ((++sideGateData.conditionCounter >= counterSize) && !TruckIdNumber.empty()){

        //  only once will get executed 
        if(sideGateData.occupiFlag && sideGateData.truckIdNumber.empty())
            sideGateData.truckIdNumber = TruckIdNumber;
        
        if(sideGateData.occupiFlag) {
            sideGateData.firstTimestamp = formattedTimestamp.str();
            TruckFirstTimestamp = sideGateData.firstTimestamp;
            occupied(sideGateData);
        }

        

        if(sideGateData.truckIdNumber != TruckIdNumber)
        {  
            --sideGateData.flag;
            if(sideGateData.flag == 10) {
                sideGateData.frontConditionCounter = 0;
                sideGateData.flag = flagSize;
                if(sideGateData.unoccupiFlag) {
                    logger.log(INFO_LOG," ******    truck unoccupied for the below data **************** /n");
                    logger.log(INFO_LOG," gateData.licenceNumber " +sideGateData.licenceNumber);
                    logger.log(INFO_LOG," gateData.frontTrackId " + sideGateData.frontTrackId); 
                    logger.log(INFO_LOG," gateData.truckIdNumber " + sideGateData.truckIdNumber); 
                    sideGateData.truckIdNumber = "";
                    sideGateData.frontTrackId = "";
                    sideGateData.licenceNumber = "";
                    unoccupied(sideGateData);
                }
            }
        }
      
        //auto elapsedSeconds =  std::chrono::duration_cast<std::chrono::seconds>(sideGateData.elapsedTime).count();
        //std::string elapsedTimeString = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(sideGateData.elapsedTime.count()));
        //logger.log(INFO_LOG," gate_id " + gate_id);  logger.log(INFO_LOG,"ideGateData.elapsedTime" + elapsedSeconds);
        //std::cout<< elapsedSeconds <<std::endl;
        int n = occupiedGateList.size();
        logger.log(INFO_LOG, "occupiedGateList.size()", n);

        auto currentTime = std::chrono::steady_clock::now();
        for (int i = 0; i < n; i++) {
            // Ensure i is a valid index for gateDataArray
            if (i < gateDataArray.size()) {
                auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - gateDataArray[i].lastUpdateTime);

                // Use elapsedTime within your logic
                if (elapsedTime > std::chrono::seconds(2500000)) {
                        logger.log(INFO_LOG," ******    truck unoccupied for the below data for loop data **************** /n");
                        logger.log(INFO_LOG," gateData.licenceNumber " +sideGateData.licenceNumber);
                        logger.log(INFO_LOG," gateData.frontTrackId " + sideGateData.frontTrackId); 
                        logger.log(INFO_LOG," gateData.truckIdNumber " + sideGateData.truckIdNumber); 
                        sideGateData.truckIdNumber = "";
                        sideGateData.frontTrackId = "";
                        sideGateData.licenceNumber = "";
                        unoccupied(sideGateData);
                    }
                }
                
        }
    }    
    else {
        // Increment conditionCounter
        ++sideGateData.conditionCounter;
    }
    sideGateData.lastTimestamp = formattedTimestamp.str();
    TruckLastTimestamp = sideGateData.lastTimestamp;
}

void TruckOccupationChecker::frontCamTruckandLicense(const std::string& licenseNumber, const std::string& truckId)
{
    if(!initialized){   
        for (int i = 1; i < numberofGates; ++i) {
            gateDataArray[i] = {0,0,flagSize,flagSize,0,0,-1, 0,"","", "", "","","", "", "", true,false,false,false,false};
        }
        initialized = true;
    }
    int gateNumber = std::stoi(gate_id) - 0;
    //std::cout<< " licenseNumber: "<< licenseNumber <<" truckId "<< truckId << " gateNumber "<< gateNumber<<std::endl;
    frontCamOccupiedStatus(licenseNumber,truckId, gateDataArray[gateNumber]);

}
void TruckOccupationChecker::processFrameData(const std::string& FrameData, const std::string& IdNumber, std::string timeStamp) {
    
    if(!initialized){   
        for (int i = 1; i < numberofGates; ++i) {
            gateDataArray[i] = {0,0,flagSize,flagSize, 0,0,-1,0, "","","","","","","","", true,false,false,false,false};
        }
        initialized = true;
    }
    int gateNumber = std::stoi(gate_id) - 0; 
    //std::cout<<" IdNumber "<< IdNumber << " gateNumber "<< gateNumber<<std::endl;
    handleGate(timeStamp, IdNumber, gateDataArray[gateNumber]);
}

void TruckOccupationChecker::processBboxes(const std::string& FrameData, const rapidjson::Value& bboxesArray, const rapidjson::Value& IdObjArray,rapidjson::SizeType Index,const std::string timeStamp)
{   // Assuming bboxesArray is an array and truckIndex is valid
    TruckOccupationChecker obj;
    BoundingBox newTruckBoundingBox;
    std::string TruckId=  std::to_string(IdObjArray[Index].GetInt());  // = IdObjArray[Index].GetString();
    if (bboxesArray.IsArray() && Index < bboxesArray.Size()) {
        const rapidjson::Value& boundingBox = bboxesArray[Index];

        // Assuming the bounding box is an array of four numbers
        if (boundingBox.IsArray() && boundingBox.Size() == 4) {
            // Extract values from the bounding box array
            newTruckBoundingBox = {
                boundingBox[0].GetDouble(),
                boundingBox[1].GetDouble(),
                boundingBox[2].GetDouble(),
                boundingBox[3].GetDouble()
            };
        }
    }
    obj.updateBoundingBox(newTruckBoundingBox);
    obj.processFrameData(FrameData, TruckId, timeStamp);
}

void TruckOccupationChecker::loadJsonData() {
        std::ifstream jsonFile(jsonFilePath);
        if (!jsonFile.is_open()) {
            std::cerr << "Error opening JSON file: " << jsonFilePath << std::endl;
            return;
        }
        std::stringstream buffer;
        buffer << jsonFile.rdbuf();
        std::string jsonDataStr = buffer.str();

        rapidjson::Document document;
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
        /* for (const auto& entry : json_data) {
            std::cout << "Key: " << entry.first << ", Type: " << entry.second["type"].GetString()
                      << ", Gate ID: " << entry.second["gate_id"].GetString() << std::endl;
        } 
    }
 
void TruckOccupationChecker::processContinuousData(const std::string& FrameData) {
    cam_id = ""; gate_id =""; side_cam =""; front_cam = "";

    LoadStatusChecker loadStatus;
    document.Parse(FrameData.c_str());
    if (document.HasParseError() || !document.IsObject()) {
        logger.log(ERROR_LOG, "Error parsing JSON data.");
        return;
    }
    for (const auto& member : document.GetObject()) {
        if (!member.value.IsObject()) {
            continue;
        }
        loadJsonData();

        /* cam_id = member.name.GetString();
        
        std::string camType = json_data[cam_id]["type"].GetString();
        if (json_data[cam_id].HasMember("gate_id")) {
            gate_id = json_data[cam_id]["gate_id"].GetString();
        } 
        std::string camType;
        try {
            assert(member.name.IsString());
            std::string cam_id = member.name.GetString();
            if (!cam_id.empty()) {
                //logger.log(INFO_LOG, "Successfully extracted camera ID: " + cam_id);

                assert(json_data[cam_id]["type"].IsString());
                camType = json_data[cam_id]["type"].GetString();

                if (json_data[cam_id].HasMember("gate_id")) {

                    assert(json_data[cam_id]["gate_id"].IsString());
                    gate_id = json_data[cam_id]["gate_id"].GetString();

                    //logger.log(INFO_LOG, "Login successful. Camera ID: " + cam_id + ", Camera Type: " + camType + ", Gate ID: " + gate_id);
                } else {
                    logger.log(ERROR_LOG, "Login failed. Camera ID: " + cam_id);
                }
            } else {
                logger.log(ERROR_LOG, "Error extracting camera ID from JSON data.");
            }
        } catch (const std::exception& e) {
            // Log the exception
            logger.log(ERROR_LOG, "Exception caught: " + std::string(e.what()));
        }

        const rapidjson::Value& objectClassesArray = member.value["object_classes"];
        const rapidjson::Value& BboxesArray = member.value["bboxes"];
        const rapidjson::Value& timeStamp = member.value["timestamp"];
        const rapidjson::Value& objectidsArray = member.value["object_ids"];
        const rapidjson::Value& licensePlateArray = member.value["license_plate"];
        const rapidjson::Value& imageFlag = member.value["take_image"];
             
        if("front" == camType){
            try {
            // std::cout<< " front type : "<< camType<<std::endl;
                std::string truckId = "";
                std::string numberPlate = "";
                for (rapidjson::SizeType i = 0; i < objectClassesArray.Size(); ++i) {

                    // Check if objectClassesArray[i] is a string
                    assert(objectClassesArray[i].IsString());
                    if (strcmp(objectClassesArray[i].GetString(), "truck") == 0) {

                        // Check if objectidsArray[i] is an integer
                        assert(objectidsArray[i].IsInt());
                        truckId = std::to_string(objectidsArray[i].GetInt());
                        // std::cout << "truck : " << truckId << std::endl;
                    }
                    if (strcmp(objectClassesArray[i].GetString(), "number_plate") == 0) {

                        // Check if licensePlateArray[i] is a string
                        if(licensePlateArray[i].IsString())
                        {
                            numberPlate = licensePlateArray[i].GetString();
                        }
                        else{
                            numberPlate = "";
                        }
                    }
                }
                frontCamTruckandLicense(numberPlate, truckId);
                //logger.log(INFO_LOG, "Truck ID: " + truckId + ", Number Plate: " + numberPlate);
            }catch (const std::exception& e) {
                // Log the exception
                logger.log(ERROR_LOG, "Exception caught: " + std::string(e.what()));
            }
        }     
        bool truckCover = true;
        if ("side" == camType) {
            try {
                for (rapidjson::SizeType i = 0; i < objectClassesArray.Size(); ++i) {
                    assert(objectClassesArray[i].IsString());
                    if (strcmp(objectClassesArray[i].GetString(), "truck") == 0) {
                        processBboxes(FrameData, BboxesArray, objectidsArray, i, timeStamp.GetString());
                        truckCover = false;
                    }
                }

                if (truckCover) {
                    for (rapidjson::SizeType i = 0; i < objectClassesArray.Size(); ++i) {
                        assert(objectClassesArray[i].IsString());
                        if (strcmp(objectClassesArray[i].GetString(), "cover") == 0) {
                            processBboxes(FrameData, BboxesArray, objectidsArray, i, timeStamp.GetString());
                        }
                    }
                }
            }
            catch (const std::exception& e) {
            // Log the exception
            logger.log(ERROR_LOG, "Exception caught: " + std::string(e.what()));
            }
        }
        
        if(gateDataArray[std::stoi(gate_id) - 0].startLoadFlag)
        {
            rapidjson::SizeType tIndex = 0;
            for (rapidjson::SizeType i = 0; i < objectClassesArray.Size(); ++i) { 
                try {
                    assert(objectClassesArray[i].IsString());
                    if (strcmp(objectClassesArray[i].GetString(), "forklift") == 0) {
                        logger.log(INFO_LOG, "forklift: ");
                        gateDataArray[std::stoi(gate_id) - 0].ForkliftTrackStatus = true;
                        assert(timeStamp.IsString());
                        detectFirstAndLastForklift(timeStamp.GetString(), gateDataArray[std::stoi(gate_id) - 0]);

                        rapidjson::SizeType tIndex = 0;

                        for (rapidjson::SizeType k = 0; k < objectClassesArray.Size(); ++k) {
                            assert(objectClassesArray[k].IsString());
                            if (strcmp(objectClassesArray[k].GetString(), "truck") == 0) {
                                tIndex = k;
                                break;
                            }
                        }

                        // Check if cover is not detected
                        for (rapidjson::SizeType k = 0; k < objectClassesArray.Size(); ++k) {
                            assert(objectClassesArray[k].IsString());
                            if (strcmp(objectClassesArray[k].GetString(), "cover") == 0) {
                                gateDataArray[std::stoi(gate_id) - 0].loadStatusFlagTemp = loadStatus.processBboxes(FrameData, BboxesArray, k, gateDataArray[std::stoi(gate_id) - 0].ForkliftTrackStatus, tIndex);
                                break;
                            }
                        }
                        break;
                    }
                } catch (const std::exception& e) {
                    logger.log(ERROR_LOG, "Exception caught: " + std::string(e.what()));
                }
                try {
                    if (strcmp(objectClassesArray[i].GetString(), "cover") == 0) {
                        // check if forklift detected or presence
                        rapidjson::SizeType tIndex = 0;

                        for (rapidjson::SizeType k = 0; k < objectClassesArray.Size(); ++k) {
                            assert(objectClassesArray[k].IsString());
                            if (strcmp(objectClassesArray[k].GetString(), "truck") == 0) {
                                tIndex = k;
                                break;
                            }
                        }

                        if (gateDataArray[std::stoi(gate_id) - 0].ForkliftTrackStatus) {
                            gateDataArray[std::stoi(gate_id) - 0].loadStatusFlagTemp = loadStatus.processBboxes(FrameData, BboxesArray, i, gateDataArray[std::stoi(gate_id) - 0].ForkliftTrackStatus, tIndex);
                            detectFirstAndLastForklift(timeStamp.GetString(), gateDataArray[std::stoi(gate_id) - 0]);
                            break;
                        }

                        for (rapidjson::SizeType j = 0; j < objectClassesArray.Size(); ++j) {
                            assert(objectClassesArray[j].IsString());
                            if (strcmp(objectClassesArray[j].GetString(), "forklift") == 0) {
                                gateDataArray[std::stoi(gate_id) - 0].loadStatusFlagTemp = loadStatus.processBboxes(FrameData, BboxesArray, j, gateDataArray[std::stoi(gate_id) - 0].ForkliftTrackStatus, tIndex);
                                detectFirstAndLastForklift(timeStamp.GetString(), gateDataArray[std::stoi(gate_id) - 0]);
                                break;
                            }
                        }
                    }
                } catch (const std::exception& e) {
                    // Log the exception
                    logger.log(ERROR_LOG, "Exception caught: " + std::string(e.what()));
                }

                if (gateDataArray[std::stoi(gate_id) - 0].loadStatusFlagTemp == 1 &&  gateDataArray[std::stoi(gate_id) - 0].loadStatusFlag!=gateDataArray[std::stoi(gate_id) - 0].loadStatusFlagTemp)
                {
                    logger.log(INFO_LOG,"LOADING LOADING");

                    meta.event_update(gateDataArray[std::stoi(gate_id) - 0].eventIdRespose, std::to_string(gateDataArray[std::stoi(gate_id) - 0].truckIdRespose), "",
                                            "LOADING", gateDataArray[std::stoi(gate_id) - 0].loadingStartTime, "");
                }
                
                gateDataArray[std::stoi(gate_id) - 0].loadStatusFlag = gateDataArray[std::stoi(gate_id) - 0].loadStatusFlagTemp;
            }
        }
    }
}
void TruckOccupationChecker::detectFirstAndLastForklift(const std::string &timeStamp, GateData& gateData) {
    std::tm timeInfo = {};
    std::istringstream iss(timeStamp);
    iss >> std::get_time(&timeInfo, "%Y-%m-%dT%H:%M:%S");
    std::ostringstream formattedTimestamp;
    formattedTimestamp << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S.000");
    if (gateData.forkliftIndexNumber == -1) {
        // First frame with forklift
        gateData.loadingStartTime = formattedTimestamp.str();
        gateData.forkliftIndexNumber = 0;
    }
    gateData.loadingEndTime = formattedTimestamp.str();
}
void TruckOccupationChecker::finalstatus()
{
    std::cout<< " ***************************   SESSION COMPLETED ***************************************\n" << std::endl;
    logger.log(INFO_LOG," TruckFirstTimestamp : " + TruckFirstTimestamp);
    logger.log(INFO_LOG," \nfirstForkliftTimestamp : " + firstForkliftTimestamp 
                        + " lastForkliftTimestamp : " + lastForkliftTimestamp);
    logger.log(INFO_LOG," \nTruckLastTimestamp : " + TruckLastTimestamp);
    logger.log(INFO_LOG," \n *****************************************************************************************");

    ForkliftIndexNumber = -1;

   // std::this_thread::sleep_for(std::chrono::milliseconds(100000));
}
TruckOccupationChecker::~TruckOccupationChecker()
{
    //std::cout.rdbuf(coutbuf);
}

















































































































                /* gateDataArray[std::stoi(gate_id) - 0].loadStatusFlag!=gateDataArray[std::stoi(gate_id) - 0].loadStatusFlagTemp &&  */

/* if(gateDataArray[std::stoi(gate_id) - 0].startLoadAfterOccupied == false && truckAvailablePercentage <=20)
        {  
            if(sideGateData.unoccupiFlag) {
                std::cout << "truckAvailablePercentage : "<< truckAvailablePercentage<< "ateDataArray[std::stoi(gate_id) - 0].startLoadAfterOccupied :"<< gateDataArray[std::stoi(gate_id) - 0].startLoadAfterOccupied<<std::endl;

                logger.log(INFO_LOG,"/n******    truck unoccupied for the below data **************** /n");
                logger.log(INFO_LOG," gateData.licenceNumber " +sideGateData.licenceNumber);
                logger.log(INFO_LOG," gateData.frontTrackId " + sideGateData.frontTrackId); 
                logger.log(INFO_LOG," gateData.truckIdNumber " + sideGateData.truckIdNumber); 
                sideGateData.truckIdNumber = "";
                sideGateData.frontTrackId = "";
                sideGateData.licenceNumber = "";
                sideGateData.frontConditionCounter = 0;
                sideGateData.flag = flagSize;
                unoccupied(sideGateData);
            }
        } */


        
/* void TruckOccupationChecker::frontCamOccupiedStatus(const std::string& licenseNumber, const std::string& truckId, GateData& frontGateData) {
    std::unique_lock<std::mutex> lock(frontGateData.mtx);

    if (frontGateData.frontConditionCounter >= 3 && !licenseNumber.empty() && !truckId.empty()) {
        std::cout << "Front: " << licenseNumber << ", " << truckId << std::endl;

        if (frontGateData.licenceNumber.empty() && frontGateData.frontTrackId.empty()) {
            frontGateData.licenceNumber = licenseNumber;
            frontGateData.frontTrackId = truckId;

            // Notify sideCamOccupiedStatus that data is ready
            isDataReady = true;
            frontGateData.cv.notify_all();
            frontGateData.cv.wait(lock, [&] { return !isDataReady; });
        }

        if ((!frontGateData.licenceNumber.empty() && frontGateData.licenceNumber != licenseNumber) ||
            (!frontGateData.frontTrackId.empty() && frontGateData.frontTrackId != truckId)) {
            std::cout << "Front: Unoccupied check start" << std::endl;
            --frontGateData.frontFlag;
            if (frontGateData.frontFlag == 0) {
                frontGateData.frontConditionCounter = 0;
                std::cout << "Front: Unoccupied" << std::endl;
                
                isDataReady = true;
                frontGateData.cv.notify_all();

                // Notify sideCamOccupiedStatus that data is ready
                frontGateData.cv.wait(lock, [&] { return !isDataReady; });

                frontGateData.licenceNumber = "";
                frontGateData.frontTrackId = "";
                frontGateData.frontFlag = 10;
            }
        }
    } else {
        ++frontGateData.frontConditionCounter;
    }

    // Wait until sideCamOccupiedStatus is done processing
    frontGateData.cv.wait(lock, [&] { return !isDataReady; });






        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        sideGateData.elapsedTime = currentTime - sideGateData.lastUpdateTime;

        // Perform some work or wait for a while...

        // Update the currentTime and calculate the elapsed time
        auto newCurrentTime = std::chrono::steady_clock::now();
        sideGateData.elapsedTime = newCurrentTime - sideGateData.currentTime;
        sideGateData.currentTime = newCurrentTime;
} */


/* 
void TruckOccupationChecker::handleGate(std::string& Timestamp, const std::string& TruckIdNumber, GateData& sideGateData) {
    //  void handleGate(std::string& Timestamp, const std::string& indexNumber, GateData& gateData);
    
    std::unique_lock<std::mutex> lock(sideGateData.mtx);

    // Wait until frontCamOccupiedStatus signals that data is ready
    sideGateData.cv.wait(lock, [&] { return isDataReady; });

    if ((sideGateData.conditionCounter >= 3) && !TruckIdNumber.empty()) {
        std::cout << "Side: " << Timestamp << ", " << TruckIdNumber << std::endl;

        if (sideGateData.occupiFlag && sideGateData.truckIdNumber.empty())
            sideGateData.truckIdNumber = TruckIdNumber;

        if (sideGateData.occupiFlag) {
            std::cout << "Side: Called to occupied()" << std::endl;
            sideGateData.firstTimestamp = Timestamp;
            occupied(sideGateData);
            isDataReady = false;
            sideGateData.cv.notify_all();
        }

        if (sideGateData.truckIdNumber != TruckIdNumber) {
            std::cout << "Side: Unoccupied checking" << std::endl;
            --sideGateData.flag;
            if (sideGateData.flag == 0) {
                sideGateData.frontConditionCounter = 0;
                sideGateData.flag = 10;
                if (sideGateData.unoccupiFlag) {
                    std::cout << "Side: Unoccupied" << std::endl;
                    isDataReady = false;
                    sideGateData.truckIdNumber = "";
                    sideGateData.frontTrackId = "";
                    sideGateData.licenceNumber = "";
                    unoccupied(sideGateData);
                    sideGateData.cv.notify_all();
                }
            }
        }
    } else {
        ++sideGateData.conditionCounter;
    }
}
 

 











































#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

struct GateData {
    std::string licenceNumber;
    std::string frontTrackId;
    std::string truckIdNumber;
    int frontFlag;
    int frontConditionCounter;
    int flag;
    int conditionCounter;
    bool occupiFlag;
    bool unoccupiFlag;
    std::string firstTimestamp;
    std::string lastTimestamp;
    std::mutex mtx;
    std::condition_variable cv;
};

bool isDataReady = false;

void occupied(GateData& data) {
    // Your implementation for occupied function
    std::cout << "Occupied function called" << std::endl;
} 

void unoccupied(GateData& data) {
    // Your implementation for unoccupied function
    std::cout << "Unoccupied function called" << std::endl;
}

void frontCamOccupiedStatus(const std::string& licenseNumber, const std::string& truckId, GateData& frontGateData, GateData& sideGateData) {
    std::unique_lock<std::mutex> lock(frontGateData.mtx);

    if (frontGateData.frontConditionCounter >= 3 && !licenseNumber.empty() && !truckId.empty()) {
        std::cout << "Front: " << licenseNumber << ", " << truckId << std::endl;

        if (frontGateData.licenceNumber.empty() && frontGateData.frontTrackId.empty()) {
            frontGateData.licenceNumber = licenseNumber;
            frontGateData.frontTrackId = truckId;

            // Notify sideCamOccupiedStatus that data is ready
            isDataReady = true;
            sideGateData.cv.notify_all();
            frontGateData.cv.wait(lock, [&] { return !isDataReady; });
        }

        if ((!frontGateData.licenceNumber.empty() && frontGateData.licenceNumber != licenseNumber) ||
            (!frontGateData.frontTrackId.empty() && frontGateData.frontTrackId != truckId)) {
            std::cout << "Front: Unoccupied check start" << std::endl;
            --frontGateData.frontFlag;
            if (frontGateData.frontFlag == 0) {
                frontGateData.frontConditionCounter = 0;
                std::cout << "Front: Unoccupied" << std::endl;
                
                /* isDataReady = true;
                sideGateData.cv.notify_all();

                // Notify sideCamOccupiedStatus that data is ready
                frontGateData.cv.wait(lock, [&] { return !isDataReady; });

                frontGateData.licenceNumber = "";
                frontGateData.frontTrackId = "";
                frontGateData.frontFlag = 10; 
            }
        }
    } else {
        ++frontGateData.frontConditionCounter;
    }

    // Wait until sideCamOccupiedStatus is done processing
    frontGateData.cv.wait(lock, [&] { return !isDataReady; });
}

void sideCamOccupiedStatus(const std::string& Timestamp, const std::string& TruckIdNumber, GateData& sideGateData, GateData& frontGateData) {
    std::unique_lock<std::mutex> lock(sideGateData.mtx);

    // Wait until frontCamOccupiedStatus signals that data is ready
    sideGateData.cv.wait(lock, [&] { return isDataReady; });

    if ((sideGateData.conditionCounter >= 3) && !TruckIdNumber.empty()) {
        std::cout << "Side: " << Timestamp << ", " << TruckIdNumber << std::endl;

        if (sideGateData.occupiFlag && sideGateData.truckIdNumber.empty())
            sideGateData.truckIdNumber = TruckIdNumber;

        if (sideGateData.occupiFlag) {
            std::cout << "Side: Called to occupied()" << std::endl;
            sideGateData.firstTimestamp = Timestamp;
            occupied(sideGateData);
            isDataReady = false;
            frontGateData.cv.notify_all();
        }

        if (sideGateData.truckIdNumber != TruckIdNumber) {
            std::cout << "Side: Unoccupied checking" << std::endl;
            --sideGateData.flag;
            if (sideGateData.flag == 0) {
                sideGateData.frontConditionCounter = 0;
                sideGateData.flag = 10;
                if (sideGateData.unoccupiFlag) {
                    std::cout << "Side: Unoccupied" << std::endl;
                    isDataReady = false;
                    sideGateData.truckIdNumber = "";
                    sideGateData.frontTrackId = "";
                    sideGateData.licenceNumber = "";
                    unoccupied(sideGateData);
                    sideGateData.cv.notify_all();
                }
            }
        }
    } else {
        ++sideGateData.conditionCounter;
    }
}
 */