//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpSinkApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(TcpSinkAppThread);

TcpSinkAppThread::~TcpSinkAppThread()
{
    cancelAndDelete(readDelayTimer);
    delete socket;
}

void TcpSinkAppThread::acceptSocket(TcpAvailableInfo *availableInfo)
{
    Enter_Method("acceptSocket");
    bytesRcvd = 0;
    socket = new TcpSocket(availableInfo);
    socket->setOutputGate(gate("socketOut"));
    socket->setCallback(this);
    socket->accept(availableInfo->getNewSocketId());
}

void TcpSinkAppThread::handleMessage(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        ASSERT(socket != nullptr && socket->belongsToSocket(message));
        socket->processMessage(message);
    }
    else if (message == readDelayTimer) {
        if (socket->isOpen())
            socket->read(par("readSize"));
    }
    else
        throw cRuntimeError("Unknown message");
}

void TcpSinkAppThread::socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent)
{
    ASSERT(socket == this->socket);
    bytesRcvd += packet->getByteLength();
    emit(packetReceivedSignal, packet);
    delete packet;
    sendOrScheduleReadCommandIfNeeded();
}

void TcpSinkAppThread::socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo)
{
    throw cRuntimeError("Model error");
}

void TcpSinkAppThread::socketEstablished(TcpSocket *socket)
{
    ASSERT(socket == this->socket);
    sendOrScheduleReadCommandIfNeeded();
}

void TcpSinkAppThread::socketPeerClosed(TcpSocket *socket)
{
    if (readDelayTimer)
        cancelEvent(readDelayTimer);
    socket->close();
}

void TcpSinkAppThread::socketClosed(TcpSocket *socket)
{
    if (readDelayTimer)
        cancelEvent(readDelayTimer);
}

void TcpSinkAppThread::socketFailure(TcpSocket *socket, int code)
{
    if (readDelayTimer)
        cancelEvent(readDelayTimer);
}

void TcpSinkAppThread::socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status)
{
}

void TcpSinkAppThread::socketDeleted(TcpSocket *socket)
{
    ASSERT(socket == this->socket);
    if (readDelayTimer)
        cancelEvent(readDelayTimer);
    socket = nullptr;
}

void TcpSinkAppThread::sendOrScheduleReadCommandIfNeeded()
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

void TcpSinkAppThread::initialize()
{
    bytesRcvd = 0;
    WATCH(bytesRcvd);
}

void TcpSinkAppThread::refreshDisplay() const
{
    std::ostringstream os;
    os << (socket ? TcpSocket::stateName(socket->getState()) : "NULL_SOCKET") << "\nrcvd: " << bytesRcvd << " bytes";
    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

} // namespace inet

