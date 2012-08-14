#ifndef SOCKS_PROCESSADDRESSSTATES_HPP
#define SOCKS_PROCESSADDRESSSTATES_HPP

#include "SocksSessionState.hpp"
#include "EstablishConnectionStateFactory.hpp"

#include <memory>       // std::shared_ptr
#include <deque>        // std::deque
#include <tuple>        // std::tuple

#include <cstring>      // memcpy
#include <netinet/in.h> // Address stuff

#include <iostream>

namespace socks
{
    class ProcessIPv4AddressState : public SessionState
    {
        private:
            std::shared_ptr<ProcessCommandStateFactory> m_command_process_state_factory;
        public:
            ProcessIPv4AddressState(std::shared_ptr<jelford::Socket> socket, std::shared_ptr<std::deque<unsigned char>> buffer, std::shared_ptr<ProcessCommandStateFactory> command_process_state_factory)
                : SessionState(socket, buffer), m_command_process_state_factory(command_process_state_factory)
            {   }

            virtual std::string identify() { return "ProcessIPv4AddressState"; }

            virtual decltype(m_no_change)
            consume_buffer()
            {
                std::cerr << "ProcessIPv4AddressState::handle_incoming_data()" << std::endl;
                std::cerr << "(buffer size: " << m_buffer->size() << ")" << std::endl;

                if (m_buffer->size() < 6)
                    return m_no_change;
               
                std::unique_ptr<sockaddr> address(new sockaddr());
                
                {
                    auto inet4_address = reinterpret_cast<sockaddr_in*>(address.get());
                    inet4_address->sin_family = AF_INET;
                    memcpy(&(inet4_address->sin_addr), &(m_buffer->front()), 4);
                    m_buffer->erase(m_buffer->begin(), m_buffer->begin()+4);
                    memcpy(&(inet4_address->sin_port), &(m_buffer->front()), 2);
                    m_buffer->erase(m_buffer->begin(), m_buffer->begin()+2);
                }

                socklen_t size = sizeof(sockaddr_in);
                auto protocol = 0;
                auto family = AF_INET;

                std::shared_ptr<jelford::Address> outgoing_address(new jelford::Address(std::move(address), size, protocol, family));
                
                auto command_process_state = m_command_process_state_factory->set_address(outgoing_address);
            
                std::set<std::tuple<std::shared_ptr<jelford::Socket>, std::shared_ptr<SessionState>>> read_mappings{std::make_tuple(m_socket, command_process_state)};
                return std::tie(read_mappings, m_no_write, m_no_exceptions);
            }
    };

    class ProcessDomainAddressState : public SessionState
    {
        private:
            std::shared_ptr<ProcessCommandStateFactory> m_command_process_state_factory;

        public:
            ProcessDomainAddressState(std::shared_ptr<jelford::Socket> socket, std::shared_ptr<std::deque<unsigned char>> buffer, std::shared_ptr<ProcessCommandStateFactory> command_process_state_factory)
                : SessionState(socket, buffer), m_command_process_state_factory(command_process_state_factory)
            {   }

            virtual std::string identify() { return "ProcessDomainAddressState"; }

            virtual decltype(m_no_change)
            consume_buffer()
            {
                std::cerr << "DOMAIN ADDRESS TYPE NOT YET SUPPORTED! Oh dear..." << std::endl;

                return m_no_change;
            }

    };
}

#endif
