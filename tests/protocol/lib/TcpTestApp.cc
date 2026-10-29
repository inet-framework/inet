//
// Protocol Test Framework for INET -- an application whose TCP sockets a test program drives.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "TcpTestApp.h"

#include "inet/common/Simsignals.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {
namespace protocoltest {

Define_Module(TcpTestApp);

static simsignal_t socketEstablishedSignal = cComponent::registerSignal("socketEstablished");
static simsignal_t socketPeerClosedSignal = cComponent::registerSignal("socketPeerClosed");
static simsignal_t socketClosedSignal = cComponent::registerSignal("socketClosed");
static simsignal_t socketFailureSignal = cComponent::registerSignal("socketFailure");
static simsignal_t socketSendReadySignal = cComponent::registerSignal("socketSendReady");

void TcpTestApp::initialize(int stage)
{
    SimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mainSocket.setOutputGate(gate("socketOut"));
        mainSocket.setCallback(this);
        socketMap.addSocket(&mainSocket);
    }
}

TcpTestApp::~TcpTestApp()
{
    for (auto socket : acceptedSockets)
        delete socket;
}

void TcpTestApp::handleMessage(cMessage *msg)
{
    if (auto socket = check_and_cast_nullable<TcpSocket *>(socketMap.findSocketFor(msg)))
        socket->processMessage(msg);
    else {
        EV_WARN << "TcpTestApp: " << msg->getName() << " arrived for an unknown socket" << endl;
        delete msg;
    }
}

L3Address TcpTestApp::addressOf(const char *nodeName) const
{
    return L3AddressResolver().resolve(nodeName);
}

void TcpTestApp::sendData(TcpSocket& socket, int64_t bytes, bool eor)
{
    Enter_Method("sendData");
    auto packet = new Packet("data", makeShared<ByteCountChunk>(B(bytes)));
    emit(packetSentToLowerSignal, packet);
    socket.send(packet, eor);
}

void TcpTestApp::socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent)
{
    emit(packetReceivedFromLowerSignal, packet);
    delete packet;
}

void TcpTestApp::socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo)
{
    auto newSocket = new TcpSocket(availableInfo);
    newSocket->setOutputGate(gate("socketOut"));
    newSocket->setCallback(this);
    socketMap.addSocket(newSocket);
    acceptedSockets.push_back(newSocket);
    socket->accept(availableInfo->getNewSocketId());
}

void TcpTestApp::socketEstablished(TcpSocket *socket)
{
    emit(socketEstablishedSignal, (intval_t)socket->getSocketId());
}

void TcpTestApp::socketPeerClosed(TcpSocket *socket)
{
    emit(socketPeerClosedSignal, (intval_t)socket->getSocketId());
}

void TcpTestApp::socketClosed(TcpSocket *socket)
{
    emit(socketClosedSignal, (intval_t)socket->getSocketId());
}

void TcpTestApp::socketFailure(TcpSocket *socket, int code)
{
    emit(socketFailureSignal, (intval_t)code);
}

void TcpTestApp::socketSendMsgArrived(TcpSocket *socket, TcpCommand *tcpCommand)
{
    // The command is the control info of a message that the socket deletes.
    emit(socketSendReadySignal, (intval_t)socket->getSocketId());
}

} // namespace protocoltest
} // namespace inet
