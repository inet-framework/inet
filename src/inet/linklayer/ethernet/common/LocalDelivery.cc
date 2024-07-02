//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/LocalDelivery.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(LocalDelivery);

void LocalDelivery::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        deliveryOutConsumer.reference(gate("deliveryOut"), false);
        forwardingOutConsumer.reference(gate("forwardingOut"), false);
        interfaceTable.reference(this, "interfaceTableModule", true);
    }
}

void LocalDelivery::pushPacket(Packet *packet, const cGate *gate)
{
    auto interfaceInd = packet->getTag<InterfaceInd>();
    auto interface = interfaceTable->getInterfaceById(interfaceInd->getInterfaceId());
    auto macAddressInd = packet->getTag<MacAddressInd>();
    auto destinationAddress = macAddressInd->getDestAddress();
    if (destinationAddress.isBroadcast())
        // TODO also deliver locally if protocol is registered, otherwise it could result in an error (e.g ARP request)
        forwardingOutConsumer.pushPacket(packet);
    else if (destinationAddress.isMulticast()) {
        if (interface->matchesMulticastMacAddress(destinationAddress))
            // TODO should we still forward in this case?
            deliveryOutConsumer.pushPacket(packet);
        else
            forwardingOutConsumer.pushPacket(packet);
    }
    else {
        if (interface->getMacAddress() == destinationAddress)
            deliveryOutConsumer.pushPacket(packet);
        else
            forwardingOutConsumer.pushPacket(packet);
    }
}

} // namespace inet

