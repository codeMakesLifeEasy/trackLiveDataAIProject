



#ifndef LOAD_STATUS_CHECKER_HPP
#define LOAD_STATUS_CHECKER_HPP

#include <vector>
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

// Assuming FrameData is a struct defined somewhere
/*struct FrameData {
    std::string objectClassType;
    std::string timestamp;
    double truckCoverOpenPercentage;
    bool forkliftDetected;
    bool truckCoverClosed;
};

enum class LoadStatus {
    NotStarted,
    LoadingStarted,
    LoadingEnded
};*/

struct CoverBoundingBox {
    double x, y, width, height;
};
struct TruckBoundingBox {
    double x, y, width, height;
};

class LoadStatusChecker {
private:
    
public:


    // Function to check the load status based on the conditions
    void checkLoadStatus();

    int processBboxes(const std::string& FrameData, const rapidjson::Value& bboxesArray, rapidjson::SizeType& truckIndex, bool,  const rapidjson::SizeType&);

    double calculateTruckCoverOpenPercentage(const CoverBoundingBox&, const TruckBoundingBox&);

private:
    // Helper function to check if forklift and truck cover conditions for loading started are met
    bool isForkliftAndTruckCoverConditionsMet();

    // Helper function to check if truck cover closed and last detection of forklift conditions for loading ended are met
    bool isTruckCoverClosedAndLastForkliftDetectionConditionsMet();

    // Helper function to share frame data to Radis
    void shareFrameData(bool loadStatus);

    
};

#endif // LOAD_STATUS_CHECKER_HPP


















