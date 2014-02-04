//
// Copyright 2004 Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


#include "TCPAppBase.h"

#include "IPvXAddressResolver.h"


simsignal_t TCPAppBase::connectSignal = registerSignal("connect");
simsignal_t TCPAppBase::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t TCPAppBase::sentPkSignal = registerSignal("sentPk");


void TCPAppBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == 0)
    {
        numSessions = numBroken = packetsSent = packetsRcvd = bytesSent = bytesRcvd = 0;

        WATCH(numSessions);
        WATCH(numBroken);
        WATCH(packetsSent);
        WATCH(packetsRcvd);
        WATCH(bytesSent);
        WATCH(bytesRcvd);
    }
    else if (stage == 3)
    {
        // parameters
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        socket.readDataTransferModePar(*this);
        socket.bind(*localAddress ? IPvXAddressResolver().resolve(localAddress) : IPvXAddress(), localPort);

        socket.setCallbackObject(this);
        socket.setOutputGate(gate("tcpOut"));

        setStatusString("waiting");
    }
}

void TCPAppBase::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleTimer(msg);
    else
        socket.processMessage(msg);
}

void TCPAppBase::connect()
{
    // we need a new connId if this is not the first connection
    socket.renewSocket();

    // connect
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");

    EV << "issuing OPEN command\n";
    setStatusString("connecting");

    IPvXAddress destination;
    IPvXAddressResolver().tryResolve(connectAddress, destination);
    if (destination.isUnspecified())
        EV << "cannot resolve destination address: " << connectAddress << endl;
    else {
        socket.connect(destination, connectPort);

        numSessions++;
        emit(connectSignal, 1L);
    }
}

void TCPAppBase::close()
{
    setStatusString("closing");
    EV << "issuing CLOSE command\n";
    socket.close();
    emit(connectSignal, -1L);
}

void TCPAppBase::sendPacket(cPacket *msg)
{
    int numBytes = msg->getByteLength();
    emit(sentPkSignal, msg);
    socket.send(msg);

    packetsSent++;
    bytesSent += numBytes;
}

void TCPAppBase::setStatusString(const char *s)
{
    if (ev.isGUI())
        getDisplayString().setTagArg("t", 0, s);
}

void TCPAppBase::socketEstablished(int, void *)
{
    // *redefine* to perform or schedule first sending
    EV << "connected\n";
    setStatusString("connected");
}

void TCPAppBase::socketDataArrived(int, void *, cPacket *msg, bool)
{
    // *redefine* to perform or schedule next sending
    packetsRcvd++;
    bytesRcvd += msg->getByteLength();
    emit(rcvdPkSignal, msg);
    delete msg;
}

void TCPAppBase::socketPeerClosed(int, void *)
{
    // close the connection (if not already closed)
    if (socket.getState() == TCPSocket::PEER_CLOSED)
    {
        EV << "remote TCP closed, closing here as well\n";
        close();
    }
}

void TCPAppBase::socketClosed(int, void *)
{
    // *redefine* to start another session etc.
    EV << "connection closed\n";
    setStatusString("closed");
}

void TCPAppBase::socketFailure(int, void *, int code)
{
    // subclasses may override this function, and add code try to reconnect after a delay.
    EV << "connection broken\n";
    setStatusString("broken");

    numBroken++;
}

void TCPAppBase::finish()
{
    std::string modulePath = getFullPath();

    EV << modulePath << ": opened " << numSessions << " sessions\n";
    EV << modulePath << ": sent " << bytesSent << " bytes in " << packetsSent << " packets\n";
    EV << modulePath << ": received " << bytesRcvd << " bytes in " << packetsRcvd << " packets\n";
}

