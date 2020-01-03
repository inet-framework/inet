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
#include "inet/networklayer/mpls/MplsPacket_m.h"
#include "inet/networklayer/mpls/MplsProtocolDissector.h"


namespace inet {

Register_Protocol_Dissector(&Protocol::mpls, MplsProtocolDissector);

void MplsProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<MplsHeader>();
    callback.startProtocolDataUnit(&Protocol::mpls);
    callback.visitChunk(header, &Protocol::mpls);
    const Protocol *encapsulatedProtocol = header->getS() ? &Protocol::ipv4 : &Protocol::mpls;
    callback.dissectPacket(packet, encapsulatedProtocol);
    callback.endProtocolDataUnit(&Protocol::mpls);
}

} // namespace inet

