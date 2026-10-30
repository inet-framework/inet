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
Register_Protocol_Dissector(&Protocol::ethernetMacNoFcs, EthernetMacProtocolDissector);

void EthernetMacProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    // the protocol tells whether the frame ends with an FCS: ethernetmac does, ethernetmacnofcs does not
    const auto& macHeader = packet->popAtFront<EthernetMacHeader>();
    callback.startProtocolDataUnit(protocol);
    callback.visitChunk(macHeader, protocol);
    Ptr<const EthernetFcs> fcs;
    if (protocol == &Protocol::ethernetMac)
        fcs = packet->popAtBack<EthernetFcs>(ETHER_FCS_BYTES);
    else if (protocol != &Protocol::ethernetMacNoFcs)
        throw cRuntimeError("Unknown protocol: %s", protocol->getName());
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
        callback.visitChunk(padding, protocol);
    }
    if (fcs != nullptr)
        callback.visitChunk(fcs, protocol);
    callback.endProtocolDataUnit(protocol);
}

} // namespace inet

