#ifndef STATE_MANAGER_SOCKS5TUNNELMANAGER
#define STATE_MANAGER_SOCKS5TUNNELMANAGER

#include <vector>       // std::vector

#include <iostream>     // std::cerr

namespace socks
{
    namespace managers
    {
        template <typename Session, typename Socket>
        class Socks5TunnelManager : public ISocksStateManager<Session, Socket>
        {
            private:
                void forward(Socket* incoming, Socket* destination, bool& round_over)
                {
                    wait_for_read(incoming);
                    auto data = incoming->read();

                    wait_for_write(destination);
                    destination->write(data);

                    if (data.size() == 0)
                        // Other end has hung up
                        round_over = true;
                }

            public:
                virtual void handle_request(Session* session, Socket& connection)
                {   
                    auto outbound = session->get_outbound_connection().get();
                    auto inbound = &connection;
                    std::vector<const Socket*> sockets = {outbound, inbound};

                    

                    bool session_over = false;
                    do
                    {
                        auto ready = select_for_reading(sockets);
                        for (auto ready_socket : ready)
                        {
                            if (ready_socket == outbound)
                            {
                                forward(outbound, inbound, session_over);
                            }
                            else if (ready_socket == inbound)
                            {
                                forward(inbound, outbound, session_over);
                            }
                            else
                                std::cerr << "Got a socket we didn't expect returned as ready from a select." << std::endl;
                        }
                    } while (!session_over);

                    session->set_state_to_close();
                    std::cerr << "Session over." << std::endl;
                }

               
        };
    }
}


#endif
