//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/ProtocolUtils.h"

#include "inet/common/ProtocolTag_m.h"

namespace inet {

void dispatchToNextEncapsulationProtocol(Packet *packet)
{
    ASSERT(!packet->findTag<DispatchProtocolReq>());
    if (auto encapsulationProtocolReq = packet->findTagForUpdate<EncapsulationProtocolReq>()) {
        auto dispatchProtocol = encapsulationProtocolReq->getProtocol(0);
        encapsulationProtocolReq->eraseProtocol(0);
        packet->addTag<DispatchProtocolReq>()->setProtocol(dispatchProtocol);
        if (encapsulationProtocolReq->getProtocolArraySize() == 0)
            packet->removeTag<EncapsulationProtocolReq>();
    }
}

void appendDispatchToNetworkInterface(Packet *packet, const NetworkInterface *networkInterface)
{
    if (auto networkInterfaceProtocol = networkInterface->getProtocol())
        appendDispatchToEncapsulationProtocol(packet, networkInterfaceProtocol);
}

void appendDispatchToEncapsulationProtocol(Packet *packet, const Protocol *protocol)
{
    if (auto dispatchProtocolReq = packet->findTag<DispatchProtocolReq>()) {
        auto encapsulationProtocolReq = packet->addTagIfAbsent<EncapsulationProtocolReq>();
        encapsulationProtocolReq->appendProtocol(protocol);
    }
    else
        packet->addTag<DispatchProtocolReq>()->setProtocol(protocol);
}

void prependDispatchToEncapsulationProtocol(Packet *packet, const Protocol *protocol)
{
    auto dispatchProtocolReq = packet->findTagForUpdate<DispatchProtocolReq>();
    if (dispatchProtocolReq == nullptr)
        packet->addTag<DispatchProtocolReq>()->setProtocol(protocol);
    else {
        auto encapsulationReq = packet->addTagIfAbsent<EncapsulationProtocolReq>();
        encapsulationReq->insertProtocol(0, dispatchProtocolReq->getProtocol());
        dispatchProtocolReq->setProtocol(protocol);
    }
}

} // namespace inet

