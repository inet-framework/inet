//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/SendToMacAddress.h"

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
    }
}

void SendToMacAddress::pushPacket(Packet *packet, const cGate *inGate)
{
    Enter_Method("pushPacket");
    take(packet);
    handleMessage(packet);
}

void SendToMacAddress::handleCanPushPacketChanged(const cGate *outGate)
{
    producer.handleCanPushPacketChanged();
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

void SendToMacAddress::handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful)
{
    producer.handlePushPacketProcessed(packet, successful);
}

} // namespace inet

