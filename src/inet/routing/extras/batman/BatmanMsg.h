#ifndef __INET_BATMANMSG_H
#define __INET_BATMANMSG_H

#include "inet/common/INETDefs.h"

#include "BatmanMsg_m.h"

namespace inet {

namespace inetmanet {

inline bool operator < (HnaElement const &a, HnaElement const &b)
{
    return (a.addr < b.addr) || (a.addr == b.addr && a.netmask < b.netmask);
}

inline bool operator > (HnaElement const &a, HnaElement const &b)
{
    return (a.addr > b.addr) || (a.addr == b.addr && a.netmask > b.netmask);
}

inline bool operator == (HnaElement const &a, HnaElement const &b)
{
    return (a.addr == b.addr && a.netmask == b.netmask);
}

inline bool operator != (HnaElement const &a, HnaElement const &b)
{
    return !(a.addr == b.addr && a.netmask == b.netmask);
}

} // namespace inetmanet

} // namespace inet

#endif
