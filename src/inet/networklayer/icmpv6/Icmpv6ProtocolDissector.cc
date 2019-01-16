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
#include "inet/networklayer/icmpv6/Icmpv6.h"
#include "inet/networklayer/icmpv6/Icmpv6ProtocolDissector.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"


namespace inet {

Register_Protocol_Dissector(&Protocol::icmpv6, Icmpv6ProtocolDissector);

void Icmpv6ProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    bool isBadPacket = !Icmpv6::verifyCrc(packet);
    const auto& header = packet->popAtFront<Icmpv6Header>();
    callback.startProtocolDataUnit(&Protocol::icmpv6);
    if (isBadPacket)
        callback.markIncorrect();
    callback.visitChunk(header, &Protocol::icmpv6);
    if (header->getType() < 128) {
        // ICMPv6 ERROR
        //TODO packet contains a complete Ipv6Header and the first 8 bytes of transport header (or ICMPv6). (protocol specified in Ipv6Header.)
        callback.dissectPacket(packet, nullptr);
    }
    else {
        // ICMPv6 INFO packets (e.g. ping), ICMPv6 Neighbour Discovery
        if (packet->getDataLength() > b(0))
            callback.dissectPacket(packet, nullptr);
    }
    callback.endProtocolDataUnit(&Protocol::icmpv6);
}

} // namespace inet

