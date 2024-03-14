/**
 * @file SparkProducerSocket.cpp
 * @brief Implementation of the SparkProducerSocket class.
 */

#include "SparkProducerSocket.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>

/**
 * @brief Constructs a SparkProducerSocket object with the specified host (IPv4 dotted-decimal format) and port.
 * @param host The host address to connect to.
 * @param port The port number to connect to.
 */
SparkProducerSocket::SparkProducerSocket(const std::string &host, int port)
    : sockfd(-1), host(host), port(port)
{
    int opt = 1;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    if (sockfd < 0)
    {
        throw std::runtime_error("Failed to create socket");
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0)
    {
        throw std::runtime_error("Invalid socket address/Address not supported. Address: " + host);
    }
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        throw std::runtime_error("Failed to bind the socket");
    }
    if (listen(sockfd, 3) == -1)
    {
        throw std::runtime_error("Failed to listen on the socket");
    }

    fd_set set;
    struct timeval timeout;

    FD_ZERO(&set);
    FD_SET(sockfd, &set);

    // wait 50 ms for incoming connection
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000; // 50 milliseconds
    int rv = select(sockfd + 1, &set, NULL, NULL, &timeout);

    if (rv == -1)
    {
        throw std::runtime_error("Error occurred while trying to select the socket");
    }
    else if (rv == 0)
    {
        throw std::runtime_error("Timeout occurred while trying to select the socket");
    }
    else
    {
        struct sockaddr_storage remoteAddr;
        socklen_t remoteAddrSize = sizeof(remoteAddr);
        latestSockfd = accept(sockfd, (struct sockaddr *)&remoteAddr, &remoteAddrSize);
    }
}

/**
 * @brief Destroys the SparkProducerSocket object.
 */
SparkProducerSocket::~SparkProducerSocket()
{
    if (sockfd >= 0)
    {
        std::cout << "Closing SPARK producer socket..." << std::endl;
        close(sockfd);
    }
}

/**
 * @brief Connects to the server with a timeout.
 * @param timeoutUSec The timeout value in microseconds.
 * @return True if the connection is successful, false otherwise.
 */
bool SparkProducerSocket::connectWithTimeout(int timeoutUSec)
{
    return false;
}

/**
 * @brief Sends data to the server.
 * @param data The data to be sent, represented as a pair of integers.
 * @return True if the data is sent successfully, false otherwise.
 */
bool SparkProducerSocket::sendData(const std::pair<int, int> &data)
{
    return false;
}
