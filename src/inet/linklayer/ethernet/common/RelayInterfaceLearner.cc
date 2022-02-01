//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/RelayInterfaceLearner.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(RelayInterfaceLearner);

void RelayInterfaceLearner::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        macForwardingTable.reference(this, "macTableModule", true);
        interfaceTable.reference(this, "interfaceTableModule", true);
    }
}

cGate *RelayInterfaceLearner::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void RelayInterfaceLearner::processPacket(Packet *packet)
{
    unsigned int vid = 0;
    if (auto vlanInd = packet->findTag<VlanInd>())
        vid = vlanInd->getVlanId();
    auto incomingInterface = interfaceTable->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    auto sourceAddress = packet->getTag<MacAddressInd>()->getSrcAddress();
    if (!sourceAddress.isMulticast()) {
        EV_INFO << "Learning peer address" << EV_FIELD(sourceAddress) << EV_FIELD(incomingInterface) << EV_ENDL;
        macForwardingTable->learnUnicastAddressForwardingInterface(incomingInterface->getInterfaceId(), sourceAddress, vid);
    }
}

} // namespace inet

