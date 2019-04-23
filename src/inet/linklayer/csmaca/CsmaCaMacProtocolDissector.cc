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
#include "inet/linklayer/csmaca/CsmaCaMacHeader_m.h"
#include "inet/linklayer/csmaca/CsmaCaMacProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::csmaCaMac, CsmaCaMacProtocolDissector);

void CsmaCaMacProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<CsmaCaMacHeader>();
    auto trailer = packet->popAtBack<CsmaCaMacTrailer>(B(4));
    callback.startProtocolDataUnit(&Protocol::csmaCaMac);
    callback.visitChunk(header, &Protocol::csmaCaMac);
    if (auto dataHeader = dynamicPtrCast<const CsmaCaMacDataHeader>(header)) {
        auto payloadProtocol = ProtocolGroup::ethertype.findProtocol(dataHeader->getNetworkProtocol());
        callback.dissectPacket(packet, payloadProtocol);
    }
    ASSERT(packet->getDataLength() == B(0));
    callback.visitChunk(trailer, &Protocol::csmaCaMac);
    callback.endProtocolDataUnit(&Protocol::csmaCaMac);
}

} // namespace inet

