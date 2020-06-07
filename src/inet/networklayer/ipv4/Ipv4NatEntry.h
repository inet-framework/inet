// TODO copyright header

#ifndef __INET_IPV4NATENTRY_H
#define __INET_IPV4NATENTRY_H

#include "inet/networklayer/ipv4/Ipv4NatEntry_m.h"

namespace inet {

class Packet;

class Ipv4NatEntry : public Ipv4NatEntry_Base
{
public:
    using Ipv4NatEntry_Base::Ipv4NatEntry_Base;

    void applyToPacket(Packet *packet) const;
};


} // namespace inet

#endif // ifndef __INET_IPV4NATENTRY_H

