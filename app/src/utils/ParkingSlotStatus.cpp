#include "ParkingSlotStatus.h"

ParkingSlotStatus::ParkingSlotStatus(int slot_id, bool is_occupied) : slot_id(slot_id), is_occupied(is_occupied)
{
    last_change_time = std::nullopt;
}

ParkingSlotStatus::ParkingSlotStatus(int slot_id, bool is_occupied, std::chrono::system_clock::time_point last_change_time) : slot_id(slot_id), is_occupied(is_occupied), last_change_time(last_change_time)
{
}

std::ostream &operator<<(std::ostream &os, const ParkingSlotStatus &status)
{
    os << "{"
       << "slot_id: " << status.slot_id << ", "
       << "is_occupied:" << status.is_occupied << ", "
       << "last_change_time: " << (status.last_change_time.has_value() ? std::to_string(std::chrono::system_clock::to_time_t(status.last_change_time.value())) : "")
       << "}";
    return os;
}
