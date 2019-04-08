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
#include "inet/linklayer/ieee802154/Ieee802154MacHeader_m.h"
#include "inet/linklayer/ieee802154/Ieee802154ProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ieee802154, Ieee802154ProtocolDissector);

void Ieee802154ProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<Ieee802154MacHeader>();
    callback.startProtocolDataUnit(&Protocol::ieee802154);
    callback.visitChunk(header, &Protocol::ieee802154);
    if (header->getNetworkProtocol() != -1) {
        auto payloadProtocol = ProtocolGroup::ethertype.findProtocol(header->getNetworkProtocol());
        callback.dissectPacket(packet, payloadProtocol);
    }
//    auto paddingLength = packet->getDataLength();
//    if (paddingLength > b(0)) {
//        const auto& padding = packet->popHeader(paddingLength);
//        callback.visitChunk(padding, &Protocol::ieee802154);
//    }
    callback.endProtocolDataUnit(&Protocol::ieee802154);
}

} // namespace inet

