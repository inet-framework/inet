//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_DHCPLEASE_H
#define __INET_DHCPLEASE_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/arp/ipv4/Arp.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

/**
 * Describes a DHCP lease.
 */
class INET_API DhcpLease
{
  public:
    long xid = -1;
    Ipv4Address ip;
    MacAddress mac;
    Ipv4Address gateway;
    Ipv4Address subnetMask;
    Ipv4Address dns;
    Ipv4Address ntp;
    Ipv4Address serverId;
    std::string hostName;
    simtime_t leaseTime;
    simtime_t renewalTime;
    simtime_t rebindTime;
    bool leased = false;
};

inline std::ostream& operator<<(std::ostream& os, DhcpLease obj)
{
    os << " IP: " << obj.ip << " with subnet mask: " << obj.subnetMask
       << " to " << obj.mac;
    return os;
}

} // namespace inet

#endif

