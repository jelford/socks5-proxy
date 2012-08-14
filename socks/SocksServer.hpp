#ifndef SOCKS_SOCKSSERVERHPP
#define SOCKS_SOCKSSERVERHPP


#include <memory>               // std::unique_ptr, std::shared_ptr
#include <map>                  // std::map
#include <set>                  // std::set
#include <tuple>                // std::tuple

#include <Socket.hpp>           // jelford::Socket

#include "SocksSessionState.hpp"    // socks::SocksSessionState
#include "SocksListenState.hpp"     // socks::ListenState

#include <iostream>             // std::cerr

namespace socks
{
    class SocksServer
    {
        private:
            std::shared_ptr<jelford::Socket> m_wrapped_socket;
            ListenState m_connection_listener;

            std::map<int, std::shared_ptr<SessionState>> m_read_sessions;
            std::set<std::shared_ptr<jelford::Socket>> m_read_sockets;
            std::set<std::shared_ptr<jelford::Socket>> m_write_sockets;
            std::map<int, std::shared_ptr<std::vector<unsigned char>>> m_write_data;
            std::set<std::shared_ptr<jelford::Socket>> m_exception_sockets; /* unused */

        public:
            SocksServer(std::shared_ptr<jelford::Socket> wrapped_socket)
                : m_wrapped_socket(wrapped_socket)
            {
                m_wrapped_socket->set_nonblocking(true);
                m_read_sockets.insert(m_wrapped_socket);
            }

            void serve()
            {
                try
                {
                    std::set<std::shared_ptr<SessionState>> dirty_sessions;
                    while(true)
                    {
                        std::cerr << "Checking " << m_read_sockets.size() << " sockets for read data: [";
                        for (auto s : m_read_sockets)
                            std::cerr << s->identify() << ", ";
                        std::cerr << "] and " << m_write_sockets.size() << " sockets for write data: [";
                        for (auto s : m_write_sockets)
                            std::cerr << s->identify() << ", ";
                        std::cerr << "] " << std::endl;

                        std::set<std::shared_ptr<jelford::Socket>> sockets_with_read_data, sockets_for_writing, sockets_with_exception_data;

                        std::tie(sockets_with_read_data, sockets_for_writing, sockets_with_exception_data) = jelford::select(m_read_sockets, m_write_sockets, m_exception_sockets);

                        for (auto r_socket : sockets_with_read_data)
                        {
                            if (r_socket == m_wrapped_socket)
                            {
                                auto new_mapping = m_connection_listener.start_new_session(r_socket);
                                for (auto s : new_mapping)
                                {
                                    auto socket = std::get<0>(s);
                                    auto session = std::get<1>(s);
                                    m_read_sessions[socket->identify()] = session;
                                    m_read_sockets.insert(socket);
                                    session->handle_incoming_data();
                                    dirty_sessions.insert(session);
                                }
                            }
                            else
                            {
                                auto session = m_read_sessions[r_socket->identify()];
                                try
                                {
                                    session->handle_incoming_data();
                                    dirty_sessions.insert(session);
                                }
                                catch (std::unique_ptr<jelford::SocketException>&& e)
                                {
                                    std::cerr << "Some problem trying to read data:" << std::endl; 
                                    std::cerr << e->what() << std::endl;
                                }
                            }
                        }

                        while (dirty_sessions.size() > 0)
                        {
                            auto tmp = dirty_sessions;
                            for (auto d_session : tmp)
                            {
                                std::cerr << "Processing [" << d_session->identify() << "]" << std::endl;
                                dirty_sessions.erase(d_session);
                                std::set<std::tuple<std::shared_ptr<jelford::Socket>, std::shared_ptr<SessionState>>> read_mappings, exception_mappings;
                                std::set<std::tuple<std::shared_ptr<jelford::Socket>, std::shared_ptr<std::vector<unsigned char>>>> write_mappings;
                                if (d_session.get() == NULL)
                                {
                                    std::cerr << "I spy with my little eye a NULL-state" << std::endl;
                                    return;
                                }
                                std::tie(read_mappings, write_mappings, exception_mappings) = d_session->consume_buffer();

                                bool had_removals = false;
                                for (auto mapping : read_mappings)
                                {
                                    std::cerr << "Processing new read" << std::endl;

                                    std::shared_ptr<jelford::Socket> sock;
                                    std::shared_ptr<SessionState> state;
                                    std::tie(sock, state) = mapping;
                                    if (state.get() == NULL)
                                    {
                                        std::cerr << "Erasing socket: " << sock->identify() << std::endl;
                                        m_read_sockets.erase(sock);
                                        m_read_sessions.erase(sock->identify());
                                        had_removals = true;
                                    }
                                    else
                                    {
                                        m_read_sessions[sock->identify()] = state;
                                        m_read_sockets.insert(sock);
                                        
                                        dirty_sessions.insert(state);
    
                                        std::cerr << "Further (read) processing on socket: " << sock->identify() << " (state: " << state->identify() << ")" << std::endl;
                                    }
                                }

                                for (auto mapping : write_mappings)
                                {
                                    auto target_socket = std::get<0>(mapping);
                                    auto data = std::get<1>(mapping);

                                    auto to_write = m_write_data[target_socket->identify()];
                                    if (to_write.get() == NULL)
                                        to_write = m_write_data[target_socket->identify()] = decltype(to_write)(new std::vector<unsigned char>());
                                    
                                    m_write_sockets.insert(target_socket);
                                    to_write->insert(to_write->end(), data->begin(), data->end());

                                    
                                    std::cerr << "Queuing " << data->size() << " bytes for writing to socket " << target_socket->identify() << std::endl;

                                }

                                if (had_removals)
                                    break;
                            }
                        }

                        for (auto w_socket : sockets_for_writing) 
                        {
                            std::cerr << "Writing to socket " << w_socket->identify() << std::endl;
                            auto to_write = m_write_data[w_socket->identify()];

                            if (to_write.get() != NULL)
                            {
                                w_socket->write(*to_write);
                                m_write_data[w_socket->identify()].reset();
                            }

                            if (to_write.get() == NULL || to_write->size() == 0)
                            {
                                std::cerr << "Erasing (write) socket: " << w_socket->identify() << std::endl;
                                m_write_data.erase(w_socket->identify());
                                m_write_sockets.erase(w_socket);

                            }
                            
                        }

                        std::cerr << "New read watch set:" << std::endl;
                        for (auto s : m_read_sockets)
                            std::cerr << s->identify() << ", ";
                        std::cerr << std::endl;

                    }
                }
                catch (std::unique_ptr<jelford::SocketException>&& e)
                {
                    std::cerr << "Problem in Socket[" << e->retrieve_socket()->identify() << "]" << std::endl;
                    std::cerr << e->what() << std::endl;
                }
            }
    };
}


#endif

