#pragma once

#include <string>
#include <netinet/in.h>
#include <netdb.h>
#include <vector>
#include <chrono>

#include "ParkingSpot.h"

class SparkProducerSocket
{
public:
    SparkProducerSocket(const std::string &hostname_ipv6, uint16_t port, const std::chrono::milliseconds min_transmit_period = std::chrono::milliseconds(1000));
    ~SparkProducerSocket();

    bool sendOccupancyData(const std::pair<int, int> &data);
    bool sendOccupancyDataDebounced(const std::vector<ParkingSpot> &data);

private:
    int sockfd;
    std::string hostname_ipv6;
    uint16_t port;
    struct addrinfo *servinfo, *spark_addrinfo;

    std::chrono::milliseconds min_transmit_period;
    std::chrono::time_point<std::chrono::system_clock> next_transmit_time;
};
