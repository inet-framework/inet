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
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/lmac/LMacHeader_m.h"
#include "inet/linklayer/lmac/LMacProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::lmac, LMacProtocolDissector);

void LMacProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<LMacHeaderBase>();
    callback.startProtocolDataUnit(&Protocol::lmac);
    callback.visitChunk(header, &Protocol::lmac);
    if (header->getType() == LMAC_DATA) {
        const auto& dataHeader = dynamicPtrCast<const LMacDataFrameHeader>(header);
        auto payloadProtocol = ProtocolGroup::ethertype.findProtocol(dataHeader->getNetworkProtocol());
        callback.dissectPacket(packet, payloadProtocol);
    }
    ASSERT(packet->getDataLength() == B(0));
    callback.endProtocolDataUnit(&Protocol::lmac);
}

} // namespace inet

