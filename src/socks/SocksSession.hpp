#ifndef SOCKS_SESSION_HPP
#define SOCKS_SESSION_HPP

#include "socks.hpp"
#include "state_managers/ISocksStateManager.hpp"
#include "factories/ISocksStateFactory.hpp"

#include <memory>       // unique_ptr
#include <exception>    // exception

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
            std::unique_ptr<managers::ISocksStateManager<SocksSession<Socket>, Socket>> m_state;
            std::vector<unsigned char> auth_methods;
     
            SocksSession(SocksSession&) = delete;  
        public:

            SocksSession(Socket&& connection, 
                    factories::ISocksStateFactory<SocksSession<Socket>, Socket>& initial_state_factory) : 
                _m_connection(std::move(connection)),           // We own the connection
                m_state(initial_state_factory.get())           // Set up state
            {
                process_incoming();
            }

            void process_incoming()
            {
                m_state->handle_request(this, _m_connection);
            }


            unsigned char get_preferred_auth_method()
            {
                if (std::find(auth_methods.begin(), auth_methods.end(), AUTH_NONE) != auth_methods.end())
                    return AUTH_NONE;
                else
                {
                    std::cerr << "Uh oh, couldn't find no-auth in list of available auth methods. Going to crash..." << std::endl;
                    throw SocksSessionException("No supported auth methods available");
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
    };
}

#endif
