#ifndef SOCKS_TRAFFICFORWARDSTATE_HPP
#define SOCKS_TRAFFICFORWARDSTATE_HPP

#include <Socket.hpp>   // jelford::Socket
#include <iostream>

#include "SocksSessionState.hpp"

#include <deque>        // std::deque

#include <netinet/in.h> // ::SOCK_STREAM

#include <sstream>

namespace socks
{
    class TrafficForwardState : public SessionState
    {
        private:
            std::shared_ptr<jelford::Socket> m_destination;
            bool m_data_sent;
        public:
            TrafficForwardState(std::shared_ptr<jelford::Socket> inbound,
                std::shared_ptr<std::deque<unsigned char>> buffer,
                std::shared_ptr<jelford::Socket> destination)
                :   SessionState(inbound, buffer), m_destination(destination),
                    m_data_sent(false)
            {   }

            virtual std::string identify() 
            { 
                std::stringstream s;
                s << "TrafficForwardState[" << m_socket->identify() << "->" << m_destination->identify() << "]"; 
                return s.str();
            }
            
            virtual decltype(m_no_change)
            consume_buffer()
            {
                std::shared_ptr<std::vector<unsigned char>> data(new std::vector<unsigned char>(m_buffer->begin(), m_buffer->end()));
                std::deque<unsigned char> empty;
                m_buffer->swap(empty);

                decltype(m_no_read) read_mappings;
                decltype(m_no_write) write_mappings;

                if (data->size() == 0 && m_data_sent)
                {
                    read_mappings.insert(
                        {
                            std::make_tuple(m_socket, std::shared_ptr<SessionState>(NULL)), 
                            std::make_tuple(m_destination, std::shared_ptr<SessionState>(NULL))
                        });
                    write_mappings.insert(std::make_tuple(m_destination, std::shared_ptr<std::vector<unsigned char>>(new std::vector<unsigned char>())));
                }
                else if (data->size() > 0 || m_data_sent)
                    write_mappings.insert(std::make_tuple(m_destination, data));

                m_data_sent = true;

                return std::tie(read_mappings, write_mappings, m_no_exceptions);

            }
    };
}
#endif
