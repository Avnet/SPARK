#include <string>
#include <iostream>

#include "DiskUtils.h"

namespace
{
    using namespace cv;
    using namespace std;

    const std::string SPARK_ROIS_FILEPATH = "/opt/spark/rois.json";
}

namespace disk_utils
{
    bool serializeROIs(const std::vector<cv::Rect> &rois)
    {
        try
        {
            // Overwrites if exists
            // filetype ending affects << operator
            FileStorage file(SPARK_ROIS_FILEPATH, FileStorage::WRITE);
            if (!file.isOpened())
            {
                std::cerr << "Failed to open file: " << SPARK_ROIS_FILEPATH << std::endl;
                return false;
            }

            file << "rois"
                 << "[";
            for (const auto &roi : rois)
            {
                // {: means compact form
                file << "{:"
                     << "roi"
                     << roi
                     << "}";
            }
            file << "]";
            file.release();
            return true;
        }
        catch (std::exception &e)
        {
            std::cerr << "Failed to serialize ROIs: " << e.what() << std::endl;
            return false;
        }
        catch (...)
        {
            std::cerr << "Failed to serialize ROIs. Unknown Exception." << std::endl;
            return false;
        }
    }
}
