//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv4modular/Ipv4SocketPacketProcessor.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/stlutils.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/ipv4/Ipv4SocketCommand_m.h"

namespace inet {

Define_Module(Ipv4SocketPacketProcessor);

void Ipv4SocketPacketProcessor::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        socketTable.reference(this, "socketTableModule", true);
        icmp.reference(this, "icmpModule", true);
    }
}

cGate *Ipv4SocketPacketProcessor::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void Ipv4SocketPacketProcessor::handleMessage(cMessage *message)
{
    if (auto command = dynamic_cast<Indication *>(message))
        processPushCommand(command, command->getArrivalGate());
    else
        PacketPusherBase::handleMessage(message);
}

void Ipv4SocketPacketProcessor::pushPacket(Packet *packet, cGate *arrivalGate)
{
    Enter_Method("pushPacket");
    take(packet);
    handlePacketProcessed(packet);

    const auto& protocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = protocolTag ? protocolTag->getProtocol() : nullptr;
    auto protocolId = protocol ? protocol->getId() : -1;
    const auto& l3AddressInd = packet->findTag<L3AddressInd>();
    Ipv4Address localAddress = l3AddressInd->getDestAddress().toIpv4();
    Ipv4Address remoteAddress = l3AddressInd->getSrcAddress().toIpv4();

    auto sockets = socketTable->findSockets(localAddress, remoteAddress, protocolId);
    bool hasSocket = false;
    for (auto socket : sockets) {
        auto packetCopy = packet->dup();
        packetCopy->setKind(IPv4_I_DATA);
        packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(socket->socketId);
        EV_INFO << "Passing up packet to socket" << EV_FIELD(socket) << EV_FIELD(packet) << EV_ENDL;
        pushOrSendPacket(packetCopy, outputGate, consumer);
        hasSocket = true;
    }
    if (contains(upperProtocols, protocol)) {
        EV_INFO << "Passing up to protocol " << *protocol << "\n";
        emit(packetSentToUpperSignal, packet);
        pushOrSendPacket(packet, outputGate, consumer);
    }
    else if (hasSocket) {
        delete packet;
    }
    else {
        EV_ERROR << "Transport protocol '" << protocol->getName() << "' not connected, discarding packet\n";
        // push back network protocol header
        packet->trim();
        packet->insertAtFront(packet->getTag<NetworkProtocolInd>()->getNetworkProtocolHeader());
        // get source interface:
        const auto& tag = packet->findTag<InterfaceInd>();
        icmp->sendErrorMessage(packet, tag ? tag->getInterfaceId() : -1, ICMP_DESTINATION_UNREACHABLE, ICMP_DU_PROTOCOL_UNREACHABLE);
        delete packet;
    }

    updateDisplayString();
}

void Ipv4SocketPacketProcessor::processPushCommand(Message *message, cGate *arrivalGate)
{
    Enter_Method("processPushCommand");
    take(message);
    if (arrivalGate->isName("socketIn")) {
        EV_DEBUG << "xyz IPv4 send to HL " << EV_FIELD(message) << EV_ENDL;
        send(message, outputGate);
    }
    else
        throw cRuntimeError("Command arrived on wrong gate '%s'", arrivalGate->getFullName());
}

void Ipv4SocketPacketProcessor::handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (gate->isName("out"))
        upperProtocols.insert(&protocol);
    TransparentProtocolRegistrationListener::handleRegisterProtocol(protocol, gate, servicePrimitive);
}

} // namespace inet
