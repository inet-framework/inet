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

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/ipv4/Ipv4.h"
#include "inet/networklayer/ipv4/Ipv4ProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ipv4, Ipv4ProtocolDissector);

void Ipv4ProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto trailerPopOffset = packet->getBackOffset();
    auto ipv4EndOffset = packet->getFrontOffset();
    const auto& header = packet->popAtFront<Ipv4Header>();
    ipv4EndOffset += B(header->getTotalLengthField());
    callback.startProtocolDataUnit(&Protocol::ipv4);
    bool incorrect = (ipv4EndOffset > trailerPopOffset || header->getTotalLengthField() <= header->getHeaderLength());
    if (incorrect) {
        callback.markIncorrect();
        ipv4EndOffset = trailerPopOffset;
    }
    callback.visitChunk(header, &Protocol::ipv4);
    packet->setBackOffset(ipv4EndOffset);
    if (header->getFragmentOffset() == 0 && !header->getMoreFragments()) {
        callback.dissectPacket(packet, header->getProtocol());
    }
    else {
        //TODO Fragmentation
        callback.dissectPacket(packet, nullptr);
    }
    if (incorrect && packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);
    ASSERT(incorrect || packet->getDataLength() == b(0));
    packet->setFrontOffset(ipv4EndOffset);
    packet->setBackOffset(trailerPopOffset);
    callback.endProtocolDataUnit(&Protocol::ipv4);
}

} // namespace inet

