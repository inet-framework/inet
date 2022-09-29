//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/contract/ipv6/Ipv6AddressType.h"

#ifdef INET_WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif // ifdef INET_WITH_IPv6

namespace inet {

const Ipv6AddressType Ipv6AddressType::INSTANCE;

const Ipv6Address Ipv6AddressType::ALL_RIP_ROUTERS_MCAST("FF02::9");

L3Address Ipv6AddressType::getLinkLocalAddress(const NetworkInterface *ie) const
{
#ifdef INET_WITH_IPv6
    if (auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>())
        return ipv6Data->getLinkLocalAddress();
#endif // ifdef INET_WITH_IPv6
    return Ipv6Address::UNSPECIFIED_ADDRESS;
}

} // namespace inet

