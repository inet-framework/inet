//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        auto payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(typeOrLength);
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

