#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char const * const * const argv)
{
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        std::cout << "Error initializing socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Opened socket" << std::endl;

    int yes_please = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes_please, sizeof(yes_please)) > 0)
    {
        std::cout << "Error setting socket re-use" << std::endl;
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(5905);
    sock_addr.sin_addr.s_addr = INADDR_ANY;

    if (-1 == bind(socket_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)))
    {
        std::cout << "Could not bind to socket" << std::endl;
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Bind complete" << std::endl;

    if (-1 == listen(socket_fd, 2))
    {
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Listen complete" << std::endl;

    int connection_fd = accept(socket_fd, NULL, NULL);
    if (0 > connection_fd)
    {
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Accept complete" << std::endl;

    unsigned char in_buf[101];
    std::cout << "Read buffer size: " << sizeof(in_buf) << std::endl;
    ssize_t read_size;
    do
    {
        memset(&in_buf, 0, sizeof(in_buf));
        read_size = read(connection_fd, in_buf, sizeof(in_buf)-1);
        in_buf[100] = '\0';
        std::cout << read_size << ": " << in_buf << std::endl;

        std::cout << static_cast<int>(in_buf[read_size]) << std::endl;
    } while (read_size > 0 && static_cast<size_t>(read_size) >= sizeof(in_buf)-1);
    
    if (read_size == -1)
        std::cout << "READ SIZE -1" << std::endl;

    char const out_buf[13] = "hello, world";
    std::cout << "Write buffer size: " << sizeof(out_buf) << std::endl;
    ssize_t write_size = write(connection_fd, out_buf, sizeof(out_buf));
    std::cout << "Wrote " << write_size << " bytes" << std::endl;

    /*if (-1 == shutdown(connection_fd, SHUT_RDWR))
    {
        std::cout << "Shutdown problem (errno = " << errno << ")" << std::endl;
        close(connection_fd);
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Shutdown complete (errno = " << errno << ")" << std::endl;*/

    close(connection_fd);
    close(socket_fd);

    std::cout << "Sockets closed" << std::endl;
    
    return EXIT_SUCCESS;
}
