//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021r/Ieee8021rTagEpdHeaderInserter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/SequenceNumberTag_m.h"
#include "inet/linklayer/ieee8021r/Ieee8021rTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021rTagEpdHeaderInserter);

void Ieee8021rTagEpdHeaderInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *nextProtocolAsString = par("nextProtocol");
        if (*nextProtocolAsString != '\0')
            nextProtocol = Protocol::getProtocol(nextProtocolAsString);
    }
    else if (stage == INITSTAGE_LINK_LAYER)
        registerService(Protocol::ieee8021rTag, inputGate, nullptr);
}

void Ieee8021rTagEpdHeaderInserter::processPacket(Packet *packet)
{
    auto header = makeShared<Ieee8021rTagEpdHeader>();
    auto sequenceNumberReq = packet->removeTagIfPresent<SequenceNumberReq>();
    if (sequenceNumberReq != nullptr) {
        auto sequenceNumber = sequenceNumberReq->getSequenceNumber();
        EV_INFO << "Setting sequence number" << EV_FIELD(sequenceNumber) << EV_ENDL;
        header->setSequenceNumber(sequenceNumber);
        packet->addTagIfAbsent<SequenceNumberInd>()->setSequenceNumber(sequenceNumber);
    }
    auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    auto protocol = packetProtocolTag->getProtocol();
    if (protocol == &Protocol::ieee8022llc)
        header->setTypeOrLength(packet->getByteLength());
    else
        header->setTypeOrLength(ProtocolGroup::ethertype.findProtocolNumber(protocol));
    packet->insertAtFront(header);
    packetProtocolTag->setProtocol(&Protocol::ieee8021rTag);
    packetProtocolTag->setFrontOffset(b(0));
    const Protocol *dispatchProtocol = nullptr;
    if (auto encapsulationProtocolReq = packet->findTagForUpdate<EncapsulationProtocolReq>()) {
        dispatchProtocol = encapsulationProtocolReq->getProtocol(0);
        encapsulationProtocolReq->eraseProtocol(0);
    }
    else if (nextProtocol != nullptr)
        dispatchProtocol = nextProtocol;
    else
        dispatchProtocol = &Protocol::ethernetMac;
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(dispatchProtocol);
}

} // namespace inet

