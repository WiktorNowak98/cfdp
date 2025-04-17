#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>

#include <cstdint>
#include <string>
#include <vector>

namespace cfdp::runtime::sockets
{
enum class SocketType
{
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM,
};

class Socket
{
  public:
    Socket(SocketType sType, uint16_t port, const std::string& ipAddr);

    int handle;
    sockaddr_in addr{};
};

class UDPClient
{
  public:
    UDPClient(uint16_t port = 8000, const std::string& ipAddr = "127.0.0.1")
        : socket(SocketType::UDP, port, ipAddr)
    {}

    void sendMessage(std::vector<uint8_t> message);

  private:
    Socket socket;
};

class UDPServer
{
  public:
    UDPServer(uint16_t port = 8000, const std::string& ipAddr = "0.0.0.0")
        : socket(SocketType::UDP, port, ipAddr)
    {}

    void socketBind();

  private:
    Socket socket;
};
} // namespace cfdp::runtime::sockets
