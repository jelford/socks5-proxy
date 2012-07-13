#ifndef SOCKS5_INITIAL_SESSION_STATE_FACTORY_HPP
#define SOCKS5_INITIAL_SESSION_STATE_FACTORY_HPP

#include "../state_managers/ISocksStateManager.hpp"

namespace socks
{
    namespace factories
    {
        template <typename SessionType, typename SocketType,
                    typename InitialHandshakeManagerType,
                    typename AuthHandshakeManagerType,
                    typename IncomingRequestsManagerType,
                    typename EstablishRemoteConnectionManagerType,
                    typename TunnelManagerType>
        class Socks5InitialSessionStateFactory : 
            public ISocksStateFactory<SessionType, SocketType>
        {
            public:
                virtual managers::ISocksStateManager<SessionType, SocketType>* get_initial()
                {
                    return new InitialHandshakeManagerType();
                }

                virtual managers::ISocksStateManager<SessionType, SocketType>* get_auth_handshake_state()
                {
                    return new AuthHandshakeManagerType();
                }

                virtual managers::ISocksStateManager<SessionType, SocketType>* get_incoming_requests_state()
                {
                    return new IncomingRequestsManagerType();
                }

                virtual managers::ISocksStateManager<SessionType, SocketType>* get_establish_remote_connection_state()
                {
                    return new EstablishRemoteConnectionManagerType();
                };

                virtual managers::ISocksStateManager<SessionType, SocketType>* get_tunnel_state()
                {
                    return new TunnelManagerType();
                };               
        };
    }
}

#endif
