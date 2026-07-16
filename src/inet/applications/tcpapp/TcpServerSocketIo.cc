//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpServerSocketIo.h"

#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/SimulationContinuation.h"

namespace inet {

Define_Module(TcpServerSocketIo);

void TcpServerSocketIo::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        trafficSink.reference(gate("trafficOut"), true);
        WATCH(bytesRcvd);
        WATCH(bytesSent);
        WATCH_EXPR("socketState", socket ? TcpSocket::stateName(socket->getState()) : "none");
    }
}


void TcpServerSocketIo::acceptSocket(TcpAvailableInfo *availableInfo)
{
    Enter_Method("acceptSocket");
    socket = new TcpSocket(availableInfo);
    socket->setOutputGate(gate("socketOut"));
    socket->setCallback(this);
    socket->accept(availableInfo->getNewSocketId());
}

void TcpServerSocketIo::handleMessage(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        ASSERT(socket != nullptr && socket->belongsToSocket(message));
        socket->processMessage(message);
    }
    else if (message->arrivedOn("trafficIn")) {
        auto *pkt = check_and_cast<Packet *>(message);
        bytesSent += pkt->getByteLength();
        socket->send(pkt);
    }
    else if (message == readDelayTimer) {
        if (socket->isOpen())
            socket->read(par("readSize"));
    }
    else
        throw cRuntimeError("Unknown message");
}

void TcpServerSocketIo::socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent)
{
    Enter_Method("socketDataArrived");
    ASSERT(socket == this->socket);
    bytesRcvd += packet->getByteLength();
    packet->removeTag<SocketInd>();
    yieldBeforePush();
    trafficSink.pushPacket(packet);
    sendOrScheduleReadCommandIfNeeded();
}

void TcpServerSocketIo::socketEstablished(TcpSocket *socket, Indication *indication)
{
    Enter_Method("socketEstablished");
    ASSERT(socket == this->socket);
    sendOrScheduleReadCommandIfNeeded();
}

void TcpServerSocketIo::sendOrScheduleReadCommandIfNeeded()
{
    if (!socket->getAutoRead() && socket->isOpen()) {
        simtime_t delay = par("readDelay");
        if (delay >= SIMTIME_ZERO) {
            if (readDelayTimer == nullptr) {
                readDelayTimer = new cMessage("readDelayTimer");
                readDelayTimer->setContextPointer(this);
            }
            scheduleAfter(delay, readDelayTimer);
        }
        else {
            // send read message to TCP
            socket->read(par("readSize"));
        }
    }
}

void TcpServerSocketIo::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    if (gate->isName("trafficIn")) {
        bytesSent += packet->getByteLength();
        socket->send(packet);
    }
    else if (gate->isName("socketIn"))
        socket->processMessage(packet);
    else
        throw cRuntimeError("Unknown packet");
}

cGate *TcpServerSocketIo::lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction)
{
    Enter_Method("lookupModuleInterface");
    EV_TRACE << "Looking up module interface" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
    if (gate->isName("trafficIn")) {
        if (type == typeid(IPassivePacketSink))
            return gate;
    }
    else if (gate->isName("socketIn")) {
        if (type == typeid(IPassivePacketSink)) {
            auto socketInd = dynamic_cast<const SocketInd *>(arguments);
            if (socketInd != nullptr && socketInd->getSocketId() == socket->getSocketId())
                return gate;
            auto packetServiceTag = dynamic_cast<const PacketServiceTag *>(arguments);
            if (packetServiceTag != nullptr && packetServiceTag->getProtocol() == &Protocol::tcp)
                return gate;
        }
    }
    return nullptr;
}

} // namespace inet

