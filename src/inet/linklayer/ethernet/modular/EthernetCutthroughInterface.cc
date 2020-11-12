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
        cutthroughOutputGate = gate("cutthroughOut");
        cutthroughConsumer = findConnectedModule<IPassivePacketSink>(cutthroughOutputGate);
        inet::registerInterface(*this, gate("cutthroughIn"), cutthroughOutputGate);
    }
}

void EthernetCutthroughInterface::pushPacketStart(Packet *packet, cGate *gate, bps datarate)
{
    Enter_Method("pushPacketStart");
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceId);
    cutthroughConsumer->pushPacketStart(packet, cutthroughOutputGate->getPathEndGate(), datarate);
}

void EthernetCutthroughInterface::pushPacketEnd(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacketEnd");
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceId);
    cutthroughConsumer->pushPacketEnd(packet, cutthroughOutputGate->getPathEndGate());
}

} // namespace inet

