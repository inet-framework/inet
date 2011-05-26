//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "IPvXAddress.h"

#include <iostream>


bool IPvXAddress::tryParse(const char *addr)
{
    // try as IPv4
    if (IPv4Address::isWellFormed(addr))
    {
        set(IPv4Address(addr));
        return true;
    }

    // try as IPv6
    IPv6Address ipv6;
    if (ipv6.tryParse(addr))
    {
        set(ipv6);
        return true;
    }

    // no luck
    return false;
}

void IPvXAddress::set(const char *addr)
{
    if (!tryParse(addr))
        throw cRuntimeError("IPvXAddress: cannot interpret address string `%s'", addr);
}

