//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Ported from the ANSAINET project.
// Original authors: Petr Vitek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/networklayer/common/VirtualForwarder.h"

#include "inet/common/stlutils.h"

namespace inet {

void VirtualForwarder::addIpAddress(const Ipv4Address& address)
{
    if (!contains(ipAddresses, address))
        ipAddresses.push_back(address);
}

void VirtualForwarder::removeIpAddress(const Ipv4Address& address)
{
    auto it = find(ipAddresses, address);
    if (it != ipAddresses.end())
        ipAddresses.erase(it);
}

bool VirtualForwarder::hasIpAddress(const Ipv4Address& address) const
{
    return contains(ipAddresses, address);
}

std::string VirtualForwarder::str() const
{
    std::stringstream out;
    out << "mac:" << macAddress << (enabled ? " enabled" : " disabled") << " ip:";
    for (auto& address : ipAddresses)
        out << address << " ";
    return out.str();
}

} // namespace inet
