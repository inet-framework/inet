//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/ReceiveFromPort.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/selectivity/SourcePortHeader_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

namespace inet {

Define_Module(ReceiveFromPort);

void ReceiveFromPort::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        registerService(AccessoryProtocol::sourcePort, nullptr, inputGate);
        registerProtocol(AccessoryProtocol::sourcePort, nullptr, outputGate);
    }
}

void ReceiveFromPort::processPacket(Packet *packet)
{
    auto header = packet->popAtFront<SourcePortHeader>();
    packet->addTagIfAbsent<L4PortInd>()->setSrcPort(header->getSourcePort());
}

} // namespace inet

