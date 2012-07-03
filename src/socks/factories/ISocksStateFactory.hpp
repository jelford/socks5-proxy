#ifndef FACTORIES_ISOCKSSTATEFACTORY_HPP
#define FACTORIES_ISOCKSSTATEFACTORY_HPP

#include "../state_managers/ISocksStateManager.hpp"

namespace socks
{
    namespace factories
    {
        template <typename SessionType, typename SocketType>
        class ISocksStateFactory
        {
            public:
                virtual managers::ISocksStateManager<SessionType, SocketType>* get() = 0;
        };
    }
}

#endif
