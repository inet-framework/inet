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
#include "inet/networklayer/nexthop/NextHopForwardingHeader_m.h"
#include "inet/networklayer/nexthop/NextHopProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::nextHopForwarding, NextHopProtocolDissector);

void NextHopProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<NextHopForwardingHeader>();
    auto trailerPopOffset = packet->getBackOffset();
    auto endOffset = packet->getFrontOffset() + header->getPayloadLengthField();
    callback.startProtocolDataUnit(&Protocol::nextHopForwarding);
    callback.visitChunk(header, &Protocol::nextHopForwarding);
    packet->setBackOffset(endOffset);
    callback.dissectPacket(packet, header->getProtocol());
    ASSERT(packet->getDataLength() == B(0));
    packet->setFrontOffset(endOffset);
    packet->setBackOffset(trailerPopOffset);
    callback.endProtocolDataUnit(&Protocol::nextHopForwarding);
}

} // namespace inet

