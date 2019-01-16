//
// Copyright (C) 2018 OpenSim Ltd.
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
// @author: Zoltan Bojthe
//

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "inet/networklayer/ipv4/IcmpProtocolDissector.h"


namespace inet {

Register_Protocol_Dissector(&Protocol::icmpv4, IcmpProtocolDissector);

void IcmpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<IcmpHeader>();
    callback.startProtocolDataUnit(&Protocol::icmpv4);
    callback.visitChunk(header, &Protocol::icmpv4);
    switch (header->getType()) {
        case ICMP_DESTINATION_UNREACHABLE:
        case ICMP_TIME_EXCEEDED:
        case ICMP_PARAMETER_PROBLEM: {
            //TODO packet contains a complete Ipv4Header and the first 8 bytes of transport header (or icmp). (protocol specified in Ipv4Header.)
            callback.dissectPacket(packet, nullptr);
            break;
        }
        default:
            callback.dissectPacket(packet, nullptr);
            break;
    }
    callback.endProtocolDataUnit(&Protocol::icmpv4);
}

} // namespace inet

