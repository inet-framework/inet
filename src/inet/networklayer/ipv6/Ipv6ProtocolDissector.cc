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
#include "inet/networklayer/ipv6/Ipv6.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6ProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ipv6, Ipv6ProtocolDissector);

void Ipv6ProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto trailerPopOffset = packet->getBackOffset();
    const auto& header = packet->popAtFront<Ipv6Header>();
    auto ipv6EndOffset = packet->getFrontOffset() + B(header->getPayloadLength());
    callback.startProtocolDataUnit(&Protocol::ipv6);
    bool incorrect = (ipv6EndOffset > trailerPopOffset);
    if (incorrect) {
        callback.markIncorrect();
        ipv6EndOffset = trailerPopOffset;
    }
    callback.visitChunk(header, &Protocol::ipv6);
    packet->setBackOffset(ipv6EndOffset);
    const Ipv6FragmentHeader *fh = dynamic_cast<const Ipv6FragmentHeader *>(header->findExtensionHeaderByType(IP_PROT_IPv6EXT_FRAGMENT));
    if (fh)
        callback.dissectPacket(packet, nullptr);   //Fragment
    else
        callback.dissectPacket(packet, header->getProtocol());
    if (incorrect && packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);
    ASSERT(packet->getDataLength() == B(0));
    packet->setFrontOffset(ipv6EndOffset);
    packet->setBackOffset(trailerPopOffset);
    callback.endProtocolDataUnit(&Protocol::ipv6);
}

} // namespace inet

