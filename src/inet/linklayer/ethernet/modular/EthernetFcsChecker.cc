//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetFcsChecker.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/protocolelement/cutthrough/CutthroughTag_m.h"

namespace inet {

Define_Module(EthernetFcsChecker);

void EthernetFcsChecker::initialize(int stage)
{
    ChecksumCheckerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        popFcs = par("popFcs");
}

void EthernetFcsChecker::processPacket(Packet *packet)
{
    if (auto cutthroughTag = packet->findTagForUpdate<CutthroughTag>()) {
        const auto& trailer = packet->peekAtBack<EthernetFcs>(ETHER_FCS_BYTES);
        cutthroughTag->setTrailerChunk(trailer);
    }
    if (popFcs) {
        const auto& trailer = packet->popAtBack<EthernetFcs>(ETHER_FCS_BYTES);
        auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
        packetProtocolTag->setBackOffset(packetProtocolTag->getBackOffset() + trailer->getChunkLength());
    }
}

bool EthernetFcsChecker::matchesPacket(const Packet *packet) const
{
    if (packet->hasTag<CutthroughTag>())
        return true;
    const auto& trailer = packet->peekAtBack<EthernetFcs>(ETHER_FCS_BYTES);
    auto fcsMode = trailer->getFcsMode();
    auto fcs = trailer->getFcs();
    ASSERT(fcsMode != FCS_DISABLED); //TODO DISABLED is illegal
    return checkChecksum(packet, (ChecksumMode)fcsMode, checksumType, fcs); //TODO KLUDGE cast should not be necessary
}

void EthernetFcsChecker::dropPacket(Packet *packet)
{
    PacketFilterBase::dropPacket(packet, INCORRECTLY_RECEIVED);
}

cGate *EthernetFcsChecker::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

} // namespace inet

