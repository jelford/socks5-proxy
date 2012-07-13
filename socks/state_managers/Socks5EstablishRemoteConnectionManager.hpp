#ifndef STATE_MANAGER_SOCKS5ESTABLISHREMOTECONNECTIONMANAGER
#define STATE_MANAGER_SOCKS5ESTABLISHREMOTECONNECTIONMANAGER

#include <vector>       // std::vector
#include <memory>       // std::shared_ptr

namespace socks
{
    namespace managers
    {
        template <typename Session, typename Socket>
        class Socks5EstablishRemoteConnectionManager : public ISocksStateManager<Session, Socket>
        {
            private:

            public:
                virtual void handle_request(Session* session, Socket& connection)
                {   
                    std::cerr << "Establishing remote connection" << std::endl;

                    /* attempt connection */
                    auto outbound_address = session->get_outbound_address();
                    std::shared_ptr<Socket> outbound_socket(new Socket(outbound_address->family, session->get_connection_method(), 0));
                    outbound_socket->connect(*outbound_address);
                    session->set_outbound_connection(outbound_socket);

                    auto connection_was_successful = true;

                    auto v_number = session->get_version_number();
                    auto success_code = connection_was_successful ? 0x00 : 0x01;
                    auto reserved = 0x00;
                    auto addr_type_vector = session->get_address_type();

                    auto server_bound_addr = {0x00, 0x00, 0x00, 0x00}; /* TODO: should be localhost? */
                    auto server_port = {0x00, 0x00};

                    std::vector<unsigned char> message;
                    message.push_back(v_number);
                    message.push_back(success_code);
                    message.push_back(reserved);
                    message.push_back(addr_type_vector);
                    for (auto c : server_bound_addr)
                        message.push_back(c);
                    for (auto c : server_port)
                        message.push_back(c);
        
                
                    connection.write(message);
                    
                    if (connection_was_successful)
                        session->set_state_to_tunnel();
                    else
                        session->set_state_to_close();
                }

               
        };
    }
}

#endif
