//
// (C) 2013 Opensim Ltd.
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Author: Andras Varga (andras@omnetpp.org)
//

#include "inet/applications/common/SocketTag_m.h"
#include "inet/networklayer/base/NetworkProtocolBase.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/contract/L3SocketCommand_m.h"

namespace inet {

NetworkProtocolBase::NetworkProtocolBase() :
    interfaceTable(nullptr)
{
}

void NetworkProtocolBase::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        registerService(getProtocol(), gate("transportIn"), gate("queueIn"));
        registerProtocol(getProtocol(), gate("queueOut"), gate("transportOut"));
    }
}

void NetworkProtocolBase::handleRegisterService(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
}

void NetworkProtocolBase::handleRegisterProtocol(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    protocolMapping.addProtocolMapping(protocol.getId(), in->getIndex());
}

void NetworkProtocolBase::sendUp(cMessage *message)
{
    if (Packet *packet = dynamic_cast<Packet *>(message)) {
        const Protocol *protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
        auto lowerBound = protocolIdToSocketDescriptors.lower_bound(protocol->getId());
        auto upperBound = protocolIdToSocketDescriptors.upper_bound(protocol->getId());
        bool hasSocket = lowerBound != upperBound;
        for (auto it = lowerBound; it != upperBound; it++) {
            Packet *packetCopy = packet->dup();
            packetCopy->addTagIfAbsent<SocketInd>()->setSocketId(it->second->socketId);
            emit(packetSentToUpperSignal, packetCopy);
            send(packetCopy, "transportOut");
        }
        if (protocolMapping.findOutputGateForProtocol(protocol->getId()) >= 0) {
            emit(packetSentToUpperSignal, packet);
            send(packet, "transportOut");
        }
        else {
            if (!hasSocket) {
                EV_ERROR << "Transport protocol '" << protocol->getName() << "' not connected, discarding packet\n";
                //TODO send an ICMP error: protocol unreachable
                // sendToIcmp(datagram, inputInterfaceId, ICMP_DESTINATION_UNREACHABLE, ICMP_DU_PROTOCOL_UNREACHABLE);
            }
            delete packet;
        }
    }
    else
        send(message, "transportOut");
}

void NetworkProtocolBase::sendDown(cMessage *message, int interfaceId)
{
    if (message->isPacket())
        emit(packetSentToLowerSignal, message);
    if (interfaceId != -1) {
        auto& tags = getTags(message);
        delete tags.removeTagIfPresent<DispatchProtocolReq>();
        tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
        send(message, "queueOut");
    }
    else {
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            InterfaceEntry *interfaceEntry = interfaceTable->getInterface(i);
            if (interfaceEntry && !interfaceEntry->isLoopback()) {
                cMessage* duplicate = utils::dupPacketAndControlInfo(message);
                auto& tags = getTags(duplicate);
                delete tags.removeTagIfPresent<DispatchProtocolReq>();
                tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceEntry->getInterfaceId());
                send(duplicate, "queueOut");
            }
        }
        delete message;
    }
}

bool NetworkProtocolBase::isUpperMessage(cMessage *message)
{
    return message->getArrivalGate()->isName("transportIn");
}

bool NetworkProtocolBase::isLowerMessage(cMessage *message)
{
    return message->getArrivalGate()->isName("queueIn");
}

void NetworkProtocolBase::handleUpperCommand(cMessage *msg)
{
    auto request = dynamic_cast<Request *>(msg);
    if (L3SocketBindCommand *command = dynamic_cast<L3SocketBindCommand *>(msg->getControlInfo())) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        SocketDescriptor *descriptor = new SocketDescriptor(socketId, command->getProtocol()->getId());
        socketIdToSocketDescriptor[socketId] = descriptor;
        protocolIdToSocketDescriptors.insert(std::pair<int, SocketDescriptor *>(command->getProtocol()->getId(), descriptor));
        delete msg;
    }
    else if (dynamic_cast<L3SocketCloseCommand *>(msg->getControlInfo()) != nullptr) {
        int socketId = request->getTag<SocketReq>()->getSocketId();
        auto it = socketIdToSocketDescriptor.find(socketId);
        if (it != socketIdToSocketDescriptor.end()) {
            int protocol = it->second->protocolId;
            auto lowerBound = protocolIdToSocketDescriptors.lower_bound(protocol);
            auto upperBound = protocolIdToSocketDescriptors.upper_bound(protocol);
            for (auto jt = lowerBound; jt != upperBound; jt++) {
                if (it->second == jt->second) {
                    protocolIdToSocketDescriptors.erase(jt);
                    break;
                }
            }
            delete it->second;
            socketIdToSocketDescriptor.erase(it);
        }
        delete msg;
    }
    else
        LayeredProtocolBase::handleUpperCommand(msg);
}



} // namespace inet

