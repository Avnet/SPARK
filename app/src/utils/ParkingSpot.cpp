#include "ParkingSpot.h"

ParkingSpot::ParkingSpot(size_t slot_id, cv::Rect coords) : coords(coords), is_online(false), is_occupied(false), slot_id(slot_id), last_change_time(std::nullopt) {}

ParkingSpot::ParkingSpot(size_t slot_id, cv::Rect coords, bool is_online, bool is_occupied) : coords(coords), is_online(is_online), is_occupied(is_occupied), slot_id(slot_id), last_change_time(std::nullopt) {}

ParkingSpot::ParkingSpot(size_t slot_id, cv::Rect coords, bool is_online, bool is_occupied, TimePoint last_change_time) : coords(coords), is_online(is_online), is_occupied(is_occupied), slot_id(slot_id), last_change_time(last_change_time) {}

std::ostream &operator<<(std::ostream &os, const ParkingSpot &status)
{
    os << "{"
       << "slot_id: " << status.slot_id << ", "
       << "is_occupied:" << status.is_occupied << ", "
       << "last_change_time: " << (status.last_change_time.has_value() ? std::to_string(std::chrono::system_clock::to_time_t(status.last_change_time.value())) : "")
       << "}";
    return os;
}
