#include <chrono>

#include "ParkingSpot.h"

ParkingSpot::ParkingSpot(size_t slot_id, cv::Rect coords) : coords(coords), is_online(false), is_occupied(false), slot_id(slot_id), last_change_time(std::nullopt) {}

ParkingSpot::ParkingSpot(size_t slot_id, cv::Rect coords, bool is_online, bool is_occupied) : coords(coords), is_online(is_online), is_occupied(is_occupied), slot_id(slot_id), last_change_time(std::nullopt) {}

ParkingSpot::ParkingSpot(size_t slot_id, cv::Rect coords, bool is_online, bool is_occupied, TimePoint last_change_time) : coords(coords), is_online(is_online), is_occupied(is_occupied), slot_id(slot_id), last_change_time(last_change_time) {}

void ParkingSpot::update_occupancy(bool is_occupied)
{
    // TODO: Should we update last_change_time on board initialization?
    // Imagine when board is booted and a the slot is empty (the default value). Then, last_change_time isn't updated
    this->is_online = true;
    if (is_occupied != this->is_occupied)
    {
        this->is_occupied = is_occupied;
        this->last_change_time = std::chrono::system_clock::now();
    }
}

std::ostream &
operator<<(std::ostream &os, const ParkingSpot &status)
{
    std::string duration_string = "";
    if (status.last_change_time.has_value())
    {
        const auto hours = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now() - status.last_change_time.value());
        const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now() - status.last_change_time.value());
        duration_string = std::to_string(hours.count()) + " hours " + std::to_string(minutes.count() % 60) + " minutes";
    }

    os << "{"
       << "slot_id: " << status.slot_id << ", "
       << "is_occupied:" << status.is_occupied << ", "
       << "last_change_time: " << duration_string
       << "}";
    return os;
}
