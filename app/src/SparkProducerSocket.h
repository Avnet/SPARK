#pragma once

#include <string>
#include <netinet/in.h>
#include <netdb.h>

class SparkProducerSocket
{
public:
    SparkProducerSocket(const std::string &hostname_ipv6, uint16_t port);
    ~SparkProducerSocket();

    bool sendOccupancyData(const std::pair<int, int> &data);

private:
    int sockfd;
    std::string hostname_ipv6;
    uint16_t port;
    struct addrinfo *servinfo;
};
