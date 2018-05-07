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

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/applications/tcpapp/TcpSessionApp.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(TcpSessionApp);

#define MSGKIND_CONNECT    1
#define MSGKIND_SEND       2
#define MSGKIND_CLOSE      3


TcpSessionApp::~TcpSessionApp()
{
    cancelAndDelete(timeoutMsg);
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
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        timeoutMsg = new cMessage("timer");
        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));

        if (isNodeUp()) {
            timeoutMsg->setKind(MSGKIND_CONNECT);
            scheduleAt(tOpen, timeoutMsg);
        }
    }
}

bool TcpSessionApp::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

bool TcpSessionApp::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (static_cast<NodeStartOperation::Stage>(stage) == NodeStartOperation::STAGE_APPLICATION_LAYER) {
            if (simTime() <= tOpen) {
                timeoutMsg->setKind(MSGKIND_CONNECT);
                scheduleAt(tOpen, timeoutMsg);
            }
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (static_cast<NodeShutdownOperation::Stage>(stage) == NodeShutdownOperation::STAGE_APPLICATION_LAYER) {
            cancelEvent(timeoutMsg);
            if (socket.getState() == TcpSocket::CONNECTED || socket.getState() == TcpSocket::CONNECTING || socket.getState() == TcpSocket::PEER_CLOSED)
                close();
            // TODO: wait until socket is closed
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (static_cast<NodeCrashOperation::Stage>(stage) == NodeCrashOperation::STAGE_CRASH)
            cancelEvent(timeoutMsg);
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void TcpSessionApp::handleTimer(cMessage *msg)
{
    switch (msg->getKind()) {
        case MSGKIND_CONNECT:
            if (activeOpen)
                connect(); // sending will be scheduled from socketEstablished()
            else
                ; //TODO
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

Packet *TcpSessionApp::createDataPacket(long sendBytes)
{
    Packet *packet = new Packet("data1");
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
        throw cRuntimeError("Invalid data transfer mode: %d", dataTransferMode);
    auto creationTimeTag = payload->addTag<CreationTimeTag>();
    creationTimeTag->setCreationTime(simTime());
    packet->insertAtBack(payload);
    return packet;
}

void TcpSessionApp::socketEstablished(TcpSocket *socket)
{
    TcpAppBase::socketEstablished(socket);

    ASSERT(commandIndex == 0);
    timeoutMsg->setKind(MSGKIND_SEND);
    simtime_t tSend = commands[commandIndex].tSend;
    scheduleAt(std::max(tSend, simTime()), timeoutMsg);
}

void TcpSessionApp::socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent)
{
    TcpAppBase::socketDataArrived(socket, msg, urgent);
}

void TcpSessionApp::socketClosed(TcpSocket *socket)
{
    TcpAppBase::socketClosed(socket);
    cancelEvent(timeoutMsg);
}

void TcpSessionApp::socketFailure(TcpSocket *socket, int code)
{
    TcpAppBase::socketFailure(socket, code);
    cancelEvent(timeoutMsg);
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
        EV_DEBUG << " add command (" << tSend << "s, " << "B)\n";
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
    std::ostringstream os;
    os << TcpSocket::stateName(socket.getState()) << "\nsent: " << bytesSent << " bytes\nrcvd: " << bytesRcvd << " bytes";
    getDisplayString().setTagArg("t", 0, os.str().c_str());
}

} // namespace inet

