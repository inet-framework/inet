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

#ifndef __INET_IPV4ADDRESSPOLICY_H
#define __INET_IPV4ADDRESSPOLICY_H

#include "INETDefs.h"
#include "IAddressPolicy.h"
#include "IPv4Address.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"

class INET_API IPv4AddressPolicy : public IAddressPolicy
{
    public:
        static IPv4AddressPolicy INSTANCE;
        static const IPv4Address ALL_RIP_ROUTERS_MCAST;

    public:
        IPv4AddressPolicy() { }
        virtual ~IPv4AddressPolicy() { }

        virtual Address getLinkLocalManetRoutersMulticastAddress() const { return IPv4Address::LL_MANET_ROUTERS; }
        virtual Address getLinkLocalRIPRoutersMulticastAddress() const { return ALL_RIP_ROUTERS_MCAST; }
        virtual INetworkProtocolControlInfo * createNetworkProtocolControlInfo() const { return new IPv4ControlInfo(); }
        virtual void joinMulticastGroup(InterfaceEntry * interfaceEntry, const Address & address) const { interfaceEntry->ipv4Data()->joinMulticastGroup(address.toIPv4()); }
};

#endif
