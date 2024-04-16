#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include "DiskUtils.h"

namespace
{
    using namespace cv;
    using namespace std;

    bool createDirectory(const std::string &path)
    {
        // The permission for the new directory is set to 777
        if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1)
        {
            return false;
        }
        return true;
    }
}

namespace disk_utils
{
    bool serializeROIs(const std::vector<cv::Rect> &rois)
    {
        try
        {
            const std::string SPARK_ROIS_FILEPATH = "/opt/spark/data/rois.json";
            auto a = createDirectory("/opt");
            auto b = createDirectory("/opt/spark");
            auto c = createDirectory("/opt/spark/data");
            if (a && b && c)
            {
                std::cout << "Directories created successfully." << std::endl;
            }
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
