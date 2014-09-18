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

#ifndef __INET_IPV4ADDRESSTYPE_H
#define __INET_IPV4ADDRESSTYPE_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/IL3AddressType.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"

namespace inet {

class INET_API IPv4AddressType : public IL3AddressType
{
  public:
    static IPv4AddressType INSTANCE;
    static const IPv4Address ALL_RIP_ROUTERS_MCAST;

  public:
    IPv4AddressType() {}
    virtual ~IPv4AddressType() {}

    virtual int getMaxPrefixLength() const { return 32; }
    virtual L3Address getUnspecifiedAddress() const { return IPv4Address::UNSPECIFIED_ADDRESS; }
    virtual L3Address getBroadcastAddress() const { return IPv4Address::ALLONES_ADDRESS; }
    virtual L3Address getLinkLocalManetRoutersMulticastAddress() const { return IPv4Address::LL_MANET_ROUTERS; }
    virtual L3Address getLinkLocalRIPRoutersMulticastAddress() const { return ALL_RIP_ROUTERS_MCAST; }
    virtual INetworkProtocolControlInfo *createNetworkProtocolControlInfo() const { return new IPv4ControlInfo(); }
    virtual L3Address getLinkLocalAddress(const InterfaceEntry *ie) const { return IPv4Address::UNSPECIFIED_ADDRESS; }
};

} // namespace inet

#endif // ifndef __INET_IPV4ADDRESSTYPE_H

