//
// Copyright (C) 2004 Andras Varga
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

#include <sstream>

#include "common.h"

std::string intToString(int i)
{
  std::ostringstream stream;
  stream << i << std::flush;
  std::string str(stream.str());
  return str;
}

int getLevel(const IPvXAddress& addr)
{
    if (addr.isIPv6())
    {
        switch(addr.get6().getScope())
        {
            case IPv6Address::UNSPECIFIED:
            case IPv6Address::MULTICAST:
                return 0;

            case IPv6Address::LOOPBACK:
                return 1;

            case IPv6Address::LINK:
                return 2;

            case IPv6Address::SITE:
                return 3;

            case IPv6Address::GLOBAL:
                return 4;

            default:
                throw cRuntimeError("Unknown IPv6 scope: %d", (int)(addr.get6().getScope()));
        }
    }
    else
    {
        switch(addr.get4().getAddressCategory())
        {
            case IPv4Address::UNSPECIFIED:
            case IPv4Address::THIS_NETWORK:
            case IPv4Address::MULTICAST:
            case IPv4Address::BENCHMARK:
            case IPv4Address::IPv6_TO_IPv4_RELAY:
                return 0;

            case IPv4Address::LOOPBACK:
                return 1;

            case IPv4Address::LINKLOCAL:
                return 2;

            case IPv4Address::PRIVATE_NETWORK:
                return 3;

            case IPv4Address::GLOBAL:
            case IPv4Address::BROADCAST:
                return 4;

            case IPv4Address::IETF:
            case IPv4Address::TEST_NET:
            case IPv4Address::RESERVED:
                return 4;         //need revision: is this good return value for these?

            default:
                throw cRuntimeError("Unknown IPv4 address category: %d", (int)(addr.get4().getAddressCategory()));
        }
    }
}
