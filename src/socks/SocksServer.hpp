#ifndef SOCKS_SERVER_HPP
#define SOCKS_SERVER_HPP

#include "SocksSession.hpp"
#include "factories/Socks5InitialSessionStateFactory.hpp"

namespace socks
{
    

    template <typename Socket>
    class SocksServer {
        private:
            Socket m_listen_socket;
            factories::Socks5InitialSessionStateFactory<SocksSession<Socket>, Socket> m_initial_state_factory;

        public:
            SocksServer(Socket&& listen_socket) : m_listen_socket(std::move(listen_socket))
            { }
            
            void serve()
            {
                while(true)
                {
                    std::cerr << "Awaiting socks request..." << std::endl;
                    SocksSession<Socket> session(m_listen_socket.accept(NULL, NULL), 
                                                    m_initial_state_factory);
                    std::cerr << "Finished socks request on socket " << m_listen_socket.identify() << std::endl;
                }
            }
    };
}

#endif
