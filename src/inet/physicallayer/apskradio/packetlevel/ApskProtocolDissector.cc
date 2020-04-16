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

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/physicallayer/apskradio/packetlevel/ApskPhyHeader_m.h"
#include "inet/physicallayer/apskradio/packetlevel/ApskProtocolDissector.h"


namespace inet {

Register_Protocol_Dissector(&Protocol::apskPhy, ApskProtocolDissector);

void ApskProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<ApskPhyHeader>();
    callback.startProtocolDataUnit(&Protocol::apskPhy);
    callback.visitChunk(header, &Protocol::apskPhy);

    //FIXME KLUDGE: remove PhyPadding if exists
    auto padding = dynamicPtrCast<const BitCountChunk>(packet->peekAtFront());
    if (padding != nullptr) {
        packet->popAtFront<BitCountChunk>(padding->getChunkLength());
        callback.visitChunk(padding, &Protocol::apskPhy);
    }
    // end of KLUDGE

    auto payloadProtocol = header->getPayloadProtocol();
    callback.dissectPacket(packet, payloadProtocol);

    auto paddingLength = packet->getDataLength();
    if (paddingLength > b(0)) {
        const auto& padding = packet->popAtFront(paddingLength);        // remove padding
        callback.visitChunk(padding, &Protocol::apskPhy);
    }
    callback.endProtocolDataUnit(&Protocol::apskPhy);
}

} // namespace inet

