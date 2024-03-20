/**
 * @file SparkProducerSocket.cpp
 * @brief Implementation of the SparkProducerSocket class.
 */

#include "SparkProducerSocket.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * @brief Constructs a SparkProducerSocket object with the specified host (IPv4 dotted-decimal format) and port.
 * @param hostname_ipv6 The host address to connect to.
 * @param port The port number to connect to.
 */
SparkProducerSocket::SparkProducerSocket(const std::string &hostname_ipv6, uint16_t port)
    : sockfd(-1), hostname_ipv6(hostname_ipv6), port(port)
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
 * @brief Sends occupancy data to the server.
 * @param data The data to be sent, represented as a pair of integers.
 * @return True if the data is sent successfully, false otherwise.
 */
bool SparkProducerSocket::sendOccupancyData(const std::pair<int, int> &data)
{
    std::string payload = std::to_string(data.first) + "," + std::to_string(data.second) + "\n";

    int total = 0;
    int numbytes;
    int bytes_remaining = payload.length();
    while (total < payload.length())
    {
        numbytes = sendto(sockfd, payload.c_str() + total, bytes_remaining, 0, spark_addrinfo->ai_addr, spark_addrinfo->ai_addrlen);
        if (numbytes == -1)
        {
            break;
        }
        total += numbytes;
        bytes_remaining -= numbytes;
    }

    return numbytes == -1 ? false : true;
}
