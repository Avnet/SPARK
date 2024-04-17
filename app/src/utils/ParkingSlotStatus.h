#pragma once
#include <optional>
#include <chrono>
#include <iostream>

using TimePoint = std::chrono::system_clock::time_point;
class ParkingSlotStatus
{
public:
    ParkingSlotStatus() = delete;
    ParkingSlotStatus(int slot_id, bool is_occupied);
    ParkingSlotStatus(int slot_id, bool is_occupied, std::chrono::system_clock::time_point last_change_time);
    friend std::ostream &operator<<(std::ostream &os, const ParkingSlotStatus &status);

private:
    int slot_id;
    bool is_occupied;
    std::optional<TimePoint> last_change_time;
};
