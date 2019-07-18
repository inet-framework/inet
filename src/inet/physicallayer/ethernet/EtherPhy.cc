//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/physicallayer/ethernet/EtherPhy.h"

namespace inet {

namespace physicallayer {

Define_Module(EtherPhy);

void EtherPhy::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        physInGate = gate("phys$i");
        upperLayerInGate = gate("upperLayerIn");
    }
}

void EtherPhy::handleMessage(cMessage *message)
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
        auto signal = check_and_cast<EthernetSignal *>(message);
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

