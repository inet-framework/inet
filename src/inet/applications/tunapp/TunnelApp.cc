//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tunapp/TunnelApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/tun/TunControlInfo_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo.h"

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
        NetworkInterface *networkInterface = interfaceTable->findInterfaceByName(interface);
        if (networkInterface == nullptr)
            throw cRuntimeError("TUN interface not found: %s", interface);
        tunSocket.setOutputGate(gate("socketOut"));
        tunSocket.open(networkInterface->getInterfaceId());
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
        for (auto s : socketMap.getMap())
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
        throw cRuntimeError("Unknown protocol: %s", packetProtocol->getName());
}

void TunnelApp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    delete indication;
}

void TunnelApp::socketClosed(UdpSocket *socket)
{
    // TODO processing socket closed at stopOperation
}

// Ipv4Socket::ICallback
void TunnelApp::socketDataArrived(Ipv4Socket *socket, Packet *packet)
{
    EV_INFO << "Received packet from IPv4 socket" << EV_FIELD(packet) << EV_ENDL;
    auto packetProtocol = packet->getTag<NetworkProtocolInd>()->getProtocol();
    if (protocol == packetProtocol) {
        packet->clearTags();
        EV_INFO << "Sending packet to TUN socket" << EV_FIELD(packet) << EV_ENDL;
        tunSocket.send(packet);
    }
    else
        throw cRuntimeError("Unknown protocol: %s", packetProtocol->getName());
}

void TunnelApp::socketClosed(Ipv4Socket *socket)
{
    // TODO processing socket closed at stopOperation
}

// TunSocket::ICallback
void TunnelApp::socketDataArrived(TunSocket *socket, Packet *packet)
{
    EV_INFO << "Received packet from TUN socket" << EV_FIELD(packet) << EV_ENDL;
    // InterfaceInd says packet is from tunnel interface and socket id is present and equals to tunSocket
    if (protocol == &Protocol::ipv4) {
        packet->clearTags();
        packet->addTag<L3AddressReq>()->setDestAddress(L3AddressResolver().resolve(destinationAddress));
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
        EV_INFO << "Sending packet to IPv4 socket" << EV_FIELD(packet) << EV_ENDL;
        ipv4Socket.send(packet);
    }
    else if (protocol == &Protocol::udp) {
        packet->clearTags();
        EV_INFO << "Sending packet to UDP socket" << EV_FIELD(packet) << EV_ENDL;
        clientSocket.send(packet);
    }
    else
        throw cRuntimeError("Unknown protocol: %s", protocol->getName());
}

void TunnelApp::handleStopOperation(LifecycleOperation *operation)
{
    ipv4Socket.close();
    serverSocket.close();
    clientSocket.close();
    for (auto s : socketMap.getMap())
        s.second->close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void TunnelApp::handleCrashOperation(LifecycleOperation *operation)
{
    ipv4Socket.destroy();
    serverSocket.destroy();
    clientSocket.destroy();
    for (auto s : socketMap.getMap())
        s.second->destroy();
    socketMap.deleteSockets();
}

void TunnelApp::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    packet->setArrival(getId(), gate->getId());
    handleMessage(packet);
}

cGate *TunnelApp::lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction)
{
    Enter_Method("lookupModuleInterface");
    EV_TRACE << "Looking up module interface" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
    if (gate->isName("socketIn")) {
        if (type == typeid(IPassivePacketSink)) {
            auto socketInd = dynamic_cast<const SocketInd *>(arguments);
            if (socketInd != nullptr && socketMap.findSocketById(socketInd->getSocketId()) != nullptr)
                return gate;
        }
    }
    return nullptr;
}

} // namespace inet

