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

#include "inet/applications/tcpapp/TelnetApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "GenericAppMsg_m.h"

namespace inet {

#define MSGKIND_CONNECT    0
#define MSGKIND_SEND       1
#define MSGKIND_CLOSE      2

Define_Module(TelnetApp);


TelnetApp::~TelnetApp()
{
    cancelAndDelete(timeoutMsg);
}

void TelnetApp::checkedScheduleAt(simtime_t t, cMessage *msg)
{
    if (stopTime < SIMTIME_ZERO || t < stopTime)
        scheduleAt(t, msg);
}

void TelnetApp::initialize(int stage)
{
    TCPAppBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numCharsToType = numLinesToType = 0;
        WATCH(numCharsToType);
        WATCH(numLinesToType);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        simtime_t startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");

        timeoutMsg = new cMessage("timer");
        timeoutMsg->setKind(MSGKIND_CONNECT);
        scheduleAt(startTime, timeoutMsg);
    }
}

bool TelnetApp::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_APPLICATION_LAYER) {
            simtime_t now = simTime();
            simtime_t startTime = par("startTime");
            simtime_t start = std::max(startTime, now);
            if (timeoutMsg && ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime))) {
                timeoutMsg->setKind(MSGKIND_CONNECT);
                scheduleAt(start, timeoutMsg);
            }
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER) {
            cancelEvent(timeoutMsg);
            if (socket.getState() == TCPSocket::CONNECTED || socket.getState() == TCPSocket::CONNECTING || socket.getState() == TCPSocket::PEER_CLOSED)
                close();
            // TODO: wait until socket is closed
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH)
            cancelEvent(timeoutMsg);
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void TelnetApp::handleTimer(cMessage *msg)
{
    switch (msg->getKind()) {
        case MSGKIND_CONNECT:
            EV_INFO << "user fires up telnet program\n";
            connect();
            break;

        case MSGKIND_SEND:
            if (numCharsToType > 0) {
                // user types a character and expects it to be echoed
                EV_INFO << "user types one character, " << numCharsToType - 1 << " more to go\n";
                sendGenericAppMsg(1, 1);
                checkedScheduleAt(simTime() + (simtime_t)par("keyPressDelay"), timeoutMsg);
                numCharsToType--;
            }
            else {
                EV_INFO << "user hits Enter key\n";
                // Note: reply length must be at least 2, otherwise we'll think
                // it's an echo when it comes back!
                sendGenericAppMsg(1, 2 + (long)par("commandOutputLength"));
                numCharsToType = (long)par("commandLength");

                // Note: no checkedScheduleAt(), because user only starts typing next command
                // when output from previous one has arrived (see socketDataArrived())
            }
            break;

        case MSGKIND_CLOSE:
            EV_INFO << "user exits telnet program\n";
            close();
            break;
    }
}

void TelnetApp::sendGenericAppMsg(int numBytes, int expectedReplyBytes)
{
    EV_INFO << "sending " << numBytes << " bytes, expecting " << expectedReplyBytes << endl;

    GenericAppMsg *msg = new GenericAppMsg("data");
    msg->setByteLength(numBytes);
    msg->setExpectedReplyLength(expectedReplyBytes);
    msg->setServerClose(false);
    sendPacket(msg);
}

void TelnetApp::socketEstablished(int connId, void *ptr)
{
    TCPAppBase::socketEstablished(connId, ptr);

    // schedule first sending
    numLinesToType = (long)par("numCommands");
    numCharsToType = (long)par("commandLength");
    timeoutMsg->setKind(numLinesToType > 0 ? MSGKIND_SEND : MSGKIND_CLOSE);
    checkedScheduleAt(simTime() + (simtime_t)par("thinkTime"), timeoutMsg);
}

void TelnetApp::socketDataArrived(int connId, void *ptr, cPacket *msg, bool urgent)
{
    int len = msg->getByteLength();
    TCPAppBase::socketDataArrived(connId, ptr, msg, urgent);

    if (len == 1) {
        // this is an echo, ignore
        EV_INFO << "received echo\n";
    }
    else {
        // output from last typed command arrived.
        EV_INFO << "received output of command typed\n";

        // If user has finished working, she closes the connection, otherwise
        // starts typing again after a delay
        numLinesToType--;

        if (numLinesToType == 0) {
            EV_INFO << "user has no more commands to type\n";
            timeoutMsg->setKind(MSGKIND_CLOSE);
            checkedScheduleAt(simTime() + (simtime_t)par("thinkTime"), timeoutMsg);
        }
        else {
            EV_INFO << "user looks at output, then starts typing next command\n";
            timeoutMsg->setKind(MSGKIND_SEND);
            checkedScheduleAt(simTime() + (simtime_t)par("thinkTime"), timeoutMsg);
        }
    }
}

void TelnetApp::socketClosed(int connId, void *ptr)
{
    TCPAppBase::socketClosed(connId, ptr);

    // start another session after a delay
    timeoutMsg->setKind(MSGKIND_CONNECT);
    checkedScheduleAt(simTime() + (simtime_t)par("idleInterval"), timeoutMsg);
}

void TelnetApp::socketFailure(int connId, void *ptr, int code)
{
    TCPAppBase::socketFailure(connId, ptr, code);

    // reconnect after a delay
    timeoutMsg->setKind(MSGKIND_CONNECT);
    checkedScheduleAt(simTime() + (simtime_t)par("reconnectInterval"), timeoutMsg);
}

} // namespace inet

