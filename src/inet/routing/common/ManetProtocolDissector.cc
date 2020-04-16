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
#include "inet/routing/common/ManetProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::manet, ManetProtocolDissector);

void ManetProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    callback.startProtocolDataUnit(&Protocol::manet);
//    callback.visitChunk(header, &Protocol::manet);
//    auto payloadProtocol = ProtocolGroup::ethertype.findProtocol(header->getNetworkProtocol());
//    callback.dissectPacket(packet, payloadProtocol);
//    ASSERT(packet->getDataLength() == B(0));

    callback.dissectPacket(packet, nullptr);    //FIXME KLUDGE choose from aodv|dsdv|dymo|gpsr

    callback.endProtocolDataUnit(&Protocol::manet);
}

} // namespace inet

