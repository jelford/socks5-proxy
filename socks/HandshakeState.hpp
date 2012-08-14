#ifndef SOCKS_SOCKSHANDSHAKESTATE_HPP
#define SOCKS_SOCKSHANDSHAKESTATE_HPP

#include <memory>       // std::shared_ptr
#include <vector>       // std::vector
#include <tuple>        // std::tuple
#include <deque>      // std::deque, for buffer

#include <Socket.hpp>   // jelford::Socket
#include "socks/AuthHandshakeState.hpp" // socks::AuthHandshakeState

#include <iostream>     // std::cerr

namespace socks
{
    class HandshakeState : public SessionState
    {
        public:
            HandshakeState(std::shared_ptr<jelford::Socket> socket, std::shared_ptr<std::deque<unsigned char>> buffer)
                : SessionState(socket, buffer)
            {
            }

            virtual std::string identify() { return "HandshakeState"; }

            virtual decltype(m_no_change)
            consume_buffer()
            {

                std::cerr << "HandshakeState::handle_incoming_data { ";
                std::cerr << "buffer size: " << m_buffer->size() << " }" << std::endl;

                if (m_buffer->size() < 1)
                    return m_no_change;

                // buffer.size >= 1
                
                auto protocol = m_buffer->front();
                m_buffer->pop_front();
                std::cerr << "SOCKS PROTOCOL V" << static_cast<short>(protocol) << std::endl;
                std::shared_ptr<socks::AuthHandshakeState> auth_handshake_state(new AuthHandshakeState(m_socket, m_buffer));

                std::set<std::tuple<std::shared_ptr<jelford::Socket>, std::shared_ptr<SessionState>>> listen_list = {
                    std::make_tuple(m_socket, auth_handshake_state)
                };
                return std::tie(listen_list, m_no_write, m_no_exceptions);
            }
    };
}

#endif
