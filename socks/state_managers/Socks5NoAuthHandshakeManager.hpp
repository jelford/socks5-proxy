#ifndef STATE_MANAGER_SOCKS5NOAUTHHANDSHAKEMANAGER_HPP
#define STATE_MANAGER_SOCKS5NOAUTHHANDSHAKEMANAGER_HPP

#include <iostream>     // std::cerr

namespace socks
{
    namespace managers
    {
        template <typename Session, typename Socket>
        class Socks5NoAuthHandshakeManager : public ISocksStateManager<Session, Socket>
        {
            public:
                virtual void handle_request(Session* session, Socket& connection)
                {
                    std::cerr << "Doing nothing when I could be doing an auth request" << std::endl;
                    session->set_state_to_expect_requests();
                    
                }
        };
    }
}
#endif
