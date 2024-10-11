//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpAppBase.h"

#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

simsignal_t TcpAppBase::connectSignal = registerSignal("connect");

void TcpAppBase::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numSessions = numBroken = packetsSent = packetsRcvd = bytesSent = bytesRcvd = 0;

        WATCH(numSessions);
        WATCH(numBroken);
        WATCH(packetsSent);
        WATCH(packetsRcvd);
        WATCH(bytesSent);
        WATCH(bytesRcvd);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        // parameters
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        autoRead = par("autoRead");
        socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);

        socket.setCallback(this);
        socket.setOutputGate(gate("socketOut"));
    }
}

void TcpAppBase::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleTimer(msg);
    else
        socket.processMessage(msg);
}

void TcpAppBase::connect()
{
    // we need a new connId if this is not the first connection
    socket.renewSocket();

    const char *localAddress = par("localAddress");
    int localPort = par("localPort");
    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);

    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int dscp = par("dscp");
    if (dscp != -1)
        socket.setDscp(dscp);

    int tos = par("tos");
    if (tos != -1)
        socket.setTos(tos);

    // connect
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");

    L3Address destination;
    L3AddressResolver().tryResolve(connectAddress, destination);
    if (destination.isUnspecified()) {
        EV_ERROR << "Connecting to " << connectAddress << " port=" << connectPort << ": cannot resolve destination address\n";
    }
    else {
        EV_INFO << "Connecting to " << connectAddress << "(" << destination << ") port=" << connectPort << endl;

        socket.setAutoRead(par("autoRead"));
        socket.connect(destination, connectPort);

        numSessions++;
        emit(connectSignal, 1L);
    }
}

void TcpAppBase::close()
{
    EV_INFO << "issuing CLOSE command\n";
    socket.close();
    emit(connectSignal, -1L);
}

void TcpAppBase::sendPacket(Packet *msg)
{
    int numBytes = msg->getByteLength();
    emit(packetSentSignal, msg);
    socket.send(msg);

    packetsSent++;
    bytesSent += numBytes;
}

void TcpAppBase::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();
    getDisplayString().setTagArg("t", 0, TcpSocket::stateName(socket.getState()));
}

void TcpAppBase::socketEstablished(TcpSocket *, Indication *indication)
{
    Enter_Method("socketEstablished");
    // *redefine* to perform or schedule first sending
    EV_INFO << "connected\n";
}

void TcpAppBase::socketDataArrived(TcpSocket *, Packet *msg, bool)
{
    Enter_Method("socketDataArrived");
    // *redefine* to perform or schedule next sending
    packetsRcvd++;
    bytesRcvd += msg->getByteLength();
    emit(packetReceivedSignal, msg);
    delete msg;
}

void TcpAppBase::socketPeerClosed(TcpSocket *socket_)
{
    Enter_Method("socketPeerClosed");
    ASSERT(socket_ == &socket);
    // close the connection (if not already closed)
    if (socket.getState() == TcpSocket::PEER_CLOSED) {
        EV_INFO << "remote TCP closed, closing here as well\n";
        close();
    }
}

void TcpAppBase::socketClosed(TcpSocket *)
{
    Enter_Method("socketClosed");
    // *redefine* to start another session etc.
    EV_INFO << "connection closed\n";
}

void TcpAppBase::socketFailure(TcpSocket *, int code)
{
    Enter_Method("socketFailure");
    // subclasses may override this function, and add code try to reconnect after a delay.
    EV_WARN << "connection broken\n";
    numBroken++;
}

void TcpAppBase::finish()
{
    std::string modulePath = getFullPath();

    EV_INFO << modulePath << ": opened " << numSessions << " sessions\n";
    EV_INFO << modulePath << ": sent " << bytesSent << " bytes in " << packetsSent << " packets\n";
    EV_INFO << modulePath << ": received " << bytesRcvd << " bytes in " << packetsRcvd << " packets\n";
}

void TcpAppBase::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    socket.processMessage(packet);
}

cGate *TcpAppBase::lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction)
{
    Enter_Method("lookupModuleInterface");
    EV_TRACE << "Looking up module interface" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
    if (gate->isName("socketIn")) {
        if (type == typeid(IPassivePacketSink)) {
            auto socketInd = dynamic_cast<const SocketInd *>(arguments);
            if (socketInd != nullptr && socketInd->getSocketId() == socket.getSocketId())
                return gate;
            auto packetServiceTag = dynamic_cast<const PacketServiceTag *>(arguments);
            if (packetServiceTag != nullptr && packetServiceTag->getProtocol() == &Protocol::tcp)
                return gate;
        }
    }
    return nullptr;
}

} // namespace inet

