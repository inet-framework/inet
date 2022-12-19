//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8022/Ieee8022LlcInserter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"

namespace inet {

Define_Module(Ieee8022LlcInserter);

void Ieee8022LlcInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registerService(Protocol::ieee8022llc, inputGate, nullptr);
        registerProtocol(Protocol::ieee8022llc, outputGate, outputGate);
    }
}

void Ieee8022LlcInserter::processPacket(Packet *packet)
{
    const auto& protocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = protocolTag ? protocolTag->getProtocol() : nullptr;
    const auto& llcHeader = makeShared<Ieee8022LlcHeader>();
    int sapData = ProtocolGroup::getIeee8022ProtocolGroup()->findProtocolNumber(protocol);
    if (sapData != -1) {
        llcHeader->setSsap((sapData >> 8) & 0xFF);
        llcHeader->setDsap(sapData & 0xFF);
        llcHeader->setControl(3);
    }
    else {
        auto sapReq = packet->getTag<Ieee802SapReq>();
        llcHeader->setSsap(sapReq->getSsap());
        llcHeader->setDsap(sapReq->getDsap());
        llcHeader->setControl(3); // TODO get from sapTag
    }
    packet->insertAtFront(llcHeader);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee8022llc);
}

const Protocol *Ieee8022LlcInserter::getProtocol(const Ptr<const Ieee8022LlcHeader>& llcHeader)
{
    int32_t sapData = ((llcHeader->getSsap() & 0xFF) << 8) | (llcHeader->getDsap() & 0xFF);
    return ProtocolGroup::getIeee8022ProtocolGroup()->findProtocol(sapData); // do not use getProtocol
}

} // namespace inet

