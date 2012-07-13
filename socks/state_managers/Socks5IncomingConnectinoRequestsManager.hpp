#ifndef STATE_MANAGERS_SOCKS5INCOMINGCONNECTIONREQUESTSMANAGER_HPP
#define STATE_MANAGERS_SOCKS5INCOMINGCONNECTIONREQUESTSMANAGER_HPP

#include "../socks.hpp"
#include "ISocksStateManager.hpp"

#include <netinet/in.h>     // sockaddr_storage, sockaddr_in, sockaddr_in6

#include <memory>           // std::unique_ptr, which I'm using (and shouldn't be!) for exceptions.

namespace socks
{
    namespace managers
    {
        namespace addr_type
        {
            const unsigned char IPV4 = 0x01;
            const unsigned char DOMAIN_NAME = 0x03;
            const unsigned char IPV6 = 0x04;
        
            template <typename Socket>
            size_t length(const unsigned char addr_type, Socket& connection) throw(SocksException)
            {
                switch(addr_type)
                {
                    case IPV4:
                        return 0x0004;
                    case DOMAIN_NAME:
                        return connection.read(1)[0];
                    case IPV6:
                        return 0x0010;
                    default:
                        throw std::unique_ptr<SocksException>(new SocksException());
                }
            }

        }

        namespace command_type
        {
            static const unsigned char CONNECT = 0x01;
            static const unsigned char BIND = 0x02;
            static const unsigned char ASSOCIATE = 0x03;
        }

        template <typename Session, typename Socket>
        class Socks5IncomingConnectionRequestsManager : public ISocksStateManager<Session, Socket>
        {

            private:
                std::shared_ptr<jelford::Address> addr_for_request(const unsigned char type, const std::vector<unsigned char> address, const std::vector<unsigned char> port)
                {
                    /*sockaddr_storage addr;
                    sockaddr_storage* addr_ptr = &addr;
                    switch(type)
                    {
                        case addr_type::IPV4:
                        {
                            // Sorry! sockaddr_storage is big enough to hold either
                                an inet4 or inet6 address, but isn't a plain-old union
                                type. So reinterpret_casts ahoy. //
                            sockaddr_in* ipv4_addr = reinterpret_cast<sockaddr_in*>(addr_ptr);
                            ipv4_addr->sin_family = AF_INET;
                            ipv4_addr->sin_port = static_cast<unsigned short>(*(&port[0]));
                            ipv4_addr->sin_addr = *(in_addr*)(&address[0]);
                            return std::shared_ptr<jelford::Address>(new jelford::Address(*ipv4_addr));
                        }
                        case addr_type::DOMAIN_NAME:
                        {
                            throw std::unique_ptr<SocksException>(new SocksException("Address by domain name unimplemented"));
                        }
                        case addr_type::IPV6:
                        {
                            throw std::unique_ptr<SocksException>(new SocksException("IPV6 addresses unimplemented"));
                        }
                        default:
                            throw std::unique_ptr<SocksException>(new SocksException("Unknown address type"));
                    }
                    */
                    addrinfo hints;
                    memset(&hints, 0, sizeof(hints));
                    return std::shared_ptr<jelford::Address>(new jelford::Address("127.0.0.1", "5600", hints));
                }

                void set_next_session_state(Session* session, unsigned char command)
                {
                    switch(command)
                    {
                        case command_type::CONNECT:
                            session->set_state_to_connect();
                            break;
                        case command_type::BIND:
                            session->set_state_to_bind();
                            break;
                        case command_type::ASSOCIATE:
                            session->set_state_to_associate();
                            break;
                        default:
                            throw std::unique_ptr<SocksException>(new SocksException("What is wrong with you? I can't handle that command!"));
                    }
                }
    
            public:
                virtual void handle_request(Session* session, Socket& connection)
                {
                    auto version = connection.read(1);
                    auto command = connection.read(1)[0];
                    /*auto reserved =*/ connection.read(1)[0]; // Why? Who knows...
                    auto address_type = connection.read(1);

                    std::cerr << "Command: " << command << std::endl;
                    std::cerr << "Address type: " << address_type[0] << std::endl;

                    auto address_length = addr_type::length(address_type[0], connection);
                    std::cerr << "Address length: " << address_length << std::endl;

                    
                    auto address = connection.read(address_length);
                    auto port = connection.read(2);

                    address.push_back('\0');
                    std::cerr << "Address: ";
                    for (size_t i=0; i<address_length; ++i)
                        std::cerr << static_cast<unsigned int>(address[i]);
                    std::cerr << ":" << static_cast<int>(port[0]) << static_cast<int>(port[1]) << " (port length: " << port.size() << ")" << std::endl;
                    address.pop_back();
                    
                    auto sock_address = addr_for_request(address_type[0], address, port);
                    std::cerr << "Setting address type: " << std::endl;
                    session->set_address_type(address_type[0]);
                    
                    session->set_outbound_address(sock_address);
                    set_next_session_state(session, command);
                }
        };
    }
}
#endif
