#ifndef SOCKS_SERVER_HPP
#define SOCKS_SERVER_HPP

#include "socks.hpp"
#include "SocksSession.hpp"
#include "factories/Socks5InitialSessionStateFactory.hpp"
#include "state_managers/Socks5InitialHandshakeManager.hpp"
#include "state_managers/Socks5NoAuthHandshakeManager.hpp"
#include "state_managers/Socks5IncomingConnectinoRequestsManager.hpp"
#include "state_managers/Socks5EstablishRemoteConnectionManager.hpp"
#include "state_managers/Socks5TunnelManager.hpp"

#include <memory>           // std::shared_ptr, std::unique_ptr
#include <thread>           // std::thread

namespace socks
{
    

    template <typename Socket>
    class SocksServer {
        private:
            Socket m_listen_socket;
            std::shared_ptr<factories::Socks5InitialSessionStateFactory<SocksSession<Socket>, 
                    Socket,
                    managers::Socks5InitialHandshakeManager<SocksSession<Socket>, Socket>,
                    managers::Socks5NoAuthHandshakeManager<SocksSession<Socket>, Socket>,
                    managers::Socks5IncomingConnectionRequestsManager<SocksSession<Socket>, Socket>,
                    managers::Socks5EstablishRemoteConnectionManager<SocksSession<Socket>, Socket>,
                    managers::Socks5TunnelManager<SocksSession<Socket>, Socket>
                >
            >
                        m_initial_state_factory;

        public:
            SocksServer(Socket&& listen_socket) : 
                m_listen_socket(std::move(listen_socket)),
                m_initial_state_factory(new factories::
                    Socks5InitialSessionStateFactory<
                        SocksSession<Socket>, 
                        Socket,
                        managers::Socks5InitialHandshakeManager<SocksSession<Socket>, Socket>,
                        managers::Socks5NoAuthHandshakeManager<SocksSession<Socket>, Socket>,
                        managers::Socks5IncomingConnectionRequestsManager<SocksSession<Socket>, Socket>,
                        managers::Socks5EstablishRemoteConnectionManager<SocksSession<Socket>, Socket>,
                        managers::Socks5TunnelManager<SocksSession<Socket>, Socket>
                    >())
            { }
            
            void serve()
            {
                int open_connections = 0;
                while(true)
                {
                    std::cerr << "Awaiting socks request..." << std::endl;
                    std::shared_ptr<SocksSession<Socket>> session(new SocksSession<Socket>(m_listen_socket.accept(NULL, NULL),
                                                    m_initial_state_factory));

                    std::cerr << ++open_connections << " open connections" << std::endl;
                    // Spin off a thread for every session
                    std::thread([this, session, &open_connections](){
                        /* Ooops! Assume session lasts forever and is blocking.
                         * Should a select(), or put this in a thread (and some shutdown logic)
                         */
                        try
                        {
                            while(session->process_incoming()) {} 
                        }
                        catch (std::unique_ptr<SocksException>&& e)
                        {
                            std::cerr << "Something went terrible wrong; I'm dropping the connection and printing out the error message:\n" << e->what() << std::endl;
                        }
                        catch (std::unique_ptr<jelford::SocketException>&& e)
                        {
                            std::cerr << "Socket problem: "
                                << e->retrieve_socket()->identify() << ": " << e->what() << std::endl;
                        }

                        std::cerr << "Finished socks request on socket " << m_listen_socket.identify() << std::endl;

                        std::cerr << --open_connections << " open connections" << std::endl;
                    }).detach();
                }
            }
    };
}

#endif
