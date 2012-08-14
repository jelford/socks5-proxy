#ifndef SOCKS_ESTABLISHCONNECTIONSTATE_HPP
#define SOCKS_ESTABLISHCONNECTIONSTATE_HPP

#include <Socket.hpp>   // jelford::Socket
#include <iostream>

#include "SocksSessionState.hpp"
#include "TrafficForwardState.hpp"

#include <deque>        // std::deque

#include <netinet/in.h> // ::SOCK_STREAM

namespace socks
{
    class EstablishConnectionState : public SessionState
    {
        private:
            std::shared_ptr<jelford::Address> m_address;
        
        public:
            EstablishConnectionState(std::shared_ptr<jelford::Socket> socket,
                std::shared_ptr<std::deque<unsigned char>> client_buffer,
                std::shared_ptr<jelford::Address> address)
                :   SessionState(socket, client_buffer), m_address(address)
            {   }

            virtual std::string identify() { return "EstablishConnectionState"; }

            
            virtual decltype(m_no_change)
            consume_buffer()
            {
                std::shared_ptr<jelford::Socket> outgoing_traffic(new jelford::Socket(m_address->family, ::SOCK_STREAM, m_address->protocol));
                outgoing_traffic->set_nonblocking(true);

                unsigned char connection_success;

                try
                {
                    outgoing_traffic->connect(m_address->address.get(), m_address->address_length);
                    connection_success = 0x00;
                }
                catch (std::unique_ptr<jelford::SocketException>&& e)
                {
                    connection_success = 0x01;
                }

                std::vector<unsigned char> reply{
                            0x05, // verion
                            connection_success,
                            0x00, // reserved
                            0x01, // ipv4
                            0x00, 0x00, 0x00, 0x00, // placeholder ipv4
                            0x00, 0x00 // port
                    };
                m_socket->write(reply);

                std::shared_ptr<SessionState> outgoing_traffic_handler(new TrafficForwardState(m_socket, m_buffer, outgoing_traffic));

                std::shared_ptr<SessionState> incoming_traffic_handler(new TrafficForwardState(outgoing_traffic, std::shared_ptr<std::deque<unsigned char>>(new std::deque<unsigned char>()), m_socket));

                std::cerr << "Incoming/Outgoing on " << m_socket->identify() << "/" << outgoing_traffic->identify() << std::endl;

                std::set<std::tuple<std::shared_ptr<jelford::Socket>, std::shared_ptr<SessionState>>> read_mappings{
                    std::make_tuple(m_socket, outgoing_traffic_handler),
                    std::make_tuple(outgoing_traffic, incoming_traffic_handler)
                };
                return std::tie(read_mappings, m_no_write, m_no_exceptions);
            }
    };
}
#endif
