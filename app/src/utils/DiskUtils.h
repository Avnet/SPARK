#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <vector>

namespace disk_utils
{
    bool serializeROIs(const std::vector<cv::Rect> &rois);
    std::vector<cv::Rect> deserializeROIs();
}
