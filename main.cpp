#include <memory>       // std::shared_ptr
#include <cstring>      // memset

#include <netinet/in.h> // AF_INET, PF_INET, SOCK_STREAM -- socket types

#include "Socket.hpp"   // High level socket wrapper
#include "socks/SocksServer.hpp" // Socks server (wraps a socket and manages sessions)

#include <string>       // std::string
#include <signal.h>

using namespace jelford;
using namespace socks;

int main(int argc, char const * const * const argv)
{
    std::shared_ptr<Socket> socket(new Socket(PF_INET, SOCK_STREAM, 0));
    socket->set_reuse(true);

    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(5905);
    sock_addr.sin_addr.s_addr = INADDR_ANY;

    socket->bind_to(sock_addr);

    socket->listen();

    SocksServer server(socket);
    server.serve();

    return EXIT_SUCCESS;
}
