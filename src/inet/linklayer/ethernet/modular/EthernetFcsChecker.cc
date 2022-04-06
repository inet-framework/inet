//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetFcsChecker.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"

namespace inet {

Define_Module(EthernetFcsChecker);

void EthernetFcsChecker::initialize(int stage)
{
    FcsCheckerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        popFcs = par("popFcs");
}

bool EthernetFcsChecker::checkFcs(const Packet *packet, FcsMode fcsMode, uint32_t fcs) const
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

void EthernetFcsChecker::processPacket(Packet *packet)
{
    if (popFcs) {
        const auto& trailer = packet->popAtBack<EthernetFcs>(ETHER_FCS_BYTES);
        auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
        packetProtocolTag->setBackOffset(packetProtocolTag->getBackOffset() + trailer->getChunkLength());
    }
}

bool EthernetFcsChecker::matchesPacket(const Packet *packet) const
{
    const auto& trailer = packet->peekAtBack<EthernetFcs>(ETHER_FCS_BYTES);
    auto fcsMode = trailer->getFcsMode();
    auto fcs = trailer->getFcs();
    return checkFcs(packet, fcsMode, fcs);
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

