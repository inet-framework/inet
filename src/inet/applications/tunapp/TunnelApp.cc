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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/tun/TunControlInfo_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo.h"

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
            ipv4Socket.setOutputGate(gate("socketOut"));
            ipv4Socket.bind(&Protocol::ipv4, Ipv4Address::UNSPECIFIED_ADDRESS);
            ipv4Socket.setCallback(this);
            socketMap.addSocket(&ipv4Socket);
        }
        if (protocol == &Protocol::udp) {
            serverSocket.setOutputGate(gate("socketOut"));
            if (localPort != -1)
                serverSocket.bind(localPort);
            clientSocket.setOutputGate(gate("socketOut"));
            if (destinationPort != -1)
                clientSocket.connect(L3AddressResolver().resolve(destinationAddress), destinationPort);
            clientSocket.setCallback(this);
            serverSocket.setCallback(this);
            socketMap.addSocket(&clientSocket);
            socketMap.addSocket(&serverSocket);
        }
        IInterfaceTable *interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        InterfaceEntry *interfaceEntry = interfaceTable->findInterfaceByName(interface);
        if (interfaceEntry == nullptr)
            throw cRuntimeError("TUN interface not found: %s", interface);
        tunSocket.setOutputGate(gate("socketOut"));
        tunSocket.open(interfaceEntry->getInterfaceId());
        tunSocket.setCallback(this);
        socketMap.addSocket(&tunSocket);
    }
}

void TunnelApp::handleMessageWhenUp(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        ASSERT(message->getControlInfo() == nullptr);

        if (auto socket = socketMap.findSocketFor(message)) {
            socket->processMessage(message);
        }
        else
            throw cRuntimeError("Unknown message: %s", message->getName());
    }
    else
        throw cRuntimeError("Message arrived on unknown gate %s", message->getArrivalGate()->getFullName());

    if (operationalState == State::STOPPING_OPERATION) {
        if (ipv4Socket.isOpen() || serverSocket.isOpen() || clientSocket.isOpen())
            return;
        for (auto s: socketMap.getMap())
            if (s.second->isOpen())
                return;
        socketMap.deleteSockets();
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
    }
}

void TunnelApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    auto packetProtocol = packet->getTag<TransportProtocolInd>()->getProtocol();
    if (protocol == packetProtocol) {
        packet->clearTags();
        tunSocket.send(packet);
    }
    else
        throw cRuntimeError("Unknown protocol: %s", packetProtocol->getName());;
}

void TunnelApp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    delete indication;
}

void TunnelApp::socketClosed(UdpSocket *socket)
{
    //TODO processing socket closed at stopOperation
}

// Ipv4Socket::ICallback
void TunnelApp::socketDataArrived(Ipv4Socket *socket, Packet *packet)
{
    auto packetProtocol = packet->getTag<NetworkProtocolInd>()->getProtocol();
    if (protocol == packetProtocol) {
        packet->clearTags();
        tunSocket.send(packet);
    }
    else
        throw cRuntimeError("Unknown protocol: %s", packetProtocol->getName());;
}

void TunnelApp::socketClosed(Ipv4Socket *socket)
{
    //TODO processing socket closed at stopOperation
}

// TunSocket::ICallback
void TunnelApp::socketDataArrived(TunSocket *socket, Packet *packet)
{
    // InterfaceInd says packet is from tunnel interface and socket id is present and equals to tunSocket
    if (protocol == &Protocol::ipv4) {
        packet->clearTags();
        packet->addTag<L3AddressReq>()->setDestAddress(L3AddressResolver().resolve(destinationAddress));
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
        ipv4Socket.send(packet);
    }
    else if (protocol == &Protocol::udp) {
        packet->clearTags();
        clientSocket.send(packet);
    }
    else
        throw cRuntimeError("Unknown protocol: %s", protocol->getName());;
}

void TunnelApp::handleStopOperation(LifecycleOperation *operation)
{
    ipv4Socket.close();
    serverSocket.close();
    clientSocket.close();
    for (auto s: socketMap.getMap())
        s.second->close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void TunnelApp::handleCrashOperation(LifecycleOperation *operation)
{
    ipv4Socket.destroy();
    serverSocket.destroy();
    clientSocket.destroy();
    for (auto s: socketMap.getMap())
        s.second->destroy();
    socketMap.deleteSockets();
}

} // namespace inet

