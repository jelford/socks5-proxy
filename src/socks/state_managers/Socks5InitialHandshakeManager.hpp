#ifndef SOCKS5_INITIAL_HANDSHAKE_MANAGER
#define SOCKS5_INITIAL_HANDSHAKE_MANAGER

#include "ISocksStateManager.hpp"

namespace socks
{
    namespace managers
    {
        template <typename Session, typename Socket>
        class Socks5InitialHandshakeManager : public ISocksStateManager<Session, Socket>
        {
            private:
                void set_auth_info(Session* session, Socket& connection)
                {
                    auto data = connection.read();
                    std::cout << "Received " << data.size() << " bytes" << std::endl;
                    auto values = data.begin();
                    auto version = *values++;
                    unsigned char available_method_count = *values++;
                    std::cout << "SOCKS version: " << version << std::endl;
                    std::cout << "Available methods: " << static_cast<unsigned short>(available_method_count) << std::endl;
                    
                    assert (data.size() >= (0x0000 & available_method_count) + 2);
                  
                    for(; values != data.end(); ++values)
                        session->add_auth_method(*values);

                    std::cout << "Available auth methods: " ;
                    for(auto auth_method : session->get_auth_methods())
                        std::cout << socks::method_desc_from_id(auth_method) << ",\t";
                    std::cout << std::endl;

                }

                void send_auth_method(Session* session, Socket& connection)
                {
                    unsigned char method_reply_array[] = {0x05, session->get_preferred_auth_method()};
                    std::vector<unsigned char> method_reply(&method_reply_array[0], method_reply_array+sizeof(method_reply_array));

                    std::cerr << "Writing out auth method" << std::endl;
                    connection.write(method_reply);
                    std::cerr << "Auth method written" << std::endl;
                }


            public:
                virtual void handle_request(Session* session, Socket& connection)
                {
                    set_auth_info(session, connection);
                    send_auth_method(session, connection);
                }

               
        };
    }
}

#endif
