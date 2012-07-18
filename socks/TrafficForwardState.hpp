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
                s << "TrafficForwardState[" << m_socket->identify() << "]"; 
                return s.str();
            }
            
            virtual std::vector<std::tuple<std::shared_ptr<jelford::Socket>, std::shared_ptr<SessionState>>>
            consume_buffer()
            {
                bool error = false;
                std::vector<unsigned char> data(m_buffer->begin(), m_buffer->end());
                std::deque<unsigned char> empty;
                m_buffer->swap(empty);
                wait_for_write(m_destination.get());
                try
                {
                    m_destination->write(data);
                }
                catch (std::unique_ptr<jelford::SocketException>&& e)
                {
                    auto s = e->retrieve_socket();
                    std::cerr << s->identify() << ": Problem forwarding data: " << e->what() << std::endl;
                    error = true;
                }

                std::vector<std::tuple<std::shared_ptr<jelford::Socket>, std::shared_ptr<SessionState>>> mappings;
                if ((data.size() == 0 && m_data_sent) || error)
                {
                    mappings.push_back(std::make_tuple(m_destination, std::shared_ptr<SessionState>(NULL)));
                    mappings.push_back(std::make_tuple(m_socket, std::shared_ptr<SessionState>(NULL)));
                }
                    
                m_data_sent = true;
                return mappings;

                

            }
    };
}
#endif
