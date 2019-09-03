//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/applications/tcpapp/TcpBasicClientApp.h"

#include "inet/applications/tcpapp/GenericAppMsg_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TimeTag_m.h"

namespace inet {

#define MSGKIND_CONNECT    0
#define MSGKIND_SEND       1

Define_Module(TcpBasicClientApp);

TcpBasicClientApp::~TcpBasicClientApp()
{
    cancelAndDelete(timeoutMsg);
}

void TcpBasicClientApp::initialize(int stage)
{
    TcpAppBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numRequestsToSend = 0;
        earlySend = false;    // TBD make it parameter
        WATCH(numRequestsToSend);
        WATCH(earlySend);

        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        timeoutMsg = new cMessage("timer");
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
    cancelEvent(timeoutMsg);
    if (socket.getState() == TcpSocket::CONNECTED || socket.getState() == TcpSocket::CONNECTING || socket.getState() == TcpSocket::PEER_CLOSED)
        close();
}

void TcpBasicClientApp::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(timeoutMsg);
    if (operation->getRootModule() != getContainingNode(this))
        socket.destroy();
}

void TcpBasicClientApp::sendRequest()
{
    long requestLength = par("requestLength");
    long replyLength = par("replyLength");
    if (requestLength < 17)
        requestLength = 17;
    if (replyLength < 1)
        replyLength = 1;

    const auto& payload = makeShared<GenericAppMsg>();
    Packet *packet = new Packet("data");
    payload->setChunkLength(B(requestLength));
    payload->setExpectedReplyLength(B(replyLength));
    payload->setServerClose(false);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);

    EV_INFO << "sending request with " << requestLength << " bytes, expected reply length " << replyLength << " bytes,"
            << "remaining " << numRequestsToSend - 1 << " request\n";

    sendPacket(packet);
}

void TcpBasicClientApp::handleTimer(cMessage *msg)
{
    switch (msg->getKind()) {
        case MSGKIND_CONNECT:
            connect();    // active OPEN

            // significance of earlySend: if true, data will be sent already
            // in the ACK of SYN, otherwise only in a separate packet (but still
            // immediately)
            if (earlySend)
                sendRequest();
            break;

        case MSGKIND_SEND:
            sendRequest();
            numRequestsToSend--;
            // no scheduleAt(): next request will be sent when reply to this one
            // arrives (see socketDataArrived())
            break;

        default:
            throw cRuntimeError("Invalid timer msg: kind=%d", msg->getKind());
    }
}

void TcpBasicClientApp::socketEstablished(TcpSocket *socket)
{
    TcpAppBase::socketEstablished(socket);

    // determine number of requests in this session
    numRequestsToSend = par("numRequestsPerSession");
    if (numRequestsToSend < 1)
        numRequestsToSend = 1;

    // perform first request if not already done (next one will be sent when reply arrives)
    if (!earlySend)
        sendRequest();

    numRequestsToSend--;
}

void TcpBasicClientApp::rescheduleOrDeleteTimer(simtime_t d, short int msgKind)
{
    cancelEvent(timeoutMsg);

    if (stopTime < SIMTIME_ZERO || d < stopTime) {
        timeoutMsg->setKind(msgKind);
        scheduleAt(d, timeoutMsg);
    }
    else {
        delete timeoutMsg;
        timeoutMsg = nullptr;
    }
}

void TcpBasicClientApp::socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent)
{
    TcpAppBase::socketDataArrived(socket, msg, urgent);

    if (numRequestsToSend > 0) {
        EV_INFO << "reply arrived\n";

        if (timeoutMsg) {
            simtime_t d = simTime() + par("thinkTime");
            rescheduleOrDeleteTimer(d, MSGKIND_SEND);
        }
    }
    else if (socket->getState() != TcpSocket::LOCALLY_CLOSED) {
        EV_INFO << "reply to last request arrived, closing session\n";
        close();
    }
}

void TcpBasicClientApp::close()
{
    TcpAppBase::close();
    cancelEvent(timeoutMsg);
}

void TcpBasicClientApp::socketClosed(TcpSocket *socket)
{
    TcpAppBase::socketClosed(socket);

    // start another session after a delay
    if (timeoutMsg) {
        simtime_t d = simTime() + par("idleInterval");
        rescheduleOrDeleteTimer(d, MSGKIND_CONNECT);
    }
}

void TcpBasicClientApp::socketFailure(TcpSocket *socket, int code)
{
    TcpAppBase::socketFailure(socket, code);

    // reconnect after a delay
    if (timeoutMsg) {
        simtime_t d = simTime() + par("reconnectInterval");
        rescheduleOrDeleteTimer(d, MSGKIND_CONNECT);
    }
}

} // namespace inet

