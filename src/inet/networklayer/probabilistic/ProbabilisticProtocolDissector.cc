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
#include "inet/networklayer/probabilistic/ProbabilisticBroadcastHeader_m.h"
#include "inet/networklayer/probabilistic/ProbabilisticProtocolDissector.h"


namespace inet {

Register_Protocol_Dissector(&Protocol::probabilistic, ProbabilisticProtocolDissector);

void ProbabilisticProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<ProbabilisticBroadcastHeader>();
    auto trailerPopOffset = packet->getBackOffset();
    auto payloadEndOffset = packet->getFrontOffset() + header->getPayloadLengthField();
    callback.startProtocolDataUnit(&Protocol::probabilistic);
    bool incorrect = (payloadEndOffset > trailerPopOffset);
    if (incorrect) {
        callback.markIncorrect();
        payloadEndOffset = trailerPopOffset;
    }
    callback.visitChunk(header, &Protocol::probabilistic);
    packet->setBackOffset(payloadEndOffset);
    callback.dissectPacket(packet, header->getProtocol());
    if (incorrect && packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);
    ASSERT(packet->getDataLength() == B(0));
    packet->setFrontOffset(payloadEndOffset);
    packet->setBackOffset(trailerPopOffset);
    callback.endProtocolDataUnit(&Protocol::probabilistic);
}

} // namespace inet

