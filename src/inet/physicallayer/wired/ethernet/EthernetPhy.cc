//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

