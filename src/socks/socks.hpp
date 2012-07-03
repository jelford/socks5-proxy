#ifndef SOCKS_HPP
#define SOCKS_HPP

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
}

#endif
