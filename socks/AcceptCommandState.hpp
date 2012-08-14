

#ifndef SOCKS_SOCKSACCEPTCOMMANDSTATE_HPP
#define SOCKS_SOCKSACCEPTCOMMANDSTATE_HPP

#include <memory>       // std::shared_ptr
#include <vector>       // std::vector
#include <tuple>        // std::tuple
#include <deque>      // std::deque, for buffer

#include <Socket.hpp>   // jelford::Socket
#include "AuthHandshakeState.hpp" // socks::AuthHandshakeState
#include "AcceptCommandState.hpp" // socks::AcceptCommandState
#include "EstablishConnectionStateFactory.hpp" // socks::EstablishConnectionStateFactory
#include "ProcessAddressStates.hpp" // socks::ProcessIPv4AddressState, socks::ProcessDomainAddressState

#include <iostream>     // std::cerr

namespace socks
{
    class AcceptCommandState : public SessionState
    {
        public:
            AcceptCommandState(std::shared_ptr<jelford::Socket> socket, std::shared_ptr<std::deque<unsigned char>> buff)
                : SessionState(socket, buff)
            {
                std::cerr << "AcceptCommandState::AcceptCommandState { buffer_size: " << m_buffer->size() << " }" << std::endl;
            }

            virtual std::string identify() { return "AcceptCommandState"; }

            virtual decltype(m_no_change)
            consume_buffer()
            {
                std::cerr << "AcceptCommandState::handle_incoming_data()" << std::endl;
                std::cerr << "(buffer size: " << m_buffer->size() << ")" << std::endl;

                if (m_buffer->size() < 4)
                    return m_no_change;

                /* auto version = m_buffer->front(); */
                m_buffer->pop_front();
                auto command = m_buffer->front();
                m_buffer->pop_front();
                /* auto reserved = m_buffer->front();  -- this is just a 0-byte*/
                m_buffer->pop_front();
                auto address_type = m_buffer->front();
                m_buffer->pop_front();

                std::shared_ptr<ProcessCommandStateFactory> process_command_state_factory;
                switch (command)
                {
                    case 0x01:
                        std::cerr << "Preping a TCP CONNECT state factory" << std::endl;
                        process_command_state_factory = std::shared_ptr<ProcessCommandStateFactory>(new EstablishConnectionStateFactory(m_socket, m_buffer));
                        break;
                    case 0x02:
                        std::cerr << "BIND commands unsupported! Oh dear..." << std::endl;
                        break;
                    case 0x03:
                        std::cerr << "UDP associate commands unsupported! Oh dear..." << std::endl;
                        break;
                    default:
                        std::cerr << "Unknown command from client (" << static_cast<short>(command) << ")! Oh dear..." << std::endl;
                        break;
                }

                std::shared_ptr<SessionState> process_address_state;
                switch (address_type)
                {
                    case 0x01:
                        std::cerr << "Preping an IPV4 address processor" << std::endl;
                        process_address_state = std::shared_ptr<SessionState>(new ProcessIPv4AddressState(m_socket, m_buffer, process_command_state_factory));
                        break;
                    case 0x03:
                        process_address_state = std::shared_ptr<SessionState>(new ProcessDomainAddressState(m_socket, m_buffer, process_command_state_factory));
                        break;
                    case 0x04:
                        std::cerr << "IPV6 not yet supported! Oh dear..." << std::endl;
                        break;
                    default:
                        std::cerr << "Unknown address type from client (" << static_cast<short>(address_type) << ")! Oh dear..." << std::endl;
                        break;
                }
                
                std::set<std::tuple<std::shared_ptr<jelford::Socket>, std::shared_ptr<SessionState>>> listen_list = {
                    std::make_tuple(m_socket, process_address_state)
                };
                return std::tie(listen_list, m_no_write, m_no_exceptions);
            }
    };
}

#endif
