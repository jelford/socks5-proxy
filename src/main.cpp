#include <iostream>

#include "Socket.hpp"


int main(int argc, char const * const * const argv)
{
    jelford::Socket socket(PF_INET, SOCK_STREAM, 0);
    socket.set_reuse(true);

    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(5905);
    sock_addr.sin_addr.s_addr = INADDR_ANY;

    socket.bind_to(sock_addr);

    socket.listen();

    auto connection = socket.accept(NULL, NULL);
    
    auto data = connection.read();
    for(auto i : data)
        std::cout << i;
    std::cout << std::endl;
    
    return EXIT_SUCCESS;
}
