//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"

namespace inet {

std::string ArpPacket::str() const
{
    std::ostringstream stream;
    switch (getOpcode()) {
        case ARP_REQUEST:
            stream << "ARP req: " << getDestIpAddress()
                   << "=? (s=" << getSrcIpAddress() << "(" << getSrcMacAddress() << "))";
            break;

        case ARP_REPLY:
            stream << "ARP reply: "
                   << getSrcIpAddress() << "=" << getSrcMacAddress()
                   << " (d=" << getDestIpAddress() << "(" << getDestMacAddress() << "))";
            break;

        case ARP_RARP_REQUEST:
            stream << "RARP req: " << getDestMacAddress()
                   << "=? (s=" << getSrcIpAddress() << "(" << getSrcMacAddress() << "))";
            break;

        case ARP_RARP_REPLY:
            stream << "RARP reply: "
                   << getSrcMacAddress() << "=" << getSrcIpAddress()
                   << " (d=" << getDestIpAddress() << "(" << getDestMacAddress() << "))";
            break;

        default:
            stream << "ARP op=" << getOpcode() << ": d=" << getDestIpAddress()
                   << "(" << getDestMacAddress()
                   << ") s=" << getSrcIpAddress()
                   << "(" << getSrcMacAddress() << ")";
            break;
    }
    return stream.str();
}

} // namespace inet

