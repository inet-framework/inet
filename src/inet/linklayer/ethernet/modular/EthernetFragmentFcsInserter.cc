//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetFragmentFcsInserter.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/EthernetCRC.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/protocolelement/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(EthernetFragmentFcsInserter);

uint32_t EthernetFragmentFcsInserter::computeComputedFcs(const Packet *packet) const
{
    auto data = packet->peekDataAsBytes();
    auto bytes = data->getBytes();
    auto fragmentTag = packet->getTag<FragmentTag>();
    currentFragmentCompleteFcs = ethernetCRC(bytes.data(), packet->getByteLength(), fragmentTag->getFirstFragment() ? 0 : lastFragmentCompleteFcs);
    if (fragmentTag->getLastFragment())
        return currentFragmentCompleteFcs;
    else
        return ethernetCRC(bytes.data(), packet->getByteLength()) ^ 0xFFFF0000;
}

uint32_t EthernetFragmentFcsInserter::computeFcs(const Packet *packet, FcsMode fcsMode) const
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

void EthernetFragmentFcsInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<EthernetFragmentFcs>();
    auto fragmentTag = packet->getTag<FragmentTag>();
    auto fcs = computeFcs(packet, fcsMode);
    header->setFcs(fcs);
    header->setFcsMode(fcsMode);
    header->setMCrc(!fragmentTag->getLastFragment());
    packet->trimBack();
    packet->insertAtBack(header);
    auto& packetProtocolTag = packet->findTagForUpdate<PacketProtocolTag>();
    if (packetProtocolTag != nullptr)
        packetProtocolTag->setBackOffset(packetProtocolTag->getBackOffset() - header->getChunkLength());
}

void EthernetFragmentFcsInserter::handlePacketProcessed(Packet *packet)
{
    FcsInserterBase::handlePacketProcessed(packet);
    lastFragmentCompleteFcs = currentFragmentCompleteFcs;
}

} // namespace inet

