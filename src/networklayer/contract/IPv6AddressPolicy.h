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

#ifndef __INET_IPV6ADDRESSPOLICY_H
#define __INET_IPV6ADDRESSPOLICY_H

#include "INETDefs.h"
#include "IAddressPolicy.h"
#include "IPv6Address.h"
#include "IPv6ControlInfo.h"
#include "IPv6InterfaceData.h"

class INET_API IPv6AddressPolicy : public IAddressPolicy
{
    public:
        static IPv6AddressPolicy INSTANCE;
        static const IPv6Address ALL_RIP_ROUTERS_MCAST;

    public:
        IPv6AddressPolicy() { }
        virtual ~IPv6AddressPolicy() { }

        virtual Address getLinkLocalManetRoutersMulticastAddress() const { return IPv6Address::LL_MANET_ROUTERS; }
        virtual Address getLinkLocalRIPRoutersMulticastAddress() const { return ALL_RIP_ROUTERS_MCAST; };
        virtual INetworkProtocolControlInfo * createNetworkProtocolControlInfo() const { return new IPv6ControlInfo(); }
        virtual void joinMulticastGroup(InterfaceEntry * interfaceEntry, const Address & address) const { } // TODO:
};

#endif
