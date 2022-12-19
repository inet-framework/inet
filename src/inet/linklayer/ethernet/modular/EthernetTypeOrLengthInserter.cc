//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetTypeOrLengthInserter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"

namespace inet {

Define_Module(EthernetTypeOrLengthInserter);

void EthernetTypeOrLengthInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        registerService(Protocol::ethernetMac, inputGate, nullptr);
}

void EthernetTypeOrLengthInserter::processPacket(Packet *packet)
{
    const auto& header = makeShared<EthernetTypeOrLengthField>();
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol == &Protocol::ieee8022llc)
        header->setTypeOrLength(packet->getByteLength());
    else
        header->setTypeOrLength(ProtocolGroup::getEthertypeProtocolGroup()->findProtocolNumber(protocol));
    packet->insertAtFront(header);
    auto packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    packetProtocolTag->setFrontOffset(packetProtocolTag->getFrontOffset() + header->getChunkLength());
    packet->removeTagIfPresent<DispatchProtocolReq>();
}

} // namespace inet

