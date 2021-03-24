//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/linklayer/ethernet/common/RelayInterfaceLearner.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(RelayInterfaceLearner);

void RelayInterfaceLearner::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        macAddressTable.reference(this, "macTableModule", true);
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
    auto macAddressInd = packet->getTag<MacAddressInd>();
    auto incomingInterface = interfaceTable->getInterfaceById(packet->getTag<InterfaceInd>()->getInterfaceId());
    auto sourceAddress = macAddressInd->getSrcAddress();
    EV_INFO << "Learning peer address" << EV_FIELD(sourceAddress) << EV_FIELD(incomingInterface) << EV_ENDL;
    macAddressTable->updateTableWithAddress(incomingInterface->getInterfaceId(), sourceAddress);
}

} // namespace inet

