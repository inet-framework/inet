/*
 * DHCPv6OdongLease.h
 *
 *  Created on: Jun 3, 2015
 *      Author: d8
 */

#ifndef INET_DHCPV6ODONGLEASE_H_
#define INET_DHCPV6ODONGLEASE_H_

#include "IPv6Address.h"
#include "MACAddress.h"
#include "ARP.h"

class DHCPv6OdongLease
{
public:
    IPv6Address prefix;
    int prefixLength;
    MACAddress mac;
    IPv6Address gateway;
    std::string hostName;
    simtime_t leaseTime;
    bool leased;
};

inline std::ostream& operator <<(std::ostream& os, DHCPv6OdongLease obj)
{
    os << "IP: " << obj.prefix << " with prefix length: " << obj.prefixLength << " to " << obj.mac;
    return os;
}

#endif /* DHCPV6ODONGLEASE_H_ */
