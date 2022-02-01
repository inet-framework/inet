//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetCutthroughInterface.h"

#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

namespace inet {

Define_Module(EthernetCutthroughInterface);

void EthernetCutthroughInterface::initialize(int stage)
{
    NetworkInterface::initialize(stage);
    if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        cutthroughInputGate = gate("cutthroughIn");
        cutthroughOutputGate = gate("cutthroughOut");
        cutthroughInputConsumer = findConnectedModule<IPassivePacketSink>(cutthroughInputGate, 1);
        cutthroughOutputConsumer = findConnectedModule<IPassivePacketSink>(cutthroughOutputGate, 1);
        inet::registerInterface(*this, cutthroughInputGate, cutthroughOutputGate);
    }
}

void EthernetCutthroughInterface::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    take(packet);
    if (gate == cutthroughInputGate)
        cutthroughInputConsumer->pushPacketStart(packet, cutthroughInputGate->getPathEndGate(), datarate);
    else if (gate == cutthroughOutputGate) {
        packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceId);
        cutthroughOutputConsumer->pushPacketStart(packet, cutthroughOutputGate->getPathEndGate(), datarate);
    }
    else
        NetworkInterface::pushPacketStart(packet, gate, datarate);
}

void EthernetCutthroughInterface::pushPacketEnd(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketEnd");
    take(packet);
    if (gate == cutthroughInputGate)
        cutthroughInputConsumer->pushPacketEnd(packet, cutthroughInputGate->getPathEndGate());
    else if (gate == cutthroughOutputGate) {
        packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceId);
        cutthroughOutputConsumer->pushPacketEnd(packet, cutthroughOutputGate->getPathEndGate());
    }
    else
        NetworkInterface::pushPacketEnd(packet, gate);
}

} // namespace inet

