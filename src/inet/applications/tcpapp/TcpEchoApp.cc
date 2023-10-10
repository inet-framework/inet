//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpEchoApp.h"

#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"

namespace inet {

Define_Module(TcpEchoAppThread);

TcpEchoAppThread::~TcpEchoAppThread()
{
    cancelAndDelete(readDelayTimer);
    cancelAndDelete(delayedPacket);
    delete socket;
}

void TcpEchoAppThread::acceptSocket(TcpAvailableInfo *availableInfo)
{
    Enter_Method("acceptSocket");
    bytesRcvd = 0;
    bytesSent = 0;
    socket = new TcpSocket(availableInfo);
    socket->setOutputGate(gate("socketOut"));
    socket->setCallback(this);
    socket->accept(availableInfo->getNewSocketId());
}

void TcpEchoAppThread::handleMessage(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        ASSERT(socket != nullptr && socket->belongsToSocket(message));
        socket->processMessage(message);
    }
    else if (message == readDelayTimer) {
        if (socket->isOpen())
            socket->read(par("readSize"));
    }
    else if (message == delayedPacket) {
        emit(packetSentSignal, delayedPacket);
        socket->send(delayedPacket);
        delayedPacket = nullptr;
        sendOrScheduleReadCommandIfNeeded();
    }
    else
        throw cRuntimeError("Unknown message");
}

void TcpEchoAppThread::socketDataArrived(TcpSocket *socket, Packet *rcvdPkt, bool urgent)
{
    ASSERT(socket == this->socket);
    ASSERT(rcvdPkt->getTag<SocketInd>()->getSocketId() == socket->getSocketId());

    int64_t rcvdBytes = rcvdPkt->getByteLength();
    bytesRcvd += rcvdBytes;
    emit(packetReceivedSignal, rcvdPkt);

    if (socket->getState() != TcpSocket::CONNECTED) {
    }
    else if (echoFactor > 0.0) {
        Packet *outPkt = new Packet(rcvdPkt->getName(), TCP_C_SEND);
        // reverse direction, modify length, and send it back
        outPkt->addTag<SocketReq>()->setSocketId(socket->getSocketId());

        if (echoFactor == 1.0) {
            auto content = rcvdPkt->peekDataAt(B(0), B(rcvdBytes))->dupShared();
            content->removeTagsWherePresent<CreationTimeTag>(b(0), content->getChunkLength());
            content->addTag<CreationTimeTag>()->setCreationTime(simTime());
            outPkt->insertAtBack(content);
        }
        else {
            int64_t outByteLen = rcvdBytes * echoFactor;
            if (outByteLen < 1)
                outByteLen = 1;
            auto content = makeShared<ByteCountChunk>(B(outByteLen));
            content->addTag<CreationTimeTag>()->setCreationTime(simTime());
            outPkt->insertAtBack(content);
        }
        simtime_t delay = par("echoDelay");
        if (delay == SIMTIME_ZERO) {
            sendDown(outPkt);
            sendOrScheduleReadCommandIfNeeded();
        }
        else {
            delayedPacket = outPkt;
            scheduleAfter(delay, outPkt); // send after a delay
        }
    }
    else {
        sendOrScheduleReadCommandIfNeeded();
    }
    delete rcvdPkt;
}

void TcpEchoAppThread::socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo)
{
    throw cRuntimeError("Model error");
}

void TcpEchoAppThread::socketEstablished(TcpSocket *socket)
{
    ASSERT(socket == this->socket);
    sendOrScheduleReadCommandIfNeeded();
}

void TcpEchoAppThread::socketPeerClosed(TcpSocket *socket)
{
    if (readDelayTimer)
        cancelEvent(readDelayTimer);
    cancelAndDelete(delayedPacket);
    delayedPacket = nullptr;
    socket->close();
}

void TcpEchoAppThread::socketClosed(TcpSocket *socket)
{
    if (readDelayTimer)
        cancelEvent(readDelayTimer);
    cancelAndDelete(delayedPacket);
    delayedPacket = nullptr;

}

void TcpEchoAppThread::socketFailure(TcpSocket *socket, int code)
{
    if (readDelayTimer)
        cancelEvent(readDelayTimer);
    cancelAndDelete(delayedPacket);
    delayedPacket = nullptr;
}

void TcpEchoAppThread::socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status)
{
}

void TcpEchoAppThread::socketDeleted(TcpSocket *socket)
{
    ASSERT(socket == this->socket);
    if (readDelayTimer)
        cancelEvent(readDelayTimer);
    cancelAndDelete(delayedPacket);
    delayedPacket = nullptr;
    socket = nullptr;
}

void TcpEchoAppThread::sendOrScheduleReadCommandIfNeeded()
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

void TcpEchoAppThread::initialize()
{
    bytesRcvd = 0;
    bytesSent = 0;
    echoFactor = par("echoFactor");
    WATCH(bytesRcvd);
    WATCH(bytesSent);
}

void TcpEchoAppThread::refreshDisplay() const
{
    std::ostringstream os;
    os << (socket ? TcpSocket::stateName(socket->getState()) : "NULL_SOCKET")
            << "\nrcvd: " << bytesRcvd << " bytes"
            << "\nsent: " << bytesSent << " bytes";
    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

void TcpEchoAppThread::sendDown(Packet *msg)
{
    bytesSent += msg->getByteLength();
    emit(packetSentSignal, msg);
    socket->send(msg);
}

} // namespace inet

