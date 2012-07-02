#include <iostream>

#include "Socket.hpp"

jelford::SocketException::SocketException(int _errno) :  _errno(_errno) { }

const char* jelford::SocketException::what()
{
    return jelford::socket_get_error(_errno);
}

const char* jelford::socket_get_error(int _errno)
{
    switch(_errno) 
    {
        case EBADF:
            return "EBADF: Bad file descriptor";
        case ENFILE:
            return "ENFILE: File table overflow";
        case EINVAL:
            return "EINVAL: Invalid argument";
        case EMFILE:
            return "EMFILE: Too many open files";
        case ESPIPE:
            return "ESPIPE: Illegal seek";
        case EWOULDBLOCK:
            return "EWOULDBLOCK: Operation would block";
        default:
            return "Unknown socket error";
    }
}

jelford::Socket::Socket(int socket_family, int socket_type, int protocol) throw(SocketException)
{
    m_socket_descriptor = ::socket(socket_family, socket_type, protocol);
    if (m_socket_descriptor < 0)
    {
        throw SocketException(errno);
    }
}

jelford::Socket::Socket(int file_descriptor) : m_socket_descriptor(file_descriptor)
{
    std::cerr << m_socket_descriptor << " : opened socket" << std::endl;
}

jelford::Socket::Socket(Socket&& other) : m_socket_descriptor(other.m_socket_descriptor)
{
    // Don't close it when the old Socket is de-allocated
    other.m_socket_descriptor = 0;
}

void jelford::Socket::set_reuse(bool should_reuse)
{
    int optvalue = should_reuse;
    ::setsockopt(m_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof(optvalue));
}

void jelford::Socket::bind_to(sockaddr_in socket_address)
{
    if (-1 == ::bind(m_socket_descriptor, (sockaddr *)&socket_address, sizeof(socket_address)))
        throw SocketException(errno);
}

void jelford::Socket::listen()
{
    if (-1 == ::listen(m_socket_descriptor, 2))
        throw SocketException(errno);
}

jelford::Socket jelford::Socket::accept(sockaddr* addr, socklen_t* addrlen)
{
    return Socket(::accept(m_socket_descriptor, addr, addrlen));
}

jelford::Socket::~Socket()
{
    ::close(m_socket_descriptor);
    std::cerr << m_socket_descriptor << " : closing socket" << std::endl;
}

std::vector<unsigned char> jelford::Socket::read(size_t length)
{
    unsigned char buff[__chunk_size];
    ::memset(&buff, 0, sizeof(buff));
    std::vector<unsigned char> data;
    ssize_t read_length = 0;
    ssize_t remaining = length;
    size_t max_read = sizeof(buff) < static_cast<size_t>(remaining) ? sizeof(buff) : static_cast<size_t>(remaining);
    do
    {
        max_read = sizeof(buff) < static_cast<size_t>(remaining) ? sizeof(buff) : static_cast<size_t>(remaining);
        read_length = ::read(m_socket_descriptor, &buff, max_read);

        if (read_length < 0)
            throw SocketException(errno);

        remaining = remaining - read_length;
        data.insert(data.end(), &buff[0], buff+read_length);
    } while(remaining > 0 && max_read - read_length == 0);

    return data;
}

std::vector<unsigned char> jelford::Socket::read()
{
    std::vector<unsigned char> data;
    auto buff = read(__chunk_size * 10);
    while (buff.size() >= __chunk_size)
    {
        data.insert(data.end(), buff.begin(), buff.end());
        buff = read(__chunk_size);
    }
    data.insert(data.end(), buff.begin(), buff.end());
    return data;
}

