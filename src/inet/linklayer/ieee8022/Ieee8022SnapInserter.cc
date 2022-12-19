//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8022/Ieee8022SnapInserter.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"

namespace inet {

Define_Module(Ieee8022SnapInserter);

void Ieee8022SnapInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registerService(Protocol::ieee8022snap, inputGate, nullptr);
        registerProtocol(Protocol::ieee8022snap, outputGate, outputGate);
    }
}

void Ieee8022SnapInserter::processPacket(Packet *packet)
{
    const auto& protocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = protocolTag ? protocolTag->getProtocol() : nullptr;
    int ethType = -1;
    int snapOui = -1;
    if (protocol) {
        ethType = ProtocolGroup::getEthertypeProtocolGroup()->findProtocolNumber(protocol);
        if (ethType == -1)
            snapOui = ProtocolGroup::getSnapOuiProtocolGroup()->findProtocolNumber(protocol);
    }
    const auto& snapHeader = makeShared<Ieee8022SnapHeader>();
    if (ethType != -1) {
        snapHeader->setOui(0);
        snapHeader->setProtocolId(ethType);
    }
    else {
        snapHeader->setOui(snapOui);
        snapHeader->setProtocolId(-1); // FIXME get value from a tag (e.g. protocolTag->getSubId() ???)
    }
    packet->insertAtFront(snapHeader);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ieee8022snap);
}

const Protocol *Ieee8022SnapInserter::getProtocol(const Ptr<const Ieee8022SnapHeader>& snapHeader)
{
    if (snapHeader->getOui() == 0)
        return ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(snapHeader->getProtocolId());
    else
        return ProtocolGroup::getSnapOuiProtocolGroup()->findProtocol(snapHeader->getOui());
}

} // namespace inet

