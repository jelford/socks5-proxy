#ifndef SOCKS_SOCKSLISTENSTATE_HPP
#define SOCKS_SOCKSLISTENSTATE_HPP 

#include <memory>       // std::shared_ptr
#include <vector>       // std::vector
#include <tuple>        // std::tuple

#include <Socket.hpp>   // jelford::Socket
#include "HandshakeState.hpp" // socks::HandshakeState

#include <iostream>     // std::cerr

namespace socks
{
    class ListenState 
    {
        public:
            virtual std::vector<std::tuple<std::shared_ptr<jelford::Socket>, std::shared_ptr<SessionState>>>
            start_new_session(std::shared_ptr<jelford::Socket> socket)
            {
                std::cerr << "About to accept incoming connection" << std::endl;
                std::shared_ptr<jelford::Socket> incoming_connection(new jelford::Socket(socket->accept(NULL, NULL)));
                incoming_connection->set_nonblocking(true);
                std::shared_ptr<std::deque<unsigned char>> buffer(new std::deque<unsigned char>());

                std::cerr << "Connection accepted" << std::endl;

                std::cerr << "Initializing handshake state" << std::endl;
                std::shared_ptr<socks::HandshakeState> handshake_state(new HandshakeState(incoming_connection, buffer));

                std::vector<std::tuple<std::shared_ptr<jelford::Socket>, std::shared_ptr<SessionState>>> listen_list = {
                    std::make_tuple(incoming_connection, handshake_state)
                };
                return listen_list;
            }
    };
}

#endif
