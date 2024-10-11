//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpSessionApp.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(TcpSessionApp);

#define MSGKIND_CONNECT    1
#define MSGKIND_SEND       2
#define MSGKIND_CLOSE      3

TcpSessionApp::~TcpSessionApp()
{
    cancelAndDelete(timeoutMsg);
    cancelAndDelete(readDelayTimer);
}

void TcpSessionApp::initialize(int stage)
{
    TcpAppBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        activeOpen = par("active");
        tOpen = par("tOpen");
        tSend = par("tSend");
        tClose = par("tClose");
        sendBytes = par("sendBytes");
        commandIndex = 0;

        const char *script = par("sendScript");
        parseScript(script);

        if (sendBytes > 0 && commands.size() > 0)
            throw cRuntimeError("Cannot use both sendScript and tSend+sendBytes");
        if (sendBytes > 0)
            commands.push_back(Command(tSend, sendBytes));
        if (commands.size() == 0)
            throw cRuntimeError("sendScript is empty");
        timeoutMsg = new cMessage("timer");
        readDelayTimer = new cMessage("readDelayTimer");
    }
}

void TcpSessionApp::handleStartOperation(LifecycleOperation *operation)
{
    if (simTime() <= tOpen) {
        timeoutMsg->setKind(MSGKIND_CONNECT);
        scheduleAt(tOpen, timeoutMsg);
    }
}

void TcpSessionApp::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(timeoutMsg);
    cancelEvent(readDelayTimer);
    if (socket.isOpen())
        close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void TcpSessionApp::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(timeoutMsg);
    cancelEvent(readDelayTimer);
    if (operation->getRootModule() != getContainingNode(this))
        socket.destroy();
}


void TcpSessionApp::handleSenderTimer(cMessage *msg)
{
    switch (msg->getKind()) {
        case MSGKIND_CONNECT:
            if (activeOpen)
                connect(); // sending will be scheduled from socketEstablished()
            else
                throw cRuntimeError("TODO");
            break;

        case MSGKIND_SEND:
            sendData();
            break;

        case MSGKIND_CLOSE:
            close();
            break;

        default:
            throw cRuntimeError("Invalid timer msg: kind=%d", msg->getKind());
    }
}

void TcpSessionApp::handleReadTimer(cMessage *msg)
{
    socket.read(par("readSize"));
}

void TcpSessionApp::handleTimer(cMessage *msg)
{
    if (msg == timeoutMsg)
        handleSenderTimer(msg);
    else if (msg == readDelayTimer)
        handleReadTimer(msg);
    else
        throw cRuntimeError("Unknown timer arrived");
}

void TcpSessionApp::sendData()
{
    long numBytes = commands[commandIndex].numBytes;
    EV_INFO << "sending data with " << numBytes << " bytes\n";
    sendPacket(createDataPacket(numBytes));

    if (++commandIndex < (int)commands.size()) {
        simtime_t tSend = commands[commandIndex].tSend;
        scheduleAt(std::max(tSend, simTime()), timeoutMsg);
    }
    else {
        timeoutMsg->setKind(MSGKIND_CLOSE);
        scheduleAt(std::max(tClose, simTime()), timeoutMsg);
    }
}

void TcpSessionApp::close()
{
    cancelEvent(readDelayTimer);
    TcpAppBase::close();
}

Packet *TcpSessionApp::createDataPacket(long sendBytes)
{
    const char *dataTransferMode = par("dataTransferMode");
    Ptr<Chunk> payload;
    if (!strcmp(dataTransferMode, "bytecount")) {
        payload = makeShared<ByteCountChunk>(B(sendBytes));
    }
    else if (!strcmp(dataTransferMode, "object")) {
        const auto& applicationPacket = makeShared<ApplicationPacket>();
        applicationPacket->setChunkLength(B(sendBytes));
        payload = applicationPacket;
    }
    else if (!strcmp(dataTransferMode, "bytestream")) {
        const auto& bytesChunk = makeShared<BytesChunk>();
        std::vector<uint8_t> vec;
        vec.resize(sendBytes);
        for (int i = 0; i < sendBytes; i++)
            vec[i] = (bytesSent + i) & 0xFF;
        bytesChunk->setBytes(vec);
        payload = bytesChunk;
    }
    else
        throw cRuntimeError("Invalid data transfer mode: %s", dataTransferMode);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    Packet *packet = new Packet("data1");
    packet->insertAtBack(payload);
    return packet;
}

void TcpSessionApp::socketEstablished(TcpSocket *socket, Indication *indication)
{
    TcpAppBase::socketEstablished(socket, indication);

    ASSERT(commandIndex == 0);
    timeoutMsg->setKind(MSGKIND_SEND);
    simtime_t tSend = commands[commandIndex].tSend;
    scheduleAt(std::max(tSend, simTime()), timeoutMsg);
    sendOrScheduleReadCommandIfNeeded();
}

void TcpSessionApp::socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent)
{
    TcpAppBase::socketDataArrived(socket, msg, urgent);
    sendOrScheduleReadCommandIfNeeded();
}

void TcpSessionApp::socketClosed(TcpSocket *socket)
{
    TcpAppBase::socketClosed(socket);
    cancelEvent(timeoutMsg);
    cancelEvent(readDelayTimer);
    if (operationalState == State::STOPPING_OPERATION && !this->socket.isOpen())
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void TcpSessionApp::socketFailure(TcpSocket *socket, int code)
{
    TcpAppBase::socketFailure(socket, code);
    cancelEvent(timeoutMsg);
    cancelEvent(readDelayTimer);
}

void TcpSessionApp::parseScript(const char *script)
{
    const char *s = script;

    EV_DEBUG << "parse script \"" << script << "\"\n";
    while (*s) {
        // parse time
        while (isspace(*s))
            s++;

        if (!*s || *s == ';')
            break;

        const char *s0 = s;
        simtime_t tSend = strtod(s, &const_cast<char *&>(s));

        if (s == s0)
            throw cRuntimeError("Syntax error in script: simulation time expected");

        // parse number of bytes
        while (isspace(*s))
            s++;

        if (!isdigit(*s))
            throw cRuntimeError("Syntax error in script: number of bytes expected");

        long numBytes = strtol(s, nullptr, 10);

        while (isdigit(*s))
            s++;

        // add command
        EV_DEBUG << " add command (" << tSend << "s, " << numBytes << "B)\n";
        commands.push_back(Command(tSend, numBytes));

        // skip delimiter
        while (isspace(*s))
            s++;

        if (!*s)
            break;

        if (*s != ';')
            throw cRuntimeError("Syntax error in script: separator ';' missing");

        s++;

        while (isspace(*s))
            s++;
    }
    EV_DEBUG << "parser finished\n";
}

void TcpSessionApp::finish()
{
    EV << getFullPath() << ": received " << bytesRcvd << " bytes in " << packetsRcvd << " packets\n";
    recordScalar("bytesRcvd", bytesRcvd);
    recordScalar("bytesSent", bytesSent);
}

void TcpSessionApp::refreshDisplay() const
{
    TcpAppBase::refreshDisplay();

    std::ostringstream os;
    os << TcpSocket::stateName(socket.getState()) << "\nsent: " << bytesSent << " bytes\nrcvd: " << bytesRcvd << " bytes";
    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

void TcpSessionApp::sendOrScheduleReadCommandIfNeeded()
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

