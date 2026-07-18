//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/SendFromPort.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/selectivity/SourcePortHeader_m.h"

namespace inet {

Define_Module(SendFromPort);

void SendFromPort::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        port = par("port");
        registerService(AccessoryProtocol::sourcePort, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::sourcePort, outputGate, nullptr);
    }
}

void SendFromPort::processPacket(Packet *packet)
{
    auto header = makeShared<SourcePortHeader>();
    header->setSourcePort(port);
    packet->insertAtFront(header);
}

} // namespace inet

