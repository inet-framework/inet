//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8022/Ieee8022SnapChecker.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"

namespace inet {

Define_Module(Ieee8022SnapChecker);

void Ieee8022SnapChecker::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER) {
        registerService(Protocol::ieee8022snap, nullptr, outputGate);
        registerProtocol(Protocol::ieee8022snap, nullptr, inputGate);
    }
}

bool Ieee8022SnapChecker::matchesPacket(const Packet *packet) const
{
    const auto& snapHeader = packet->peekAtFront<Ieee8022SnapHeader>();
    auto protocol = getProtocol(snapHeader);
    return protocol != nullptr;
}

void Ieee8022SnapChecker::dropPacket(Packet *packet)
{
    EV_WARN << "Unknown protocol, dropping packet\n";
    PacketFilterBase::dropPacket(packet, NO_PROTOCOL_FOUND);
}

void Ieee8022SnapChecker::processPacket(Packet *packet)
{
    const auto& snapHeader = packet->popAtFront<Ieee8022SnapHeader>();
    auto protocol = getProtocol(snapHeader);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(protocol);
}

const Protocol *Ieee8022SnapChecker::getProtocol(const Ptr<const Ieee8022SnapHeader>& snapHeader)
{
    if (snapHeader->getOui() == 0)
        return ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(snapHeader->getProtocolId());
    else
        return ProtocolGroup::getSnapOuiProtocolGroup()->findProtocol(snapHeader->getOui());
}

} // namespace inet

