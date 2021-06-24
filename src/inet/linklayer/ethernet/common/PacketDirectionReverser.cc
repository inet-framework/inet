//
// Copyright (C) 2021 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/linklayer/ethernet/common/PacketDirectionReverser.h"

#include "inet/common/DirectionTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/protocolelement/redundancy/SequenceNumberTag_m.h"
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
    auto vlanInd = packet->findTag<VlanInd>();
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
    if (vlanInd != nullptr)
        packet->addTag<VlanReq>()->setVlanId(vlanInd->getVlanId());
    if (userPriorityInd != nullptr)
        packet->addTag<UserPriorityReq>()->setUserPriority(userPriorityInd->getUserPriority());
    if (streamInd != nullptr)
        packet->addTag<StreamReq>()->setStreamName(streamInd->getStreamName());
    if (sequenceNumberInd != nullptr)
        packet->addTag<SequenceNumberReq>()->setSequenceNumber(sequenceNumberInd->getSequenceNumber());
}

} // namespace inet

