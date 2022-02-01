//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/SendToMacAddress.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/protocolelement/common/AccessoryProtocol.h"
#include "inet/protocolelement/selectivity/DestinationMacAddressHeader_m.h"

namespace inet {

Define_Module(SendToMacAddress);

void SendToMacAddress::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *addressAsString = par("address");
        if (strlen(addressAsString) != 0)
            address = MacAddress(addressAsString);
        registerService(AccessoryProtocol::destinationMacAddress, inputGate, nullptr);
        registerProtocol(AccessoryProtocol::destinationMacAddress, outputGate, nullptr);
    }
}

void SendToMacAddress::pushPacket(Packet *packet, cGate *inGate)
{
    Enter_Method("pushPacket");
    take(packet);
    handleMessage(packet);
}

void SendToMacAddress::handleCanPushPacketChanged(cGate *outGate)
{
    producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
}

void SendToMacAddress::processPacket(Packet *packet)
{
    const auto& macAddressReq = packet->findTag<MacAddressReq>();
    auto destinationAddress = macAddressReq != nullptr ? macAddressReq->getDestAddress() : address;
    auto header = makeShared<DestinationMacAddressHeader>();
    header->setDestinationAddress(destinationAddress);
    packet->insertAtFront(header);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&AccessoryProtocol::destinationMacAddress);
}

void SendToMacAddress::handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    producer->handlePushPacketProcessed(packet, gate, successful);
}

} // namespace inet

