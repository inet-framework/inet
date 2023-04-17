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
#include "inet/protocolelement/cutthrough/CutthroughTag_m.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"
#include "inet/protocolelement/shaper/EligibilityTimeTag_m.h"

namespace inet {

Define_Module(PacketDirectionReverser);

void PacketDirectionReverser::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        forwardVlan = par("forwardVlan");
        forwardPcp = par("forwardPcp");
        auto excludeEncapsulationProtocolsAsArray = check_and_cast<cValueArray *>(par("excludeEncapsulationProtocols").objectValue());
        for (int i = 0; i < excludeEncapsulationProtocolsAsArray->size(); i++) {
            auto protocol = Protocol::getProtocol(excludeEncapsulationProtocolsAsArray->get(i).stringValue());
            excludeEncapsulationProtocols.push_back(protocol);
        }
    }
    else if (stage == INITSTAGE_QUEUEING)
        registerAnyProtocol(outputGate, inputGate);
}

void PacketDirectionReverser::processPacket(Packet *packet)
{
    auto packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto directionTag = packet->findTag<DirectionTag>();
    auto cutthroughTag = packet->findTag<CutthroughTag>();
    auto eligibilityTimeTag = packet->findTag<EligibilityTimeTag>();
    auto macAddressInd = packet->findTag<MacAddressInd>();
    auto dropEligibleInd = packet->findTag<DropEligibleInd>();
    auto vlanInd = packet->findTag<VlanInd>();
    auto pcpInd = packet->findTag<PcpInd>();
    auto userPriorityInd = packet->findTag<UserPriorityInd>();
    auto streamInd = packet->findTag<StreamInd>();
    auto sequenceNumberInd = packet->findTag<SequenceNumberInd>();
    auto interfaceInd = packet->findTag<InterfaceInd>();
    auto encapsulationProtocolInd = packet->findTag<EncapsulationProtocolInd>();
    packet->trim();
    packet->clearTags();
    if (packetProtocolTag != nullptr)
        *(packet->addTag<PacketProtocolTag>()) = *packetProtocolTag;
    if (directionTag != nullptr) {
        if (directionTag->getDirection() != DIRECTION_INBOUND)
            throw cRuntimeError("Packet must be inbound");
        packet->addTag<DirectionTag>()->setDirection(DIRECTION_OUTBOUND);
    }
    if (cutthroughTag != nullptr) {
        *(packet->addTag<CutthroughTag>()) = *cutthroughTag;
    }
    if (eligibilityTimeTag != nullptr)
        *(packet->addTag<EligibilityTimeTag>()) = *eligibilityTimeTag;
    if (interfaceInd != nullptr)
        packet->addTag<InterfaceInd>()->InterfaceTagBase::operator=(*interfaceInd);
    if (macAddressInd != nullptr) {
        packet->addTag<MacAddressReq>()->MacAddressTagBase::operator=(*macAddressInd);
    }
    if (dropEligibleInd != nullptr)
        packet->addTag<DropEligibleReq>()->DropEligibleTagBase::operator=(*dropEligibleInd);
    if (vlanInd != nullptr && forwardVlan)
        packet->addTag<VlanReq>()->VlanTagBase::operator=(*vlanInd);
    if (pcpInd != nullptr && forwardPcp)
        packet->addTag<PcpReq>()->PcpTagBase::operator=(*pcpInd);
    if (userPriorityInd != nullptr)
        packet->addTag<UserPriorityReq>()->UserPriorityTagBase::operator=(*userPriorityInd);
    if (streamInd != nullptr)
        packet->addTag<StreamReq>()->StreamTagBase::operator=(*streamInd);
    if (sequenceNumberInd != nullptr)
        packet->addTag<SequenceNumberReq>()->SequenceNumberTagBase::operator=(*sequenceNumberInd);
    if (encapsulationProtocolInd != nullptr) {
        int n = encapsulationProtocolInd->getProtocolArraySize();
        auto encapsulationProtocolReq = packet->addTag<EncapsulationProtocolReq>();
        std::vector<const Protocol *> protocols;
        for (int i = 0; i < n; i++) {
            auto protocol = encapsulationProtocolInd->getProtocol(i);
            if (std::find(excludeEncapsulationProtocols.begin(), excludeEncapsulationProtocols.end(), protocol) == excludeEncapsulationProtocols.end())
                protocols.push_back(protocol);
        }
        encapsulationProtocolReq->setProtocolArraySize(protocols.size());
        for (int i = 0; i < protocols.size(); i++)
            encapsulationProtocolReq->setProtocol(protocols.size() - i - 1, protocols[i]);
    }
}

} // namespace inet

