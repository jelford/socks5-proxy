#ifndef SOCKS_ESTABLISHCONNECTIONSTATEFACTORY_HPP
#define SOCKS_ESTABLISHCONNECTIONSTATEFACTORY_HPP

#include <memory>           // std::shared_ptr
#include <deque>            // std::deque
#include "SocksSessionState.hpp"
#include "EstablishConnectionState.hpp"
#include <Socket.hpp>

#include <iostream>         // std::cerr

namespace socks
{
    class ProcessCommandStateFactory
    {
        public:
            virtual std::shared_ptr<SessionState> set_address(std::shared_ptr<jelford::Address> address)=0;
    };

    class EstablishConnectionStateFactory : public ProcessCommandStateFactory
    {
        private:
            std::shared_ptr<jelford::Socket> m_socket;
            std::shared_ptr<std::deque<unsigned char>> m_buffer;
        public:
            EstablishConnectionStateFactory(std::shared_ptr<jelford::Socket> socket, std::shared_ptr<std::deque<unsigned char>> buffer)
                : m_socket(socket), m_buffer(buffer)
            {   }

            virtual std::shared_ptr<SessionState> set_address(std::shared_ptr<jelford::Address> address)
            {
                return std::shared_ptr<SessionState>(new EstablishConnectionState(m_socket, m_buffer, address));
            }
    };
}
#endif
