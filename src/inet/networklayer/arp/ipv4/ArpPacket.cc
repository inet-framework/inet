/*
 * Copyright (C) 2018 OpenSim Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * @author: Zoltan Bojthe
 */

#include "inet/common/INETDefs.h"
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

