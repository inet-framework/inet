//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/PacketDirectionReverser.h"

#include "inet/common/DirectionTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/SequenceNumberTag_m.h"
#include "inet/linklayer/common/DropEligibleTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/PcpTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"
#include "inet/protocolelement/shaper/EligibilityTimeTag_m.h"

namespace inet {

Define_Module(PacketDirectionReverser);

void PacketDirectionReverser::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_QUEUEING)
        registerAnyProtocol(outputGate, inputGate);
}

void PacketDirectionReverser::processPacket(Packet *packet)
{
    auto packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto directionTag = packet->findTag<DirectionTag>();
    auto eligibilityTimeTag = packet->findTag<EligibilityTimeTag>();
    auto macAddressInd = packet->findTag<MacAddressInd>();
    auto dropEligibleInd = packet->findTag<DropEligibleInd>();
    auto vlanInd = packet->findTag<VlanInd>();
    auto pcpInd = packet->findTag<PcpInd>();
    auto userPriorityInd = packet->findTag<UserPriorityInd>();
    auto streamInd = packet->findTag<StreamInd>();
    auto sequenceNumberInd = packet->findTag<SequenceNumberInd>();
    auto interfaceInd = packet->findTag<InterfaceInd>();
    packet->trim();
    packet->clearTags();
    if (packetProtocolTag != nullptr)
        packet->addTag<PacketProtocolTag>()->setProtocol(packetProtocolTag->getProtocol());
    if (directionTag != nullptr) {
        if (directionTag->getDirection() != DIRECTION_INBOUND)
            throw cRuntimeError("Packet must be inbound");
        packet->addTagIfAbsent<DirectionTag>()->setDirection(DIRECTION_OUTBOUND);
    }
    if (eligibilityTimeTag != nullptr)
        packet->addTag<EligibilityTimeTag>()->setEligibilityTime(eligibilityTimeTag->getEligibilityTime());
    if (interfaceInd != nullptr)
        packet->addTag<InterfaceInd>()->setInterfaceId(interfaceInd->getInterfaceId());
    if (macAddressInd != nullptr) {
        const auto& macAddressReq = packet->addTag<MacAddressReq>();
        macAddressReq->setSrcAddress(macAddressInd->getSrcAddress());
        macAddressReq->setDestAddress(macAddressInd->getDestAddress());
    }
    if (dropEligibleInd != nullptr)
        packet->addTag<DropEligibleReq>()->setDropEligible(dropEligibleInd->getDropEligible());
    if (vlanInd != nullptr)
        packet->addTag<VlanReq>()->setVlanId(vlanInd->getVlanId());
    if (pcpInd != nullptr)
        packet->addTag<PcpReq>()->setPcp(pcpInd->getPcp());
    if (userPriorityInd != nullptr)
        packet->addTag<UserPriorityReq>()->setUserPriority(userPriorityInd->getUserPriority());
    if (streamInd != nullptr)
        packet->addTag<StreamReq>()->setStreamName(streamInd->getStreamName());
    if (sequenceNumberInd != nullptr)
        packet->addTag<SequenceNumberReq>()->setSequenceNumber(sequenceNumberInd->getSequenceNumber());
}

} // namespace inet

