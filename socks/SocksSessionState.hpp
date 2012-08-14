#ifndef SOCKS_SOCKSSESSIONSTATE_HPP
#define SOCKS_SOCKSSESSIONSTATE_HPP

#include <memory>       // std::shared_ptr
#include <vector>       // std::vector
#include <tuple>        // std::tuple
#include <deque>        // std::deque

#include <Socket.hpp>   // jelford::Socket

#include <iostream>

namespace socks
{
    class SessionState
    {
        protected:
            std::shared_ptr<jelford::Socket> m_socket;
            std::shared_ptr<std::deque<unsigned char>> m_buffer;

            std::set<std::tuple<std::shared_ptr<jelford::Socket>, std::shared_ptr<SessionState>>> m_no_read;
            std::set<std::tuple<std::shared_ptr<jelford::Socket>, std::shared_ptr<std::vector<unsigned char>>>> m_no_write;
            std::set<std::tuple<std::shared_ptr<jelford::Socket>, std::shared_ptr<SessionState>>> m_no_exceptions;

            std::tuple<
                decltype(m_no_read), 
                decltype(m_no_write), 
                decltype(m_no_exceptions)> 
                    m_no_change;

   
        public:
            SessionState(std::shared_ptr<jelford::Socket> socket, std::shared_ptr<std::deque<unsigned char>> buff)
                : m_socket(socket), m_buffer(buff)
            { }

            virtual decltype(m_no_change)
            consume_buffer()=0;

            virtual std::string identify()=0;

            bool
            handle_incoming_data()
            {
                auto data = m_socket->read();
                m_buffer->insert(m_buffer->end(), data.begin(), data.end());
                std::cerr << this->identify() << ": Received " << data.size() << " bytes of data" << std::endl;
                return !(data.size()==0);
            }
    };
}

#endif
