#include <vector>
#include <exception>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


namespace jelford 
{
    class SocketException : public std::exception
    {   
        private:
            int _errno;
        public:
            SocketException(int _errno);
            
            virtual const char* what();
    };

    const char* socket_get_error(int _errno);

    // Requires sys/socket.h
    class Socket
    {
        private:
            static const size_t __chunk_size = 1024;
            int m_socket_descriptor;
            Socket(Socket&) = delete;
        public:
            Socket(int socket_family, int socket_type, int protocol) throw(SocketException);
            Socket(int file_descriptor);
            Socket(Socket&& other);
            
            void set_reuse(bool should_reuse);
            
            void bind_to(sockaddr_in socket_address);
            
            void listen();
            Socket accept(sockaddr* addr, socklen_t* addrlen);

            virtual ~Socket();
            
            std::vector<unsigned char> read(size_t length);
            
            std::vector<unsigned char> read();
    };
}
