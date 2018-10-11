//
// Copyright (C) OpenSim Ltd.
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021QTag_m.h"
#include "inet/linklayer/vlan/Vlan.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(Vlan);

void Vlan::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        vlanId = par("vlanId");
        interfaceEntry = getContainingNicModule(this);
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        auto interfaceTable = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        realInterfaceEntry = interfaceTable->getInterfaceByName(par("realInterfaceName"));
        const char *addressString = par("address");
        MacAddress address;
        if (!strcmp(addressString, "auto"))
            address = MacAddress::generateAutoAddress();
        else if (!strcmp(addressString, "copy"))
            address = realInterfaceEntry->getMacAddress();
        else
            address = MacAddress(addressString);
        interfaceEntry->setMacAddress(address);
        interfaceEntry->setInterfaceToken(address.formInterfaceIdentifier());
        interfaceEntry->setBroadcast(realInterfaceEntry->isBroadcast());
        interfaceEntry->setMulticast(realInterfaceEntry->isMulticast());
        interfaceEntry->setPointToPoint(realInterfaceEntry->isPointToPoint());
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        // KLUDGE: depends on other interface
        if (!strcmp(par("address"), "copy"))
            interfaceEntry->setMacAddress(realInterfaceEntry->getMacAddress());
        socket.setCallback(this);
        socket.setOutputGate(gate("upperLayerOut"));
        socket.setInterfaceEntry(realInterfaceEntry);
        socket.bind(MacAddress(), interfaceEntry->getMacAddress(), nullptr, vlanId);
    }
}

void Vlan::handleMessage(cMessage *message)
{
    if (socket.belongsToSocket(message))
        socket.processMessage(message);
    else {
        auto packet = check_and_cast<Packet *>(message);
        packet->addTag<Ieee8021QReq>()->setVid(vlanId);
        socket.send(packet);
    }
}

void Vlan::socketDataArrived(EthernetSocket *socket, Packet *packet)
{
    packet->removeTag<SocketInd>();
    packet->getTag<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
    send(packet, "upperLayerOut");
}

void Vlan::socketErrorArrived(EthernetSocket *socket, Indication *indication)
{
    throw cRuntimeError("Invalid operation");
}

} // namespace inet

