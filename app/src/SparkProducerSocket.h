#pragma once

#include <string>
#include <netinet/in.h>

class SparkProducerSocket
{
public:
    SparkProducerSocket(const std::string &host, int port);
    ~SparkProducerSocket();

    bool connectWithTimeout(int timeoutUSec);
    bool sendData(const std::pair<int, int> &data);

private:
    int sockfd, latestSockfd;
    std::string host;
    int port;
    struct sockaddr_in serverAddr;
};
