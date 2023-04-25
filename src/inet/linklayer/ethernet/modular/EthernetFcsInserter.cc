//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetFcsInserter.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/protocolelement/cutthrough/CutthroughTag_m.h"

namespace inet {

Define_Module(EthernetFcsInserter);

uint32_t EthernetFcsInserter::computeFcs(const Packet *packet, FcsMode fcsMode) const
{
    switch (fcsMode) {
        case FCS_DECLARED_CORRECT:
            return computeDeclaredCorrectFcs(packet);
        case FCS_DECLARED_INCORRECT:
            return computeDeclaredIncorrectFcs(packet);
        case FCS_COMPUTED:
            return computeComputedFcs(packet);
        default:
            throw cRuntimeError("Unknown FCS mode: %d", (int)fcsMode);
    }
}

void EthernetFcsInserter::processPacket(Packet *packet)
{
    if (auto cutthroughTag = packet->findTag<CutthroughTag>()) {
        auto header = dynamicPtrCast<const EthernetFcs>(cutthroughTag->getTrailerChunk());
        packet->insertAtBack(header);
    }
    else {
        const auto& header = makeShared<EthernetFcs>();
        auto fcs = computeFcs(packet, fcsMode);
        header->setFcs(fcs);
        header->setFcsMode(fcsMode);
        packet->insertAtBack(header);
    }
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setProtocol(&Protocol::ethernetMac);
    packetProtocolTag->setFrontOffset(b(0));
    packetProtocolTag->setBackOffset(b(0));
}

} // namespace inet

