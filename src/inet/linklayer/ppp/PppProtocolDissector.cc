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
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/ppp/PppFrame_m.h"
#include "inet/linklayer/ppp/PppProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ppp, PppProtocolDissector);

void PppProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    callback.startProtocolDataUnit(&Protocol::ppp);
    const auto& header = packet->popAtFront<PppHeader>();
    const auto& trailer = packet->popAtBack<PppTrailer>(PPP_TRAILER_LENGTH);
    callback.visitChunk(header, &Protocol::ppp);
    auto payloadProtocol = ProtocolGroup::pppprotocol.findProtocol(header->getProtocol());
    callback.dissectPacket(packet, payloadProtocol);
    callback.visitChunk(trailer, &Protocol::ppp);
    callback.endProtocolDataUnit(&Protocol::ppp);
}

} // namespace inet

