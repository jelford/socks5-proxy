#ifndef SOCKS5_INITIAL_HANDSHAKE_MANAGER
#define SOCKS5_INITIAL_HANDSHAKE_MANAGER

#include "../socks.hpp"
#include "ISocksStateManager.hpp"

#include <exception>        // std::exception

namespace socks
{
    class MalformedHanshakeException : public SocksException
    {
        public:
            virtual const char * what()
            {
                return "Incoming handshake was malformed :(";
            }
    };

    namespace managers
    {
        template <typename Session, typename Socket>
        class Socks5InitialHandshakeManager : public ISocksStateManager<Session, Socket>
        {
            private:
                void set_auth_info(Session* session, Socket& connection)
                {
                    auto data = connection.read(2);
                    std::cout << "Received " << data.size() << " bytes" << std::endl;
                    if (data.size() < 2)
                        throw std::unique_ptr<SocksException>(new SocksException("Not enough data received to do authentication..."));
                    auto values = data.begin();
                    auto version = *values++;
                    unsigned char available_method_count = *values++;
                    std::cerr << "SOCKS version: " << version << std::endl;
                    std::cerr << "Available methods: " << static_cast<unsigned short>(available_method_count) << std::endl;

                    if (available_method_count < 1 || available_method_count > 256)
                    {
                        throw MalformedHanshakeException();
                    }

                    auto auth_methods = connection.read(0x0000 | available_method_count);
                 
                    for (auto method : auth_methods) 
                        session->add_auth_method(method);

                    std::cout << "Available auth methods: " ;
                    for(auto auth_method : session->get_auth_methods())
                        std::cout << socks::method_desc_from_id(auth_method) << ",\t";
                    std::cout << std::endl;

                }

                void send_auth_method(Session* session, Socket& connection)
                {
                    unsigned char method_reply_array[] = {0x05, session->get_auth_method()};
                    std::vector<unsigned char> method_reply(&method_reply_array[0], method_reply_array+sizeof(method_reply_array));

                    std::cerr << "Writing out auth method" << std::endl;
                    connection.write(method_reply);
                    std::cerr << "Auth method written" << std::endl;
                }

                void set_session_state_to_auth_handshake(Session* session)
                {
                    session->set_state_to_auth_handshake();
                }


            public:
                virtual void handle_request(Session* session, Socket& connection)
                {   
                    std::cerr << "Handling initial handshake" << std::endl;
                    set_auth_info(session, connection);
                    send_auth_method(session, connection);
                    set_session_state_to_auth_handshake(session);
                    std::cerr << "Finished initial handshake" << std::endl;
                }

               
        };
    }
}

#endif
