//
// Copyright (C) 2015 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/tun/Tun.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/stlutils.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/tun/TunControlInfo_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(Tun);

void Tun::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        lowerLayerInGateId = findGate("phys$i");
        lowerLayerOutGateId = findGate("phys$o");
    }
}

void Tun::configureNetworkInterface()
{
    networkInterface->setMtu(par("mtu"));
}

void Tun::handleUpperMessage(cMessage *message)
{
    if (!message->isPacket())
        handleUpperCommand(message);
    else {
        handleUpperPacket(check_and_cast<Packet *>(message));
    }
}

void Tun::handleUpperPacket(Packet *packet)
{
    EV_INFO << "Received packet from upper layer" << EV_FIELD(packet) << EV_ENDL;
    const auto& socketReq = packet->findTag<SocketReq>();
    // check if packet is from app by finding SocketReq with sockedId that is in socketIds
    auto sId = socketReq != nullptr ? socketReq->getSocketId() : -1;
    ASSERT(packet->getControlInfo() == nullptr);
    if (socketReq != nullptr && contains(socketIds, sId)) {
        // TODO should we determine the network protocol by looking at the packet?!
        packet->clearTags();
        packet->addTag<InterfaceInd>()->setInterfaceId(networkInterface->getInterfaceId());
        packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
        packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
        emit(packetSentToUpperSignal, packet);
        EV_INFO << "Sending packet to IPv4" << EV_FIELD(packet) << EV_ENDL;
        upperLayerSink.pushPacket(packet);
    }
    else {
        for (int socketId : socketIds) {
            Packet *copy = packet->dup();
            copy->setKind(TUN_I_DATA);
            copy->clearTags();
            copy->addTag<SocketInd>()->setSocketId(socketId);
            copy->addTag<InterfaceInd>()->setInterfaceId(networkInterface->getInterfaceId());
            copy->addTag<PacketProtocolTag>()->setProtocol(packet->getTag<PacketProtocolTag>()->getProtocol());
            auto npTag = packet->getTag<NetworkProtocolInd>();
            auto newnpTag = copy->addTag<NetworkProtocolInd>();
            *newnpTag = *npTag;
            EV_INFO << "Sending packet to socket" << EV_FIELD(socketId) << EV_FIELD(packet) << EV_ENDL;
            upperLayerSink.pushPacket(copy);
        }
        delete packet;
    }
}

void Tun::handleUpperCommand(cMessage *message)
{
    cObject *controlInfo = message->getControlInfo();
    int socketId = check_and_cast<Request *>(message)->getTag<SocketReq>()->getSocketId();
    if (dynamic_cast<TunOpenCommand *>(controlInfo) != nullptr) {
        if (contains(socketIds, socketId))
            throw cRuntimeError("Socket is already open: %d", socketId);
        socketIds.push_back(socketId);
        delete message;
    }
    else if (dynamic_cast<TunCloseCommand *>(controlInfo) != nullptr) {
        auto it = find(socketIds, socketId);
        if (it != socketIds.end())
            socketIds.erase(it);
        delete message;
        auto indication = new Indication("closed", TUN_I_CLOSED);
        auto ctrl = new TunSocketClosedIndication();
        indication->setControlInfo(ctrl);
        indication->addTagIfAbsent<SocketInd>()->setSocketId(socketId);
        send(indication, "upperLayerOut");
    }
    else if (dynamic_cast<TunDestroyCommand *>(controlInfo) != nullptr) {
        auto it = find(socketIds, socketId);
        if (it != socketIds.end())
            socketIds.erase(it);
        delete message;
    }
    else
        throw cRuntimeError("Unknown command: %s", message->getName());
}

void Tun::open(int socketId)
{
    if (contains(socketIds, socketId))
        throw cRuntimeError("Socket is already open: %d", socketId);
    socketIds.push_back(socketId);
}

cGate *Tun::lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction)
{
    Enter_Method("lookupModuleInterface");
    EV_TRACE << "Looking up module interface" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
    if (gate->isName("upperLayerIn")) {
        if (type == typeid(IPassivePacketSink)) {
            if (arguments == nullptr)
                return gate;
            else {
                auto socketInd = dynamic_cast<const SocketInd *>(arguments);
                if (socketInd != nullptr && contains(socketIds, socketInd->getSocketId()))
                    return gate;
            }
        }
        else if (type == typeid(ITun))
            return gate;
    }
    return nullptr;
}

} // namespace inet

