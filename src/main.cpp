#include <vector>
#include <iostream>
#include <cassert>      // assert
#include <algorithm>    // std::find

#include <cstring>      // memset

#include <netinet/in.h> // AF_INET, PF_INET, SOCK_STREAM -- socket types

#include "Socket.hpp"   // High level socket wrapper

#include <string>       // std::string

namespace socks 
{
    static const unsigned char AUTH_NONE = 0x00;
    static const unsigned char AUTH_GSSAPI = 0x01;
    static const unsigned char AUTH_USERPASS = 0x02;

    std::string method_desc_from_id(unsigned char id)
    {
        switch(id)
        {
            case AUTH_NONE:
                return "No Auth";
            case AUTH_GSSAPI:
                return "GSSAPI";
            case AUTH_USERPASS:
                return "Username/password";
        }
        if (id > 0x03 && id < 0x80)
            return "IANA defined";
        else if (id < 0xFF)
            return "Privately defined";
        else
            return "None acceptable";
    }

    class SocksSession
    {
        private:
            jelford::Socket m_connection;
            std::vector<unsigned char> auth_methods;

            void receive_establish_request()
            {
                auto data = m_connection.read();
                std::cout << "Received " << data.size() << " bytes" << std::endl;
                auto values = data.begin();
                auto version = *values++;
                unsigned short available_method_count = *values++;
                std::cout << "SOCKS version: " << version << std::endl;
                std::cout << "Available methods: " << available_method_count << std::endl;
                
                assert (data.size() >= available_method_count + 2);
              
                for(; values != data.end(); ++values)
                    auth_methods.push_back(*values);

                std::cout << "Available auth methods: " ;
                for(auto auth_method : auth_methods)
                    std::cout << socks::method_desc_from_id(auth_method) << ",\t";
                std::cout << std::endl;
            }

       
        public:

            SocksSession(jelford::Socket&& connection) : m_connection(std::move(connection))
            {
                this->receive_establish_request();
            }

            void do_auth_handshake()
            {
                unsigned char method_reply_array[] = {0x05, 0x00};
                std::vector<unsigned char> method_reply(&method_reply_array[0], method_reply_array+sizeof(method_reply_array));

                std::cerr << "Writing out auth method" << std::endl;
                if (std::find(auth_methods.begin(), auth_methods.end(), socks::AUTH_NONE) != auth_methods.end())
                    m_connection.write(method_reply);
                    //std::cerr << "not writing to socket... " << std::endl;
                else
                    std::cerr << "No Auth unavailable..." << std::endl;

                std::cerr << "Auth method written" << std::endl;
            }
    };

    class SocksServer {
        private:
            jelford::Socket m_listen_socket;

        public:
            SocksServer(jelford::Socket&& listen_socket) : m_listen_socket(std::move(listen_socket))
            { }
            
            void serve()
            {
                while(true)
                {
                    SocksSession session(m_listen_socket.accept(NULL, NULL));
                    session.do_auth_handshake();
                }
            }
    };
}


int main(int argc, char const * const * const argv)
{
    jelford::Socket socket(PF_INET, SOCK_STREAM, 0);
    socket.set_reuse(true);

    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(5905);
    sock_addr.sin_addr.s_addr = INADDR_ANY;

    socket.bind_to(sock_addr);

    socket.listen();

    socks::SocksServer server(std::move(socket));
    server.serve();

    return EXIT_SUCCESS;
}
