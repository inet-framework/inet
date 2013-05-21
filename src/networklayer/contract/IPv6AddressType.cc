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

#include "IPv6AddressType.h"
#include "IPv6InterfaceData.h"

IPv6AddressType IPv6AddressType::INSTANCE;

const IPv6Address IPv6AddressType::ALL_RIP_ROUTERS_MCAST("FF02::9");

Address IPv6AddressType::getLinkLocalAddress(const InterfaceEntry *ie) const
{
    return ie->ipv6Data() ? ie->ipv6Data()->getLinkLocalAddress() : IPv6Address::UNSPECIFIED_ADDRESS;
}
