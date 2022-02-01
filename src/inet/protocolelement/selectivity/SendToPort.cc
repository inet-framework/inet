//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/SendToPort.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/selectivity/DestinationPortHeader_m.h"

namespace inet {

Define_Module(SendToPort);

void SendToPort::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        port = par("port");
        registerService(AccessoryProtocol::destinationPort, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::destinationPort, outputGate, nullptr);
    }
}

void SendToPort::processPacket(Packet *packet)
{
    auto header = makeShared<DestinationPortHeader>();
    header->setDestinationPort(port);
    packet->insertAtFront(header);
}

} // namespace inet

