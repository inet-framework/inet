//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wired/ethernet/EthernetPhy.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"
#include "inet/physicallayer/wired/ethernet/EthernetSignal_m.h"

namespace inet {

namespace physicallayer {

Define_Module(EthernetPhy);

void EthernetPhy::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        physInGate = gate("phys$i");
        upperLayerInGate = gate("upperLayerIn");
    }
}

void EthernetPhy::handleMessage(cMessage *message)
{
    if (message->getArrivalGate() == upperLayerInGate) {
        Packet *packet = check_and_cast<Packet *>(message);
        auto phyHeader = makeShared<EthernetPhyHeader>();
        packet->insertAtFront(phyHeader);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetPhy);
        auto signal = new EthernetSignal(packet->getName());
        signal->setSrcMacFullDuplex(true);
        signal->encapsulate(packet);
        send(signal, "phys$o");
    }
    else if (message->getArrivalGate() == physInGate) {
        auto signal = check_and_cast<EthernetSignalBase *>(message);
        if (!signal->getSrcMacFullDuplex())
            throw cRuntimeError("Ethernet misconfiguration: MACs on the same link must be all in full duplex mode, or all in half-duplex mode");
        auto packet = check_and_cast<Packet *>(signal->decapsulate());
        delete signal;
        auto phyHeader = packet->popAtFront<EthernetPhyHeader>();
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
        send(packet, "upperLayerOut");
    }
    else
        throw cRuntimeError("Received unknown message");
}

} // namespace physicallayer

} // namespace inet

