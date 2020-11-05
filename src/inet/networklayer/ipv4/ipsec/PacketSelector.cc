//
// Copyright (C) 2020 OpenSim Ltd and Marcel Marek
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "PacketSelector.h"

#include "inet/networklayer/common/IPProtocolId_m.h"

namespace inet {
namespace ipsec {

bool PacketSelector::matches(const PacketInfo *packet) const
{
    if (!localAddress.empty() && !localAddress.contains(packet->getLocalAddress()))
        return false;
    if (!remoteAddress.empty() && !remoteAddress.contains(packet->getRemoteAddress()))
        return false;
    if (!nextProtocol.empty() && !nextProtocol.contains(packet->getNextProtocol()))
        return false;

    auto protocol = packet->getNextProtocol();
    if (protocol == IP_PROT_UDP || protocol == IP_PROT_TCP) {
        if (!localPort.empty() && !localPort.contains(packet->getLocalPort()))
            return false;
        if (!remotePort.empty() && !remotePort.contains(packet->getRemotePort()))
            return false;
    }
    else if (protocol == IP_PROT_ICMP) {
        if (!icmpType.empty() && !icmpType.contains(packet->getIcmpType()))
            return false;
        if (!icmpCode.empty() && !icmpCode.contains(packet->getIcmpCode()))
            return false;
    }
    return true;
}

std::string PacketSelector::str() const
{
    std::stringstream out;
    out << "Protocol: " << nextProtocol.str();
    if (localPort.empty())
        out << " Local: " << localAddress.str() << ";";
    else
        out << " Local: " << localAddress.str() << ":" << localPort.str() << ";";
    if (remotePort.empty())
        out << " Remote: " << remoteAddress.str() << ";";
    else
        out << " Remote: " << remoteAddress.str() << ":" << remotePort.str() << ";";
    if (!icmpType.empty())
        out << " ICMP type: " << icmpType.str() << ";";
    if (!icmpCode.empty())
        out << " ICMP code: " << icmpCode.str() << ";";
    return out.str();
}

}  // namespace ipsec
}  // namespace inet

