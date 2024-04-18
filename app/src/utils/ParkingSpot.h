#pragma once

#include <optional>
#include <chrono>
#include <iostream>
#include <opencv2/core.hpp>

class ParkingSpot
{
    using TimePoint = std::chrono::system_clock::time_point;

public:
    ParkingSpot() = delete;
    ParkingSpot(const ParkingSpot &other) = default;

    ParkingSpot(size_t slot_id, cv::Rect coords);
    ParkingSpot(size_t slot_id, cv::Rect coords, bool is_online, bool is_occupied);
    ParkingSpot(size_t slot_id, cv::Rect coords, bool is_online, bool is_occupied, std::chrono::system_clock::time_point last_change_time);
    friend std::ostream &operator<<(std::ostream &os, const ParkingSpot &status);

    void update_occupancy(bool is_occupied);

    cv::Rect coords;
    bool is_online;
    bool is_occupied;
    size_t slot_id;

private:
    // a parking space is online if DRP-AI is running inference for it
    std::optional<TimePoint> last_change_time;
};
