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

void EthernetFcsInserter::processPacket(Packet *packet)
{
    if (auto cutthroughTag = packet->findTag<CutthroughTag>()) {
        auto header = dynamicPtrCast<const EthernetFcs>(cutthroughTag->getTrailerChunk());
        packet->insertAtBack(header);
    }
    else {
        const auto& header = makeShared<EthernetFcs>();
        ASSERT(checksumType == CHECKSUM_CRC32);
        auto fcs = computeChecksum(packet, checksumMode, checksumType);
        header->setFcs(fcs);
        FcsMode fcsMode = (FcsMode)checksumMode; //TODO KLUDGE: use ChecksumMode everywhere
        header->setFcsMode(fcsMode);
        packet->insertAtBack(header);
    }
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setProtocol(&Protocol::ethernetMac);
    packetProtocolTag->setFrontOffset(b(0));
    packetProtocolTag->setBackOffset(b(0));
}

} // namespace inet

