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

#include "INETDefs.h"
#include "IAddressType.h"
#include "IPv6Address.h"
#include "IPv6ControlInfo.h"

class INET_API IPv6AddressType : public IAddressType
{
    public:
        static IPv6AddressType INSTANCE;
        static const IPv6Address ALL_RIP_ROUTERS_MCAST;

    public:
        IPv6AddressType() { }
        virtual ~IPv6AddressType() { }

        virtual int getMaxPrefixLength() const { return 128; }
        virtual Address getUnspecifiedAddress() const { return IPv6Address::UNSPECIFIED_ADDRESS; }
        virtual Address getLinkLocalManetRoutersMulticastAddress() const { return IPv6Address::LL_MANET_ROUTERS; }
        virtual Address getLinkLocalRIPRoutersMulticastAddress() const { return ALL_RIP_ROUTERS_MCAST; };
        virtual INetworkProtocolControlInfo * createNetworkProtocolControlInfo() const { return new IPv6ControlInfo(); }
        virtual Address getLinkLocalAddress(const InterfaceEntry *ie) const;
};

#endif
