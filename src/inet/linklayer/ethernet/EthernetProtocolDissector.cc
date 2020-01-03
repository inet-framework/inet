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
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/linklayer/ethernet/EthernetProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ethernetMac, EthernetMacDissector);
Register_Protocol_Dissector(&Protocol::ethernetPhy, EthernetPhyDissector);

void EthernetPhyDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<EthernetPhyHeader>();
    callback.startProtocolDataUnit(&Protocol::ethernetPhy);
    callback.visitChunk(header, &Protocol::ethernetPhy);
    callback.dissectPacket(packet, &Protocol::ethernetMac);
    callback.endProtocolDataUnit(&Protocol::ethernetPhy);
}

void EthernetMacDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<EthernetMacHeader>();
    callback.startProtocolDataUnit(&Protocol::ethernetMac);
    callback.visitChunk(header, &Protocol::ethernetMac);
    const auto& fcs = packet->popAtBack<EthernetFcs>(ETHER_FCS_BYTES);
    if (isEth2Header(*header)) {
        auto payloadProtocol = ProtocolGroup::ethertype.findProtocol(header->getTypeOrLength());
        callback.dissectPacket(packet, payloadProtocol);
    }
    else {
        // LLC header
        auto ethEndOffset = packet->getFrontOffset() + B(header->getTypeOrLength());
        auto trailerOffset = packet->getBackOffset();
        packet->setBackOffset(ethEndOffset);
        callback.dissectPacket(packet, &Protocol::ieee8022);
        packet->setBackOffset(trailerOffset);
    }
    auto paddingLength = packet->getDataLength();
    if (paddingLength > b(0)) {
        const auto& padding = packet->popAtFront(paddingLength);        // remove padding (type is not EthernetPadding!)
        callback.visitChunk(padding, &Protocol::ethernetMac);
    }
    callback.visitChunk(fcs, &Protocol::ethernetMac);
    callback.endProtocolDataUnit(&Protocol::ethernetMac);
}

} // namespace inet

