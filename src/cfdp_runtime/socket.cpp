#include <cfdp_core/utils.hpp>
#include <cfdp_runtime/socket.hpp>

#include <fcntl.h>

#include <bit>
#include <stdexcept>
#include <sys/fcntl.h>

namespace
{
// IP converter related retcodes.
constexpr int invalid_addr_in_addr_family = 0;

// File descriptor related retcodes.
constexpr int file_descriptor_error = -1;

// Socket related retcodes.
constexpr int socket_error = -1;
} // namespace

cfdp::runtime::sockets::Socket::Socket(SocketType sType, uint16_t port, const std::string& ipAddr)
    : handle(socket(AF_INET, utils::toUnderlying(sType), 0))
{
    if (handle == socket_error)
    {
        throw std::runtime_error{"Could not create a socket."};
    }

    addr.sin_family = AF_INET;
    addr.sin_port   = port;

    auto result = inet_pton(AF_INET, ipAddr.c_str(), &addr.sin_addr);

    if (result == invalid_addr_in_addr_family)
    {
        throw std::runtime_error{"Could not parse passed IP address."};
    }

    auto currentFlags = fcntl(handle, F_GETFL);

    if (currentFlags == file_descriptor_error)
    {
        throw std::runtime_error{"Could not fetch file descriptor flags."};
    }

    auto status = fcntl(handle, F_SETFL, currentFlags | O_NONBLOCK);

    if (status == file_descriptor_error)
    {
        throw std::runtime_error{"Could not set the socket to non-blocking."};
    }
}

void cfdp::runtime::sockets::UDPClient::sendMessage(std::vector<uint8_t> message)
{
    auto length = message.size();
    auto addr   = std::bit_cast<sockaddr*>(&socket.addr);

    auto result = sendto(socket.handle, message.data(), length, 0, addr, sizeof(socket.addr));

    if (result == socket_error)
    {
        throw std::runtime_error{"Could not send data via socket."};
    }
}

void cfdp::runtime::sockets::UDPServer::socketBind()
{
    auto addr   = std::bit_cast<sockaddr*>(&socket.addr);
    auto result = bind(socket.handle, addr, sizeof(socket.addr));

    if (result == socket_error)
    {
        throw std::runtime_error{"Could not bind the socket."};
    }
}
