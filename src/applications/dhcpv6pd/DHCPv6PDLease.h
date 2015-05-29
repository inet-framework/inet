//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
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

#ifndef INET_DHCPv6PDLEASE_H__
#define INET_DHCPv6PDLEASE_H__

#include "IPv6Address.h"
#include "MACAddress.h"
#include "ARP.h"

/**
 * Describes a DHCPv6PD lease.
 */
class DHCPv6PDLease
{
    public:
        long xid;
        IPv6Address prefix;
        MACAddress mac;
        IPv6Address gateway;
        IPv6Address prefixLength;
        IPv6Address dns;
        IPv6Address ntp;
        IPv6Address serverId;
        std::string hostName;
        simtime_t leaseTime;
        simtime_t renewalTime;
        simtime_t rebindTime;
        bool leased;
};

inline std::ostream& operator <<(std::ostream& os, DHCPv6PDLease obj)
{
    os << " IP: " << obj.prefix << " with subnet mask: " << obj.prefixLength
            << " to " << obj.mac;
    return os;
}

#endif
