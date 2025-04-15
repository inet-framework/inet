//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetFragmentFcsInserter.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/Checksum.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/protocolelement/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(EthernetFragmentFcsInserter);

uint64_t EthernetFragmentFcsInserter::computeComputedChecksum(const Packet *packet, ChecksumType checksumType) const
{
    auto data = packet->peekDataAsBytes();
    auto bytes = data->getBytes();
    auto fragmentTag = packet->getTag<FragmentTag>();
    currentFragmentCompleteFcs = ethernetCRC(bytes.data(), bytes.size(), fragmentTag->getFirstFragment() ? 0 : lastFragmentCompleteFcs);
    if (fragmentTag->getLastFragment())
        return currentFragmentCompleteFcs;
    else
        return ethernetCRC(bytes.data(), bytes.size()) ^ 0xFFFF0000;
}

void EthernetFragmentFcsInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<EthernetFragmentFcs>();
    auto fragmentTag = packet->getTag<FragmentTag>();
    ASSERT(checksumType == CHECKSUM_CRC32);
    auto fcs = computeChecksum(packet, checksumMode, checksumType);
    header->setFcs(fcs);
    FcsMode fcsMode = (FcsMode)checksumMode; //TODO KLUDGE: use ChecksumMode everywhere
    header->setFcsMode(fcsMode);
    header->setMFcs(!fragmentTag->getLastFragment());
    packet->trimBack();
    packet->insertAtBack(header);
    auto& packetProtocolTag = packet->findTagForUpdate<PacketProtocolTag>();
    if (packetProtocolTag != nullptr)
        packetProtocolTag->setBackOffset(packetProtocolTag->getBackOffset() - header->getChunkLength());
}

void EthernetFragmentFcsInserter::handlePacketProcessed(Packet *packet)
{
    ChecksumInserterBase::handlePacketProcessed(packet);
    lastFragmentCompleteFcs = currentFragmentCompleteFcs;
}

} // namespace inet

