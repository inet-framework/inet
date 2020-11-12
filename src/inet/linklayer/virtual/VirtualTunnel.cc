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

#include "inet/linklayer/virtual/VirtualTunnel.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(VirtualTunnel);

void VirtualTunnel::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        vlanId = par("vlanId");
        networkInterface = getContainingNicModule(this);
        const char *protocolAsString = par("protocol");
        if (*protocolAsString != '\0')
            protocol = Protocol::findProtocol(protocolAsString);
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        auto interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        realNetworkInterface = CHK(interfaceTable->findInterfaceByName(par("realInterface")));
        const char *addressString = par("address");
        MacAddress address;
        if (!strcmp(addressString, "auto"))
            address = MacAddress::generateAutoAddress();
        else if (!strcmp(addressString, "copy"))
            address = realNetworkInterface->getMacAddress();
        else
            address = MacAddress(addressString);
        networkInterface->setMacAddress(address);
        networkInterface->setInterfaceToken(address.formInterfaceIdentifier());
        networkInterface->setBroadcast(realNetworkInterface->isBroadcast());
        networkInterface->setMulticast(realNetworkInterface->isMulticast());
        networkInterface->setPointToPoint(realNetworkInterface->isPointToPoint());
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        // KLUDGE: depends on other interface
        if (!strcmp(par("address"), "copy"))
            networkInterface->setMacAddress(realNetworkInterface->getMacAddress());
        if (protocol == &Protocol::ethernetMac) {
            auto ethernetSocket = new EthernetSocket();
            ethernetSocket->setCallback(this);
            ethernetSocket->setOutputGate(gate("upperLayerOut"));
            ethernetSocket->setNetworkInterface(realNetworkInterface);
            ethernetSocket->bind(networkInterface->getMacAddress(), MacAddress(), nullptr, par("steal"));
            socket = ethernetSocket;
        }
        else if (protocol == &Protocol::ieee8021qCTag || protocol == &Protocol::ieee8021qSTag) {
            auto ieee8021qSocket = new Ieee8021qSocket();
            ieee8021qSocket->setCallback(this);
            ieee8021qSocket->setOutputGate(gate("upperLayerOut"));
            ieee8021qSocket->setNetworkInterface(realNetworkInterface);
            ieee8021qSocket->setProtocol(protocol);
            ieee8021qSocket->bind(nullptr, vlanId, par("steal"));
            socket = ieee8021qSocket;
        }
        else
            throw cRuntimeError("Unknown protocol");
    }
}

void VirtualTunnel::handleMessage(cMessage *message)
{
    if (socket->belongsToSocket(message))
        socket->processMessage(message);
    else {
        auto packet = check_and_cast<Packet *>(message);
        if (vlanId != -1)
            packet->addTagIfAbsent<VlanReq>()->setVlanId(vlanId);
        packet->addTagIfAbsent<MacAddressReq>()->setSrcAddress(networkInterface->getMacAddress());
        socket->send(packet);
    }
}

void VirtualTunnel::socketDataArrived(EthernetSocket *socket, Packet *packet)
{
    packet->removeTag<SocketInd>();
    packet->getTagForUpdate<InterfaceInd>()->setInterfaceId(networkInterface->getInterfaceId());
    send(packet, "upperLayerOut");
}

void VirtualTunnel::socketDataArrived(Ieee8021qSocket *socket, Packet *packet)
{
    packet->removeTag<SocketInd>();
    packet->getTagForUpdate<InterfaceInd>()->setInterfaceId(networkInterface->getInterfaceId());
    send(packet, "upperLayerOut");
}

} // namespace inet

