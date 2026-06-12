//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_DHCPLEASE_H
#define __INET_DHCPLEASE_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/arp/ipv4/Arp.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

/**
 * Lifecycle state of a DHCP lease entry at the server.
 *
 * FREE     - slot is unused or has been released/expired and may be re-offered;
 * OFFERED  - server has sent a DHCPOFFER and is waiting for DHCPREQUEST;
 * LEASED   - server has sent DHCPACK and the binding is active until expiry;
 * DECLINED - client sent DHCPDECLINE; address is quarantined until expiry.
 */
enum DhcpLeaseState {
    DHCP_LEASE_FREE,
    DHCP_LEASE_OFFERED,
    DHCP_LEASE_LEASED,
    DHCP_LEASE_DECLINED
};

/**
 * Describes a DHCP lease.
 */
class INET_API DhcpLease
{
  public:
    long xid = -1;
    Ipv4Address ip;
    MacAddress mac;
    Ipv4Address gateway;
    Ipv4Address subnetMask;
    Ipv4Address dns;
    Ipv4Address ntp;
    Ipv4Address serverId;
    std::string hostName;
    simtime_t leaseTime;
    simtime_t renewalTime;
    simtime_t rebindTime;
    // Server-side state machine for the lease (used only by DhcpServer).
    DhcpLeaseState state = DHCP_LEASE_FREE;
    // Absolute simulation time at which OFFERED/LEASED/DECLINED expires.
    simtime_t serverExpiryTime;
};

inline const char *dhcpLeaseStateName(DhcpLeaseState state)
{
    switch (state) {
        case DHCP_LEASE_FREE: return "FREE";
        case DHCP_LEASE_OFFERED: return "OFFERED";
        case DHCP_LEASE_LEASED: return "LEASED";
        case DHCP_LEASE_DECLINED: return "DECLINED";
    }
    return "?";
}

inline std::ostream& operator<<(std::ostream& os, DhcpLease obj)
{
    os << obj.mac << "  [" << dhcpLeaseStateName(obj.state) << "]";
    if (obj.state != DHCP_LEASE_FREE)
        os << ", expires t=" << (long)obj.serverExpiryTime.dbl() << "s";
    return os;
}

} // namespace inet

#endif

