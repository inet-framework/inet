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

#include "inet/networklayer/contract/ipv6/IPv6AddressType.h"

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"
#endif // ifdef WITH_IPv6

namespace inet {

IPv6AddressType IPv6AddressType::INSTANCE;

const IPv6Address IPv6AddressType::ALL_RIP_ROUTERS_MCAST("FF02::9");

L3Address IPv6AddressType::getLinkLocalAddress(const InterfaceEntry *ie) const
{
#ifdef WITH_IPv6
    if (ie->ipv6Data())
        return ie->ipv6Data()->getLinkLocalAddress();
#endif // ifdef WITH_IPv6
    return IPv6Address::UNSPECIFIED_ADDRESS;
}

} // namespace inet

