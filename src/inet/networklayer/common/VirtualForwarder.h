//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Ported from the ANSAINET project.
// Original authors: Petr Vitek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_VIRTUALFORWARDER_H
#define __INET_VIRTUALFORWARDER_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

/**
 * Represents a virtual MAC address together with one or more virtual IPv4
 * addresses that a network interface may additionally own. Used by first-hop
 * redundancy protocols (e.g. VRRP, RFC 3768): while a node is the active
 * forwarder, it installs an *enabled* VirtualForwarder on the interface so that
 * the interface accepts link-layer frames destined to the virtual MAC and
 * answers ARP / accepts traffic for the virtual IP address(es).
 *
 * A VirtualForwarder is created and owned by the redundancy protocol module and
 * registered on a NetworkInterface via NetworkInterface::addVirtualForwarder().
 * NetworkInterface keeps only a (non-owning) pointer.
 */
class INET_API VirtualForwarder : public cObject
{
  protected:
    MacAddress macAddress;
    std::vector<Ipv4Address> ipAddresses;
    bool enabled = false;

  public:
    VirtualForwarder() {}

    const MacAddress& getMacAddress() const { return macAddress; }
    void setMacAddress(const MacAddress& address) { macAddress = address; }

    bool isEnabled() const { return enabled; }
    void setEnabled(bool enabled) { this->enabled = enabled; }

    const std::vector<Ipv4Address>& getIpAddresses() const { return ipAddresses; }
    void addIpAddress(const Ipv4Address& address);
    void removeIpAddress(const Ipv4Address& address);
    bool hasIpAddress(const Ipv4Address& address) const;

    virtual std::string str() const override;
};

} // namespace inet

#endif
