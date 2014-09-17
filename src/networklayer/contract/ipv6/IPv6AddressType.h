//
// Copyright (C) 2013 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IPV6ADDRESSTYPE_H
#define __INET_IPV6ADDRESSTYPE_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/IL3AddressType.h"
#include "inet/networklayer/contract/ipv6/IPv6Address.h"
#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"

namespace inet {

class INET_API IPv6AddressType : public IL3AddressType
{
  public:
    static IPv6AddressType INSTANCE;
    static const IPv6Address ALL_RIP_ROUTERS_MCAST;

  public:
    IPv6AddressType() {}
    virtual ~IPv6AddressType() {}

    virtual int getMaxPrefixLength() const { return 128; }
    virtual L3Address getUnspecifiedAddress() const { return IPv6Address::UNSPECIFIED_ADDRESS; }
    virtual L3Address getBroadcastAddress() const { return IPv6Address::ALL_NODES_1; }
    virtual L3Address getLinkLocalManetRoutersMulticastAddress() const { return IPv6Address::LL_MANET_ROUTERS; }
    virtual L3Address getLinkLocalRIPRoutersMulticastAddress() const { return ALL_RIP_ROUTERS_MCAST; };
    virtual INetworkProtocolControlInfo *createNetworkProtocolControlInfo() const { return new IPv6ControlInfo(); }
    virtual L3Address getLinkLocalAddress(const InterfaceEntry *ie) const;
};

} // namespace inet

#endif // ifndef __INET_IPV6ADDRESSTYPE_H

