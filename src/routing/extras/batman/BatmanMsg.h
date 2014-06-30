#ifndef __INET_BATMANMSG_H_
#define __INET_BATMANMSG_H_

#include "INETDefs.h"

#include "BatmanMsg_m.h"

namespace inet {


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


} //namespace


#endif // __INET_BATMANMSG_H_
