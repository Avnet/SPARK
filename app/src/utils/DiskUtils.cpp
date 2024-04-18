#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include "DiskUtils.h"

namespace
{
    using namespace cv;
    using namespace std;
    const std::string SPARK_ROIS_FILEPATH = "/opt/spark/data/rois.json";

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
    bool serializeROIs(const std::vector<ParkingSpot> &rois)
    {
        try
        {
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
                     << roi.coords
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

    vector<ParkingSpot> deserializeROIs()
    {
        try
        {
            FileStorage file(SPARK_ROIS_FILEPATH, FileStorage::READ);
            if (!file.isOpened())
            {
                std::cerr << "Failed to open file: " << SPARK_ROIS_FILEPATH << std::endl;
                return {};
            }

            vector<ParkingSpot> rois;
            FileNode roisNode = file["rois"];
            for (FileNodeIterator it = roisNode.begin(); it != roisNode.end(); ++it)
            {
                Rect roi;
                (*it)["roi"] >> roi;
                std::cout << "rois size " << roi << std::endl;
                rois.push_back(ParkingSpot(rois.size() + 1, roi));
            }
            file.release();
            if (!rois.empty())
            {
                std::cout << "ROIs read from disk." << std::endl;
            }
            return rois;
        }
        catch (std::exception &e)
        {
            std::cerr << "Failed to deserialize ROIs: " << e.what() << std::endl;
            return {};
        }
        catch (...)
        {
            std::cerr << "Failed to deserialize ROIs. Unknown Exception." << std::endl;
            return {};
        }
    }
}
