#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include "loadStatus.hpp"
#include "meta_service.hpp"
#include "occupiedStatusDetection.hpp"
#include "loggerConfig.hpp"

Meta meta;


double LoadStatusChecker::calculateTruckCoverOpenPercentage(const CoverBoundingBox& coverBoundingBox, const TruckBoundingBox& truckBoundingBox) {
    // Parse the bounding box data
   
    // Extract individual values
    double coverX1 = coverBoundingBox.x;
    double coverY1 = coverBoundingBox.y;
    double coverX2 = coverBoundingBox.width;
    double coverY2 = coverBoundingBox.height;

    double truckX1 = truckBoundingBox.x;
    double truckY1 = truckBoundingBox.y;
    double truckX2 = truckBoundingBox.width;
    double truckY2 = truckBoundingBox.height;


    double cover = (coverX2 - coverX1) * (coverY2 - coverY1);
    double truck = (truckX2 - truckX1) * (truckY2 - truckY1);
    double closeAreaPercentage = (cover/truck)*100;

    // std::cout<< " x1 "<< x1 << " y1 "<< y1 << " x2 "<< x2 << " y2 "<< y2 <<std::endl;

     //logger.log(INFO_LOG,"",closeAreaPercentage);
    //logger.log(INFO_LOG, "closeAreaPercentage : ",closeAreaPercentage);
    return closeAreaPercentage;
}
    
int LoadStatusChecker::processBboxes(const std::string& FrameData, const rapidjson::Value& bboxesArray, rapidjson::SizeType& coverIndex ,bool ForkliftTrackStatus, const rapidjson::SizeType& truckIndex) {
                                    
    // Assuming bboxesArray is an array and truckIndex is valid
    LoadStatusChecker obj;
    CoverBoundingBox newCoverBoundingBox;
    TruckBoundingBox newSideTruckBoundingBox;

    try {
        if (bboxesArray.IsArray() && !bboxesArray.Empty()) {
            const rapidjson::Value& boundingBox = bboxesArray[coverIndex];
            // Assuming the bounding box is an array of four numbers
            if (boundingBox.IsArray() && boundingBox.Size() == 4) {
                // Extract values from the bounding box array
                newCoverBoundingBox = {
                    boundingBox[0].GetDouble(),
                    boundingBox[1].GetDouble(),
                    boundingBox[2].GetDouble(),
                    boundingBox[3].GetDouble()
                };
            }
            else
            {
                newCoverBoundingBox = {0.0, 0.0, 0.0, 0.0};
            }
        } else {
            newCoverBoundingBox = {0.0, 0.0, 0.0, 0.0};
        }
        
    } catch (const std::exception& e) {
        // Log the exception for the truck bounding box
        logger.log(ERROR_LOG, "Exception caught for truck bounding box: " + std::string(e.what()));
    }
    
    //double percentage = obj.calculateTruckCoverOpenPercentage(newCoverBoundingBox, newTruckBoundingBox);
    //logger.log(INFO_LOG," percentage : ", percentage);

    logger.log(INFO_LOG," ForkliftTrackStatus : ", ForkliftTrackStatus);
    int loadStatus = 0;
    if(ForkliftTrackStatus){
         //std::cout<< " load detection is working successfully " << std::endl;
         rapidjson::Document document;
         document.Parse(FrameData.c_str());
         for (auto& entry : document.GetObject()) {
            rapidjson::Value& object = entry.value;
            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
            object.AddMember("Occupied_status", 1, allocator);
            object.AddMember("Load_status", 1, allocator);
            loadStatus = 1;
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            //std::cout << " occupied and load : " << buffer.GetString() << std::endl;
           
            // Simulate waiting for the next JSON data point
            //std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }
    else{
        rapidjson::Document document;
         document.Parse(FrameData.c_str());
         for (auto& entry : document.GetObject()) {
            rapidjson::Value& object = entry.value;
            rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
            object.AddMember("Occupied_status", 1, allocator);
            object.AddMember("Load_status", 0, allocator);
            loadStatus = 0;
            ForkliftTrackStatus = false;
             rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            //std::cout << " occupied : " << buffer.GetString() << std::endl;
                // Simulate waiting for the next JSON data point
            //std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }
    return loadStatus;
}


/*

/* 

   /*
     if (bboxesArray.IsArray() && truckIndex < bboxesArray.Size()) {
        const rapidjson::Value& boundingBox = bboxesArray[truckIndex];

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

    if (bboxesArray.IsArray() && coverIndex < bboxesArray.Size()) {
        const rapidjson::Value& boundingBox = bboxesArray[coverIndex];

        // Assuming the bounding box is an array of four numbers
        if (boundingBox.IsArray() && boundingBox.Size() == 4) {
            // Extract values from the bounding box array
            newCoverBoundingBox = {
                boundingBox[0].GetDouble(),
                boundingBox[1].GetDouble(),
                boundingBox[2].GetDouble(),
                boundingBox[3].GetDouble()
            };
        }
    } */

   

/* try {
        if (bboxesArray.IsArray() && coverIndex < bboxesArray.Size()) {
            const rapidjson::Value& boundingBox = bboxesArray[coverIndex];

            // Assuming the bounding box is an array of four numbers
            assert(boundingBox.IsArray());
            assert(boundingBox.Size() == 4);
            if (boundingBox.IsArray() && boundingBox.Size() == 4) {
                // Extract values from the bounding box array
                newCoverBoundingBox = {
                    boundingBox[0].GetDouble(),
                    boundingBox[1].GetDouble(),
                    boundingBox[2].GetDouble(),
                    boundingBox[3].GetDouble()
                };
            }
        }
    } catch (const std::exception& e) {
        // Log the exception for the cover bounding box
        logger.log(ERROR_LOG, "Exception caught for cover bounding box: " + std::string(e.what()));
    } 
        if (!bboxesArray.Empty() && 0 < bboxesArray.Size()) {
            if (bboxesArray.IsArray() && truckIndex < bboxesArray.Size()) {
                const rapidjson::Value& boundingBox = bboxesArray[truckIndex];

                // Assuming the bounding box is an array of four numbers
                assert(boundingBox.IsArray());
                assert(boundingBox.Size() == 4);
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
            else
            {

            } 

// Function to add frame data
void LoadStatusChecker::addFrameData(const FrameData& frameData) {
    frameDataList.push_back(frameData);
}

// Function to check the load status based on the conditions
void LoadStatusChecker::checkLoadStatus() {
    if (currentLoadStatus == LoadStatus::NotStarted) {
        // Check if forklift and truck cover conditions for loading started are met
        if (isForkliftAndTruckCoverConditionsMet()) {
            currentLoadStatus = LoadStatus::LoadingStarted;
            std::cout << "Loading started." << std::endl;
            shareFrameData(true); // Share occupied state + load status true to Radis
        }
    } else if (currentLoadStatus == LoadStatus::LoadingStarted) {
        // Check if truck cover closed and last detection of forklift conditions for loading ended are met
        if (isTruckCoverClosedAndLastForkliftDetectionConditionsMet()) {
            currentLoadStatus = LoadStatus::LoadingEnded;
            std::cout << "Loading ended." << std::endl;
            shareFrameData(true); // Share occupied state + load status true to Radis
        }
    }

    // If the conditions are not met, share occupied state + load status false to Radis
    if (currentLoadStatus == LoadStatus::NotStarted || currentLoadStatus == LoadStatus::LoadingStarted) {
        shareFrameData(false);
    }
}

// Helper function to check if forklift and truck cover conditions for loading started are met
bool LoadStatusChecker::isForkliftAndTruckCoverConditionsMet() {
    for (const auto& frameData : frameDataList) {
        if (frameData.objectClassType == "forklift" && frameData.truckCoverOpenPercentage >= 30) {
            return true;
        }
    }
    return false;
}

// Helper function to check if truck cover closed and last detection of forklift conditions for loading ended are met
bool LoadStatusChecker::isTruckCoverClosedAndLastForkliftDetectionConditionsMet() {
    size_t lastIndex = frameDataList.size() - 1;
    if (lastIndex >= 0 && frameDataList[lastIndex].objectClassType == "forklift" &&
        frameDataList[lastIndex].truckCoverClosed) {
        return true;
    }
    return false;
}

// Helper function to share frame data to Radis
void LoadStatusChecker::shareFrameData(bool loadStatus) {
    for (const auto& frameData : frameDataList) {
        std::cout << "Occupied state: true, Load status: " << (loadStatus ? "true" : "false") << std::endl;
        // Share frame data to Radis (print for demonstration purposes)
        std::cout << "Frame Data: " << frameData.objectClassType << ", " << frameData.timestamp << ", "
                  << frameData.truckCoverOpenPercentage << ", " << frameData.forkliftDetected << ", "
                  << frameData.truckCoverClosed << std::endl;
    }
}
*/