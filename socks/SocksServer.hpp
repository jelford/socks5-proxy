#ifndef SOCKS_SOCKSSERVERHPP
#define SOCKS_SOCKSSERVERHPP


#include <memory>               // std::unique_ptr, std::shared_ptr
#include <map>                  // std::map
#include <set>                  // std::set

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
            std::map<int, std::shared_ptr<SessionState>> m_socks_sessions;
            std::set<std::shared_ptr<jelford::Socket>> m_known_sockets;

        public:
            SocksServer(std::shared_ptr<jelford::Socket> wrapped_socket)
                : m_wrapped_socket(wrapped_socket)
            {
                m_wrapped_socket->set_nonblocking(true);
                m_known_sockets.insert(m_wrapped_socket);
            }

            void serve()
            {
                try
                {
                    std::set<std::shared_ptr<SessionState>> dirty_sessions;
                    while(true)
                    {
                        std::cerr << "Checking " << m_known_sockets.size() << " sockets for read data: [";
                        for (auto s : m_known_sockets)
                            std::cerr << s->identify() << ", ";
                        std::cerr << "]" << std::endl;

                        auto sockets_with_read_data = select_for_reading(m_known_sockets);

                        for (auto r_socket : sockets_with_read_data)
                        {
                            if (r_socket == m_wrapped_socket)
                            {
                                auto new_mapping = m_connection_listener.start_new_session(r_socket);
                                for (auto s : new_mapping)
                                {
                                    auto socket = std::get<0>(s);
                                    auto session = std::get<1>(s);
                                    m_socks_sessions[socket->identify()] = session;
                                    m_known_sockets.insert(socket);
                                    session->handle_incoming_data();
                                    dirty_sessions.insert(session);
                                }
                            }
                            else
                            {
                                auto session = m_socks_sessions[r_socket->identify()];
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
                                dirty_sessions.erase(d_session);
                                auto new_mapping = d_session->consume_buffer();

                                bool had_removals = false;
                                for (auto mapping : new_mapping)
                                {
                                    std::shared_ptr<jelford::Socket> sock;
                                    std::shared_ptr<SessionState> state;
                                    std::tie(sock, state) = mapping;
                                    if (state.get() == NULL)
                                    {
                                        m_known_sockets.erase(sock);
                                        m_socks_sessions.erase(sock->identify());
                                        had_removals = true;
                                    }
                                    else
                                    {
                                        m_socks_sessions[sock->identify()] = state;
                                        m_known_sockets.insert(sock);
                                        dirty_sessions.insert(state);
                                    }
                                }
                                if (had_removals)
                                    break;
                            }
                        }
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

