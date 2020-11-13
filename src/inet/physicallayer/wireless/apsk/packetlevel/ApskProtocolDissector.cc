//
// Copyright (C) 2018 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/wireless/apsk/packetlevel/ApskProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/physicallayer/wireless/apsk/packetlevel/ApskPhyHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::apskPhy, ApskProtocolDissector);

void ApskProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<ApskPhyHeader>();
    callback.startProtocolDataUnit(&Protocol::apskPhy);
    callback.visitChunk(header, &Protocol::apskPhy);
    auto headerPaddingLength = header->getHeaderLengthField() - header->getChunkLength();
    if (headerPaddingLength > b(0)) {
        const auto& headerPadding = packet->popAtFront(headerPaddingLength);
        callback.visitChunk(headerPadding, &Protocol::apskPhy);
    }
    auto trailerPaddingLength = packet->getDataLength() - header->getPayloadLengthField();
    auto trailerPadding = trailerPaddingLength > b(0) ? packet->popAtBack(trailerPaddingLength) : nullptr;
    auto payloadProtocol = header->getPayloadProtocol();
    callback.dissectPacket(packet, payloadProtocol);
    if (trailerPaddingLength > b(0))
        callback.visitChunk(trailerPadding, &Protocol::apskPhy);
    callback.endProtocolDataUnit(&Protocol::apskPhy);
}

} // namespace inet

