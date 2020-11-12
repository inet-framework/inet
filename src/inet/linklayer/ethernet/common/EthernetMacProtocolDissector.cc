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

#include "inet/linklayer/ethernet/common/EthernetMacProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ethernetMac, EthernetMacProtocolDissector);

void EthernetMacProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& macHeader = packet->popAtFront<EthernetMacHeader>();
    callback.startProtocolDataUnit(&Protocol::ethernetMac);
    callback.visitChunk(macHeader, &Protocol::ethernetMac);
    const auto& fcs = packet->popAtBack<EthernetFcs>(ETHER_FCS_BYTES);
    int typeOrLength = macHeader->getTypeOrLength();
    if (isEth2Type(typeOrLength)) {
        auto payloadProtocol = ProtocolGroup::ethertype.findProtocol(typeOrLength);
        callback.dissectPacket(packet, payloadProtocol);
    }
    else {
        // LLC header
        auto ethEndOffset = packet->getFrontOffset() + B(typeOrLength);
        auto trailerOffset = packet->getBackOffset();
        packet->setBackOffset(ethEndOffset);
        callback.dissectPacket(packet, &Protocol::ieee8022llc);
        packet->setBackOffset(trailerOffset);
    }
    auto paddingLength = packet->getDataLength();
    if (paddingLength > b(0)) {
        const auto& padding = packet->popAtFront(paddingLength);
        callback.visitChunk(padding, &Protocol::ethernetMac);
    }
    callback.visitChunk(fcs, &Protocol::ethernetMac);
    callback.endProtocolDataUnit(&Protocol::ethernetMac);
}

} // namespace inet

