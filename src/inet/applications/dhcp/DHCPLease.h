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

#ifndef __INET_DHCPLEASE_H
#define __INET_DHCPLEASE_H

#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/arp/ipv4/ARP.h"

namespace inet {

/**
 * Describes a DHCP lease.
 */
class INET_API DHCPLease
{
  public:
    long xid = -1;
    IPv4Address ip;
    MACAddress mac;
    IPv4Address gateway;
    IPv4Address subnetMask;
    IPv4Address dns;
    IPv4Address ntp;
    IPv4Address serverId;
    std::string hostName;
    simtime_t leaseTime;
    simtime_t renewalTime;
    simtime_t rebindTime;
    bool leased = false;
};

inline std::ostream& operator<<(std::ostream& os, DHCPLease obj)
{
    os << " IP: " << obj.ip << " with subnet mask: " << obj.subnetMask
       << " to " << obj.mac;
    return os;
}

} // namespace inet

#endif // ifndef __INET_DHCPLEASE_H

