#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <vector>

#include <ParkingSpot.h>

namespace disk_utils
{
    bool serializeROIs(const std::vector<ParkingSpot> &rois);
    std::vector<ParkingSpot> deserializeROIs();
}
