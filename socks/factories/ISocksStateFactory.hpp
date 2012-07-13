#ifndef FACTORIES_ISOCKSSTATEFACTORY_HPP
#define FACTORIES_ISOCKSSTATEFACTORY_HPP

#include "../state_managers/ISocksStateManager.hpp"

namespace socks
{
    namespace factories
    {
        template <typename SessionType, typename SocketType>
        class ISocksStateFactory
        {
            public:
                virtual managers::ISocksStateManager<SessionType, SocketType>* get_initial() = 0;
                virtual managers::ISocksStateManager<SessionType, SocketType>* get_auth_handshake_state() = 0;
                virtual managers::ISocksStateManager<SessionType, SocketType>* get_incoming_requests_state() = 0;

                virtual managers::ISocksStateManager<SessionType, SocketType>* get_establish_remote_connection_state() = 0;
                virtual managers::ISocksStateManager<SessionType, SocketType>* get_tunnel_state() = 0;
        };
    }
}

#endif
