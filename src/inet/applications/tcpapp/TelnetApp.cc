//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TelnetApp.h"

#include "inet/applications/tcpapp/GenericAppMsg_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Packet.h"

namespace inet {

#define MSGKIND_CONNECT    0
#define MSGKIND_SEND       1
#define MSGKIND_CLOSE      2

Define_Module(TelnetApp);

TelnetApp::~TelnetApp()
{
    cancelAndDelete(timeoutMsg);
    cancelAndDelete(readDelayTimer);
}

void TelnetApp::checkedScheduleAt(simtime_t t, cMessage *msg)
{
    if (stopTime < SIMTIME_ZERO || t < stopTime)
        scheduleAt(t, msg);
}

void TelnetApp::initialize(int stage)
{
    TcpAppBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numCharsToType = numLinesToType = 0;
        WATCH(numCharsToType);
        WATCH(numLinesToType);
        simtime_t startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        timeoutMsg = new cMessage("timer");
        readDelayTimer = new cMessage("readDelayTimer");
    }
}

void TelnetApp::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t now = simTime();
    simtime_t startTime = par("startTime");
    simtime_t start = std::max(startTime, now);
    if (timeoutMsg && ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime))) {
        timeoutMsg->setKind(MSGKIND_CONNECT);
        scheduleAt(start, timeoutMsg);
    }
}

void TelnetApp::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(timeoutMsg);
    cancelEvent(readDelayTimer);
    if (socket.isOpen()) {
        close();
        delayActiveOperationFinish(par("stopOperationTimeout"));
    }
}

void TelnetApp::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(timeoutMsg);
    cancelEvent(readDelayTimer);
    if (operation->getRootModule() != getContainingNode(this))
        socket.destroy();
}

void TelnetApp::handleSenderTimer(cMessage *msg)
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
                checkedScheduleAt(simTime() + par("keyPressDelay"), timeoutMsg);
                numCharsToType--;
            }
            else {
                EV_INFO << "user hits Enter key\n";
                // Note: reply length must be at least 2, otherwise we'll think
                // it's an echo when it comes back!
                sendGenericAppMsg(1, 2 + par("commandOutputLength").intValue());
                numCharsToType = par("commandLength");

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

void TelnetApp::handleReadTimer(cMessage *msg)
{
    socket.read(par("readSize"));
}

void TelnetApp::handleTimer(cMessage *msg)
{
    if (msg == timeoutMsg)
        handleSenderTimer(msg);
    else if (msg == readDelayTimer)
        handleReadTimer(msg);
    else
        throw cRuntimeError("Unknown timer arrived");
}

void TelnetApp::sendGenericAppMsg(int numBytes, int expectedReplyBytes)
{
    EV_INFO << "sending " << numBytes << " bytes, expecting " << expectedReplyBytes << endl;

    const auto& payload = makeShared<GenericAppMsg>();
    Packet *packet = new Packet("data");
    payload->setChunkLength(B(numBytes));
    payload->setExpectedReplyLength(B(expectedReplyBytes));
    payload->setServerClose(false);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);

    sendPacket(packet);
}

void TelnetApp::socketEstablished(TcpSocket *socket, Indication *indication)
{
    TcpAppBase::socketEstablished(socket, indication);

    // schedule first sending
    numLinesToType = par("numCommands");
    numCharsToType = par("commandLength");
    timeoutMsg->setKind(numLinesToType > 0 ? MSGKIND_SEND : MSGKIND_CLOSE);
    checkedScheduleAt(simTime() + par("thinkTime"), timeoutMsg);
    sendOrScheduleReadCommandIfNeeded();
}

void TelnetApp::socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent)
{
    int len = msg->getByteLength();
    TcpAppBase::socketDataArrived(socket, msg, urgent);

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
            cancelEvent(timeoutMsg);
            timeoutMsg->setKind(MSGKIND_CLOSE);
            checkedScheduleAt(simTime() + par("thinkTime"), timeoutMsg);
        }
        else {
            EV_INFO << "user looks at output, then starts typing next command\n";
            if (!timeoutMsg->isScheduled()) {
                timeoutMsg->setKind(MSGKIND_SEND);
                checkedScheduleAt(simTime() + par("thinkTime"), timeoutMsg);
            }
        }
    }
    sendOrScheduleReadCommandIfNeeded();
}

void TelnetApp::socketClosed(TcpSocket *socket)
{
    TcpAppBase::socketClosed(socket);
    cancelEvent(timeoutMsg);
    cancelEvent(readDelayTimer);
    if (operationalState == State::OPERATING) {
        // start another session after a delay
        timeoutMsg->setKind(MSGKIND_CONNECT);
        checkedScheduleAt(simTime() + par("idleInterval"), timeoutMsg);
    }
    else if (operationalState == State::STOPPING_OPERATION) {
        if (!this->socket.isOpen())
            startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
    }
}

void TelnetApp::socketFailure(TcpSocket *socket, int code)
{
    TcpAppBase::socketFailure(socket, code);

    // reconnect after a delay
    cancelEvent(timeoutMsg);
    cancelEvent(readDelayTimer);
    timeoutMsg->setKind(MSGKIND_CONNECT);
    checkedScheduleAt(simTime() + par("reconnectInterval"), timeoutMsg);
}

void TelnetApp::close()
{
    cancelEvent(readDelayTimer);
    TcpAppBase::close();
}

void TelnetApp::sendOrScheduleReadCommandIfNeeded()
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

