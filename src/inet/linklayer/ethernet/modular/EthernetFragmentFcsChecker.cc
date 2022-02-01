//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetFragmentFcsChecker.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/checksum/EthernetCRC.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/protocolelement/fragmentation/tag/FragmentTag_m.h"

namespace inet {

Define_Module(EthernetFragmentFcsChecker);

bool EthernetFragmentFcsChecker::checkComputedFcs(const Packet *packet, uint32_t receivedFcs) const
{
    auto data = packet->peekDataAsBytes();
    auto bytes = data->getBytes();
    uint32_t fragmentFcs = ethernetCRC(bytes.data(), packet->getByteLength() - 4);
    auto& fragmentTag = packet->getTag<FragmentTag>();
    currentFragmentCompleteFcs = ethernetCRC(bytes.data(), packet->getByteLength() - 4, fragmentTag->getFirstFragment() ? 0 : lastFragmentCompleteFcs);
    bool lastFragment = receivedFcs != (fragmentFcs ^ 0xFFFF0000);
    return !lastFragment || receivedFcs == currentFragmentCompleteFcs;
}

bool EthernetFragmentFcsChecker::checkFcs(const Packet *packet, FcsMode fcsMode, uint32_t fcs) const
{
    switch (fcsMode) {
        case FCS_DECLARED_CORRECT:
            return checkDeclaredCorrectFcs(packet, fcs);
        case FCS_DECLARED_INCORRECT:
            return checkDeclaredIncorrectFcs(packet, fcs);
        case FCS_COMPUTED:
            return checkComputedFcs(packet, fcs);
        default:
            throw cRuntimeError("Unknown FCS mode");
    }
}

void EthernetFragmentFcsChecker::processPacket(Packet *packet)
{
    const auto& trailer = packet->popAtBack<EthernetFragmentFcs>(B(4));
    auto& fragmentTag = packet->getTagForUpdate<FragmentTag>();
    fragmentTag->setLastFragment(!trailer->getMCrc());
    auto packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    packetProtocolTag->setBackOffset(packetProtocolTag->getBackOffset() + trailer->getChunkLength());
    lastFragmentCompleteFcs = currentFragmentCompleteFcs;
}

bool EthernetFragmentFcsChecker::matchesPacket(const Packet *packet) const
{
    const auto& trailer = packet->peekAtBack<EthernetFragmentFcs>(B(4));
    auto fcsMode = trailer->getFcsMode();
    auto fcs = trailer->getFcs();
    return checkFcs(packet, fcsMode, fcs);
}

void EthernetFragmentFcsChecker::dropPacket(Packet *packet)
{
    PacketFilterBase::dropPacket(packet, INCORRECTLY_RECEIVED);
}

} // namespace inet

