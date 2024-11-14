//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpBasicClientApp.h"

#include "inet/applications/tcpapp/GenericAppMsg_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"

namespace inet {

#define MSGKIND_CONNECT    0
#define MSGKIND_SEND       1

Define_Module(TcpBasicClientApp);

TcpBasicClientApp::~TcpBasicClientApp()
{
    cancelAndDelete(timeoutMsg);
    cancelAndDelete(readDelayTimer);
}

void TcpBasicClientApp::initialize(int stage)
{
    TcpAppBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numRequestsToSend = 0;
        waitedReplyLength = 0;
        earlySend = false; // TODO make it parameter
        WATCH(numRequestsToSend);
        WATCH(earlySend);

        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        timeoutMsg = new cMessage("timer");
        readDelayTimer = new cMessage("readDelayTimer");
    }
}

void TcpBasicClientApp::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t now = simTime();
    simtime_t start = std::max(startTime, now);
    if (timeoutMsg && ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime))) {
        timeoutMsg->setKind(MSGKIND_CONNECT);
        scheduleAt(start, timeoutMsg);
    }
}

void TcpBasicClientApp::handleStopOperation(LifecycleOperation *operation)
{
    if (timeoutMsg != nullptr)
        cancelEvent(timeoutMsg);
    cancelEvent(readDelayTimer);
    if (socket.getState() == TcpSocket::CONNECTED || socket.getState() == TcpSocket::CONNECTING || socket.getState() == TcpSocket::PEER_CLOSED)
        close();
}

void TcpBasicClientApp::handleCrashOperation(LifecycleOperation *operation)
{
    if (timeoutMsg != nullptr)
        cancelEvent(timeoutMsg);
    cancelEvent(readDelayTimer);
    if (operation->getRootModule() != getContainingNode(this))
        socket.destroy();
}

void TcpBasicClientApp::sendRequest()
{
    ASSERT(waitedReplyLength == 0);
    long requestLength = par("requestLength");
    long replyLength = par("replyLength");
    if (requestLength < 1)
        requestLength = 1;
    if (replyLength < 1)
        replyLength = 1;
    waitedReplyLength = replyLength;

    const auto& payload = makeShared<GenericAppMsg>();
    Packet *packet = new Packet("data");
    payload->setChunkLength(B(requestLength));
    payload->setExpectedReplyLength(B(replyLength));
    payload->setServerClose(false);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);

    numRequestsToSend--;

    EV_INFO << "sending request with " << requestLength << " bytes, expected reply length " << replyLength << " bytes,"
            << "remaining " << numRequestsToSend << " request\n";

    sendPacket(packet);
}

void TcpBasicClientApp::handleTimer(cMessage *msg)
{
    switch (msg->getKind()) {
        case MSGKIND_CONNECT:
            // determine number of requests in this session
            numRequestsToSend = par("numRequestsPerSession");
            if (numRequestsToSend < 1)
                numRequestsToSend = 1;

            connect(); // active OPEN
            waitedReplyLength = 0;

            // significance of earlySend: if true, data will be sent already
            // in the ACK of SYN, otherwise only in a separate packet (but still
            // immediately)
            if (earlySend)
                sendRequest();
            break;

        case MSGKIND_SEND:
            sendRequest();
            // next request will be sent when reply to this one arrives (see socketDataArrived())
            sendOrScheduleReadCommandIfNeeded();
            break;

        default:
            throw cRuntimeError("Invalid timer msg: kind=%d", msg->getKind());
    }
}

void TcpBasicClientApp::socketEstablished(TcpSocket *socket)
{
    TcpAppBase::socketEstablished(socket);

    // perform first request if not already done (next one will be sent when reply arrives)
    if (!earlySend)
        sendRequest();

    sendOrScheduleReadCommandIfNeeded();
}

void TcpBasicClientApp::rescheduleAfterOrDeleteTimer(simtime_t d, short int msgKind)
{
    if (stopTime < SIMTIME_ZERO || simTime() + d < stopTime) {
        timeoutMsg->setKind(msgKind);
        rescheduleAfter(d, timeoutMsg);
    }
    else {
        cancelAndDelete(timeoutMsg);
        timeoutMsg = nullptr;
    }
}

void TcpBasicClientApp::socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent)
{
    auto dataLength = msg->getDataLength();
    TcpAppBase::socketDataArrived(socket, msg, urgent);

    EV_INFO << "Arrived data with " << dataLength << " length\n";

    waitedReplyLength -= dataLength.get<B>();
    if (waitedReplyLength > 0) {
        EV_INFO << "Waiting remained " << waitedReplyLength << " bytes of reply\n";
        sendOrScheduleReadCommandIfNeeded();
    }
    else {
        EV_INFO << "The complete reply arrived\n";
        if (waitedReplyLength < 0)
            EV_ERROR << "Arrived data was larger than required\n";
        waitedReplyLength = 0;

        if (numRequestsToSend > 0) {
            if (timeoutMsg) {
                simtime_t d = par("thinkTime");
                rescheduleAfterOrDeleteTimer(d, MSGKIND_SEND);
            }
        }
        else if (socket->getState() != TcpSocket::LOCALLY_CLOSED) {
            EV_INFO << "reply to last request arrived, closing session\n";
            close();
        }
    }
}

void TcpBasicClientApp::close()
{
    TcpAppBase::close();
    if (timeoutMsg != nullptr)
        cancelEvent(timeoutMsg);
    cancelEvent(readDelayTimer);
}

void TcpBasicClientApp::socketClosed(TcpSocket *socket)
{
    cancelEvent(readDelayTimer);
    TcpAppBase::socketClosed(socket);

    // start another session after a delay
    if (timeoutMsg) {
        simtime_t d = par("idleInterval");
        rescheduleAfterOrDeleteTimer(d, MSGKIND_CONNECT);
    }
}

void TcpBasicClientApp::socketFailure(TcpSocket *socket, int code)
{
    cancelEvent(readDelayTimer);
    TcpAppBase::socketFailure(socket, code);

    // reconnect after a delay
    if (timeoutMsg) {
        simtime_t d = par("reconnectInterval");
        rescheduleAfterOrDeleteTimer(d, MSGKIND_CONNECT);
    }
}

void TcpBasicClientApp::sendOrScheduleReadCommandIfNeeded()
{
    if (!socket.getAutoRead() && socket.isOpen()) {
        simtime_t delay = par("readDelay");
        if (delay >= SIMTIME_ZERO)
            scheduleAfter(delay, readDelayTimer);
        else
            // send read message to TCP
            socket.read(par("readSize"));
    }
}

} // namespace inet

