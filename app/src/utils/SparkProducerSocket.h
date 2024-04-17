#pragma once

#include <string>
#include <netinet/in.h>
#include <netdb.h>
#include <vector>

#include "ParkingSpot.h"

class SparkProducerSocket
{
public:
    SparkProducerSocket(const std::string &hostname_ipv6, uint16_t port);
    ~SparkProducerSocket();

    bool sendOccupancyData(const std::pair<int, int> &data);
    bool sendOccupancyData(const std::vector<ParkingSpot> &data);

private:
    int sockfd;
    std::string hostname_ipv6;
    uint16_t port;
    struct addrinfo *servinfo, *spark_addrinfo;
    // using hash to avoid sending the same data multiple times
    // If k1 == k2 is true, h(k1) == h(k2) is also true. (c++ standard)
    std::size_t last_telemetry_hash;
};
