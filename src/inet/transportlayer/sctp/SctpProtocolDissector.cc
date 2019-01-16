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
#include "inet/transportlayer/sctp/SctpHeader.h"
#include "inet/transportlayer/sctp/SctpProtocolDissector.h"

namespace inet {
namespace sctp {

Register_Protocol_Dissector(&Protocol::sctp, SctpProtocolDissector);

void SctpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    //TODO
    callback.startProtocolDataUnit(&Protocol::sctp);
    auto header = packet->popAtFront<SctpHeader>();
    callback.visitChunk(header, &Protocol::sctp);
    if (packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);
    ASSERT(packet->getDataLength() == b(0));
    callback.endProtocolDataUnit(&Protocol::udp);
}

} // namespace sctp
} // namespace inet

