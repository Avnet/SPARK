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
#include <bitset>

#include "SparkProducerSocket.h"

namespace
{
    int get_1_2_8_9_encoding(const std::vector<ParkingSpot> &data)
    {
        if (data.size() < 0)
        {
            return 0;
        }

        // Due to IoT connect limitations, we will encode parking spots 1, 2, 8, and 9 positions
        // into the 4 LSB of the encoding integer
        // = b[xxxxxxxx xxxxxxxx xxxxxxxx xxxxssss] = encoding(data)
        // where x is do not care
        // where ssss is the occupancy status of parking spots 1, 2, 8, and 9 respectively
        auto data_size = data.size();
        int encoding = 0;

        encoding |= data_size >= 1 ? data[0].is_occupied << 3 : 0;
        encoding |= data_size >= 2 ? data[1].is_occupied << 2 : 0;
        encoding |= data_size >= 8 ? data[7].is_occupied << 1 : 0;
        encoding |= data_size >= 9 ? data[8].is_occupied : 0;

        std::cout << "1289 encoding: " << std::bitset<32>(encoding) << std::endl;

        return encoding;
    }

    int get_3_4_10_11_encoding(const std::vector<ParkingSpot> &data)
    {
        if (data.size() < 0)
        {
            return 0;
        }

        // Due to IoT connect limitations, we will encode parking spots 3, 4, 10, and 11 positions
        // into the 4 LSB of the encoding integer
        // = b[xxxxxxxx xxxxxxxx xxxxxxxx xxxxssss] = encoding(data)
        // where x is do not care
        // where ssss is the occupancy status of parking spots 3, 4, 10, and 11 respectively
        auto data_size = data.size();
        int encoding = 0;

        encoding |= data_size >= 3 ? data[2].is_occupied << 3 : 0;
        encoding |= data_size >= 4 ? data[3].is_occupied << 2 : 0;
        encoding |= data_size >= 10 ? data[9].is_occupied << 1 : 0;
        encoding |= data_size >= 11 ? data[10].is_occupied : 0;
        std::cout << "341011 encoding: " << std::bitset<32>(encoding) << std::endl;
        return encoding;
    }

    int get_5_6_12_13_encoding(const std::vector<ParkingSpot> &data)
    {
        if (data.size() < 0)
        {
            return 0;
        }

        auto data_size = data.size();
        int encoding = 0;

        // Due to IoT connect limitations, we will encode parking spots 5, 6, 12, and 13 positions
        // into the 4 LSB of the encoding integer
        // = b[xxxxxxxx xxxxxxxx xxxxxxxx xxxxssss] = encoding(data)
        // where x is do not care
        // where ssss is the occupancy status of parking spots 5, 6, 12, and 13 respectively
        encoding |= data_size >= 5 ? data[4].is_occupied << 3 : 0;
        encoding |= data_size >= 6 ? data[5].is_occupied << 2 : 0;
        encoding |= data_size >= 12 ? data[11].is_occupied << 1 : 0;
        encoding |= data_size >= 13 ? data[12].is_occupied : 0;

        std::cout << "561213 encoding: " << std::bitset<32>(encoding) << std::endl;
        return encoding;
    }

    int get_7_x_14_x_encoding(const std::vector<ParkingSpot> &data)
    {
        if (data.size() < 0)
        {
            return 0;
        }

        auto data_size = data.size();
        int encoding = 0;
        // Due to IoT connect limitations, we will encode parking spots 7, 14 positions
        // into the 4 LSB of the encoding integer
        // = b[xxxxxxxx xxxxxxxx xxxxxxxx xxxxsxsx] = encoding(data)
        // where x is do not care
        // where sxsx is the occupancy status of parking spots 7, 14 respectively (note the dont care bits in between)
        encoding |= data_size >= 7 ? data[6].is_occupied << 3 : 0;
        encoding |= data_size >= 14 ? data[13].is_occupied << 1 : 0;
        std::cout << "7x14x encoding: " << std::bitset<32>(encoding) << std::endl;
        return encoding;
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
    payload << "\"psStatus1_2_8_9\": " << std::to_string(get_1_2_8_9_encoding(data)) << ",";
    // underlying datatype: int
    payload << "\"psStatus3_4_10_11\": " << std::to_string(get_3_4_10_11_encoding(data)) << ",";
    // underlying datatype: int
    payload << "\"psStatus5_6_12_13\": " << std::to_string(get_5_6_12_13_encoding(data)) << ",";
    // underlying datatype: int
    payload << "\"psStatus7_14\": " << std::to_string(get_7_x_14_x_encoding(data)) << ",";
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
        std::cout << "Sent telemetry: " << payload_str << std::endl;
    }
    return success;
}
