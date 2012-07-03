#ifndef STATE_MANAGERS_ISOCKSSTATEMANAGER_HPP
#define STATE_MANAGERS_ISOCKSSTATEMANAGER_HPP
namespace socks
{
    namespace managers
    {
        template <typename Session, typename Socket>
        class ISocksStateManager
        {
            public:
                virtual void handle_request(Session*, Socket&)=0;
        };
    }
}
#endif
