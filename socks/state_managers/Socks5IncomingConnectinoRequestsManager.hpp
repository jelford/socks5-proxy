#ifndef STATE_MANAGERS_SOCKS5INCOMINGCONNECTIONREQUESTSMANAGER_HPP
#define STATE_MANAGERS_SOCKS5INCOMINGCONNECTIONREQUESTSMANAGER_HPP

#include "../socks.hpp"
#include "ISocksStateManager.hpp"

#include <netinet/in.h>     // sockaddr_storage, sockaddr_in, sockaddr_in6

#include <memory>           // std::unique_ptr, which I'm using (and shouldn't be!) for exceptions.

#include <arpa/inet.h>      // inet_ntoa

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
                    switch (type)
                    {
                        case addr_type::IPV4:
                        {
                            std::cerr << "Trying to put ";
                            for(auto i : address)
                                std::cerr << (unsigned short)i << (i != address.back() ? "." : "");
                            std::cerr << ":";
                            std::cerr << (unsigned short)port[0] << (unsigned short)port[1] << " into a sockaddr_in" << std::endl;

                            std::unique_ptr<sockaddr> address_structure(new sockaddr());
                            auto addr_ipv4 = reinterpret_cast<sockaddr_in*>(address_structure.get());
                            
                            memcpy(&(addr_ipv4->sin_addr), &address[0], sizeof(addr_ipv4->sin_addr));
                            memcpy(&(addr_ipv4->sin_port), &port[0], sizeof(addr_ipv4->sin_port));
                            addr_ipv4->sin_family = AF_INET;

                            return std::shared_ptr<jelford::Address>(
                                new jelford::Address(
                                    std::move(address_structure), sizeof(*addr_ipv4), 0, AF_INET));
                        }
                        default:
                            throw jelford::AddressException(0,0);
                    }
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
