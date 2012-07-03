#ifndef SOCKS5_INITIAL_SESSION_STATE_FACTORY_HPP
#define SOCKS5_INITIAL_SESSION_STATE_FACTORY_HPP

#include "../state_managers/ISocksStateManager.hpp"
#include "../state_managers/Socks5InitialHandshakeManager.hpp"

namespace socks
{
    namespace factories
    {
        template <typename SessionType, typename SocketType>
        class Socks5InitialSessionStateFactory : 
            public ISocksStateFactory<SessionType, SocketType>
        {
            public:
                virtual managers::ISocksStateManager<SessionType, SocketType>* get()
                {
                    return new managers::Socks5InitialHandshakeManager<SessionType, SocketType>();
                }
        };
    }
}

#endif
