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
#include "inet/routing/pim/PimPacket_m.h"
#include "inet/routing/pim/PimProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::pim, PimProtocolDissector);

void PimProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<PimPacket>();
    callback.startProtocolDataUnit(&Protocol::pim);
    callback.visitChunk(header, &Protocol::pim);
    if (packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);        //TODO interpret payloads correctly
    ASSERT(packet->getDataLength() == b(0));
    callback.endProtocolDataUnit(&Protocol::pim);
}

} // namespace inet

