//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/tun/TunControlInfo_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo.h"
#include "inet/applications/tunapp/TunnelApp.h"

namespace inet {

Define_Module(TunnelApp);

TunnelApp::TunnelApp()
{
}

TunnelApp::~TunnelApp()
{
}

void TunnelApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interface = par("interface");
        const char *protocolName = par("protocol");
        protocol = Protocol::getProtocol(protocolName);
        destinationAddress = par("destinationAddress");
        if (protocol == &Protocol::udp) {
            destinationPort = par("destinationPort");
            localPort = par("localPort");
        }
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        if (protocol == &Protocol::ipv4) {
            l3Socket.setOutputGate(gate("socketOut"));
            l3Socket.setControlInfoProtocolId(Protocol::ipv4.getId());
            l3Socket.bind(IP_PROT_IP);
        }
        if (protocol == &Protocol::udp) {
            serverSocket.setOutputGate(gate("socketOut"));
            if (localPort != -1)
                serverSocket.bind(localPort);
            clientSocket.setOutputGate(gate("socketOut"));
            if (destinationPort != -1)
                clientSocket.connect(L3AddressResolver().resolve(destinationAddress), destinationPort);
        }
        IInterfaceTable *interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        InterfaceEntry *interfaceEntry = interfaceTable->getInterfaceByName(interface);
        if (interfaceEntry == nullptr)
            throw cRuntimeError("TUN interface not found: %s", interface);
        tunSocket.setOutputGate(gate("socketOut"));
        tunSocket.open(interfaceEntry->getInterfaceId());
    }
}

void TunnelApp::handleMessageWhenUp(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        cObject *controlInfo = message->getControlInfo();
        if (dynamic_cast<IPv4ControlInfo *>(controlInfo)) {
            delete message->removeControlInfo();
            tunSocket.send(PK(message));
        }
        else if (dynamic_cast<UDPControlInfo *>(controlInfo)) {
            delete message->removeControlInfo();
            tunSocket.send(PK(message));
        }
        else if (dynamic_cast<TunControlInfo *>(controlInfo)) {
            delete message->removeControlInfo();
            if (protocol == &Protocol::ipv4) {
                IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
                controlInfo->setTransportProtocol(IP_PROT_IP);
                message->setControlInfo(controlInfo);
                message->ensureTag<L3AddressReq>()->setDestination(L3AddressResolver().resolve(destinationAddress));
                l3Socket.send(PK(message));
            }
            else if (protocol == &Protocol::udp)
                clientSocket.send(PK(message));
            else
                throw cRuntimeError("Unknown protocol: %s", protocol->getName());;
        }
        else
            throw cRuntimeError("Unknown message: %s", message->getName());
    }
    else
        throw cRuntimeError("Unknown message: %s", message->getName());
}

} // namespace inet

