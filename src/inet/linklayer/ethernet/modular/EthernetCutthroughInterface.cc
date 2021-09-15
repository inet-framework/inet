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

