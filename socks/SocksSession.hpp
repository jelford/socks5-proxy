#ifndef SOCKS_SESSION_HPP
#define SOCKS_SESSION_HPP

#include "socks.hpp"
#include "state_managers/ISocksStateManager.hpp"
#include "factories/ISocksStateFactory.hpp"

#include <memory>       // unique_ptr
#include <exception>    // exception
#include <sys/socket.h> // SOCK_STREAM

#include <netinet/in.h>     // sockaddr_storage, sockaddr_in, sockaddr_in6...

namespace socks
{
    class SocksSessionException : public std::exception
    {
        private:
            char const * const msg;
        public:
            SocksSessionException(char const * const msg) : msg(msg)
            { }
    };
    
    template <typename Socket>
    class SocksSession
    {
        private:
            Socket _m_connection;
            std::vector<unsigned char> auth_methods;
            std::shared_ptr<factories::ISocksStateFactory<SocksSession<Socket>, Socket>> m_state_factory;
            std::unique_ptr<managers::ISocksStateManager<SocksSession<Socket>, Socket>> m_state;
     
            SocksSession(SocksSession&) = delete;  
            bool should_continue;
            std::shared_ptr<jelford::Address> outbound_address;
            unsigned char m_address_type;
            std::shared_ptr<Socket> _m_outbound;
            int m_method; // SOCK_STREAM/SOCK_DGRAM
        public:

            SocksSession(Socket&& connection, 
                    std::shared_ptr<factories::ISocksStateFactory<SocksSession<Socket>, Socket>> state_factory) : 
                _m_connection(std::move(connection)),           // We own the connection
                m_state_factory(state_factory),
                m_state(state_factory->get_initial()),           // Set up state
                should_continue(true),
                outbound_address(NULL),
                _m_outbound(NULL),
                m_method(SOCK_STREAM)
            {
            }

            bool process_incoming()
            {
                m_state->handle_request(this, _m_connection);
                return should_continue;
            }


            unsigned char get_auth_method()
            {
                if (std::find(auth_methods.begin(), auth_methods.end(), AUTH_NONE) != auth_methods.end())
                    return AUTH_NONE;
                else
                {
                    std::cerr << "Uh oh, couldn't find no-auth in list of available auth methods. Going to crash..." << std::endl;
                    throw new SocksSessionException("No supported auth methods available");
                }
                    
            }

            std::vector<unsigned char> get_auth_methods()
            {
                return auth_methods;
            }

            void add_auth_method(unsigned char method)
            {
                auth_methods.push_back(method);
            }

            void set_outbound_address(std::shared_ptr<jelford::Address> address)
            {
                outbound_address = address;
            }

            std::shared_ptr<jelford::Address> get_outbound_address()
            {
                return outbound_address;
            }

            constexpr unsigned char get_version_number() const
            {
                return 0x05;
            }

            void set_address_type(const unsigned char type)
            {
                std::cerr << "Session manager: address type set to: " << type << std::endl;
                m_address_type = type;
            }

            unsigned char get_address_type() const
            {
                return m_address_type;
            }

            void set_outbound_connection(std::shared_ptr<Socket> outbound)
            {
                _m_outbound = outbound;
            }

            std::shared_ptr<Socket> get_outbound_connection()
            {
                return _m_outbound;
            }
    
            int get_connection_method()
            {
                return m_method;
            }

            /* State transitions! */

            void set_state_to_auth_handshake()
            {
                std::cerr << "state -> auth handshake" << std::endl;
                m_state.reset(m_state_factory->get_auth_handshake_state());
            }

            void set_state_to_expect_requests()
            {
                std::cerr << "state -> expecting requests" << std::endl;
                m_state.reset(m_state_factory->get_incoming_requests_state());
            }

            void set_state_to_close()
            {
                std::cerr << "state -> close" << std::endl;
                should_continue = false;
            }

            void set_state_to_connect()
            {
                std::cerr << "state -> establish remote connection" << std::endl;
                m_state.reset(m_state_factory->get_establish_remote_connection_state());
                m_method = SOCK_STREAM;
            }
    
            void set_state_to_bind()
            {
                std::cerr << "state -> bind" << std::endl;
                throw std::unique_ptr<SocksException>(new SocksException("Not implemented"));
            }
        
            void set_state_to_associate()
            {
                std::cerr << "state -> associate" << std::endl;
                throw std::unique_ptr<SocksException>(new SocksException("Not implemented"));
            }

            void set_state_to_tunnel()
            {
                std::cerr << "state -> tunnel" << std::endl;
                m_state.reset(m_state_factory->get_tunnel_state());
            }
    };
}

#endif
