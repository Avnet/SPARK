/**
 * @file SparkProducerSocket.cpp
 * @brief Implementation of the SparkProducerSocket class.
 */

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <numeric>

#include "SparkProducerSocket.h"

namespace
{
    int get_1289(const std::vector<ParkingSpot> &data)
    {
        return 0;
    }
    int get_341011(const std::vector<ParkingSpot> &data)
    {
        return 0;
    }
    int get_561213(const std::vector<ParkingSpot> &data)
    {
        return 0;
    }
    int get_714(const std::vector<ParkingSpot> &data)
    {
        return 0;
    }
}

/**
 * @brief Constructs a SparkProducerSocket object with the specified host (IPv4 dotted-decimal format) and port.
 * @param hostname_ipv6 The host address to connect to.
 * @param port The port number to connect to.
 */
SparkProducerSocket::SparkProducerSocket(const std::string &hostname_ipv6, uint16_t port)
    : sockfd(-1), hostname_ipv6(hostname_ipv6), port(port), servinfo(nullptr), spark_addrinfo(nullptr), last_telemetry_hash(0)
{
    int opt = 1;
    struct addrinfo hints, *p;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(hostname_ipv6.c_str(), std::to_string(port).c_str(), &hints, &servinfo)) != 0)
    {
        throw std::runtime_error("getaddrinfo: " + std::string(gai_strerror(rv)));
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            std::cerr << "Producer socket" << std::endl;
            continue;
        }

        int yes = 1;
        // Configure reusable port
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        {
            throw std::runtime_error("Setsockopt(SO_REUSEADDR) failed");
        }
        spark_addrinfo = p;
        break;
    }

    if (p == NULL)
    {
        throw std::runtime_error("Failed to create socket");
    }
    std::cout << "SPARK producer socket created" << std::endl;
}

/**
 * @brief Destroys the SparkProducerSocket object.
 */
SparkProducerSocket::~SparkProducerSocket()
{
    freeaddrinfo(servinfo);
    if (sockfd >= 0)
    {
        std::cout << "Closing SPARK producer socket..." << std::endl;
        close(sockfd);
    }
}

/**
 * @brief DEPRECATED IN FAVOR OF "sendOccupancyData(const std::vector<ParkingSpot> &data)"
 * Sends occupancy data to the server.
 * @param data The data to be sent, represented as a pair of integers (taken, empty).
 * @return True if the data is sent successfully, false otherwise.
 */
bool SparkProducerSocket::sendOccupancyData(const std::pair<int, int> &data)
{
    std::string payload = std::to_string(data.first) + "," + std::to_string(data.second) + "\n";

    const auto cur_hash = std::hash<std::string>{}(payload);
    if (cur_hash == last_telemetry_hash)
    {
        return false;
    }

    int total = 0;
    int bytes_sent;
    int bytes_remaining = payload.length();
    while (total < payload.length())
    {
        bytes_sent = sendto(sockfd, payload.c_str() + total, bytes_remaining, 0, spark_addrinfo->ai_addr, spark_addrinfo->ai_addrlen);
        if (bytes_sent == -1)
        {
            break;
        }
        total += bytes_sent;
        bytes_remaining -= bytes_sent;
    }
    bool success = bytes_sent != -1;
    if (success)
    {
        last_telemetry_hash = cur_hash;
    }
    return success;
}

/// @brief Sends relevant telemetry to SPARK Datagram socket. If you have that socket configured with IoT connect, you can see the data in the IoT connect dashboard.
/// @param data Parking spots data, but for demo purposes, only 14 slots matter
/// @return True if the data is sent successfully, false otherwise.
bool SparkProducerSocket::sendOccupancyData(const std::vector<ParkingSpot> &data)
{

    const auto taken = std::accumulate(begin(data), end(data), 0, [](const int &acc, const ParkingSpot &ps)
                                       { return acc + (ps.is_occupied ? 1 : 0); });
    const auto empty = data.size() - taken;

    // Generating the payload according to a specific IoT Connect Demo target for Boston, SF, and Germany demos
    std::stringstream payload;
    payload << "{";
    // underlying datatype: int
    payload << "\"psStatus1_2_8_9\": " << std::to_string(get_1289(data)) << ",";
    // underlying datatype: int
    payload << "\"psStatus3_4_10_11\": " << std::to_string(get_341011(data)) << ",";
    // underlying datatype: int
    payload << "\"psStatus5_6_12_13\": " << std::to_string(get_561213(data)) << ",";
    // underlying datatype: int
    payload << "\"psStatus7_14\": " << std::to_string(get_714(data)) << ",";
    // underlying datatype: int
    payload << "\"taken\": " << std::to_string(taken) << ",";
    // underlying datatype: int
    payload << "\"empty\": " << std::to_string(empty) << ",";
    // underlying datatype: latlong
    payload << "\"location\": [0, 0]";
    payload << "}";

    int total = 0;
    int bytes_sent;
    int bytes_remaining = payload.str().length();
    // need to ensure lifetime of string is long enough to be sent ota
    const auto payload_str = payload.str();

    const auto cur_hash = std::hash<std::string>{}(payload_str);
    if (cur_hash == last_telemetry_hash)
    {
        return false;
    }

    while (total < payload.str().length())
    {
        bytes_sent = sendto(sockfd, payload_str.c_str() + total, bytes_remaining, 0, spark_addrinfo->ai_addr, spark_addrinfo->ai_addrlen);
        if (bytes_sent == -1)
        {
            break;
        }
        total += bytes_sent;
        bytes_remaining -= bytes_sent;
    }
    bool success = bytes_sent != -1;
    if (success)
    {
        last_telemetry_hash = cur_hash;
    }
    return success;
}
