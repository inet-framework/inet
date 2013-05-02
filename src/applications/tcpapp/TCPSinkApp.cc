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


#include "TCPSinkApp.h"

#include "ModuleAccess.h"
#include "NodeStatus.h"
#include "TCPSocket.h"


Define_Module(TCPSinkApp);

simsignal_t TCPSinkApp::rcvdPkSignal = SIMSIGNAL_NULL;

void TCPSinkApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == 0)
    {
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");

        bytesRcvd = 0;
        WATCH(bytesRcvd);
        rcvdPkSignal = registerSignal("rcvdPk");

        //TODO should use IPvXAddressResolver in stage 3
        TCPSocket socket;
        socket.setOutputGate(gate("tcpOut"));
        socket.readDataTransferModePar(*this);
        socket.bind(localAddress[0] ? IPvXAddress(localAddress) : IPvXAddress(), localPort);
        socket.listen();
    }
    else if (stage == 1)
    {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void TCPSinkApp::handleMessage(cMessage *msg)
{
    if (msg->getKind() == TCP_I_PEER_CLOSED)
    {
        // we close too
        msg->setName("close");
        msg->setKind(TCP_C_CLOSE);
        send(msg, "tcpOut");
    }
    else if (msg->getKind() == TCP_I_DATA || msg->getKind() == TCP_I_URGENT_DATA)
    {
        cPacket *pk = PK(msg);
        long packetLength = pk->getByteLength();
        bytesRcvd += packetLength;
        emit(rcvdPkSignal, pk);
        delete msg;

        if (ev.isGUI())
        {
            char buf[32];
            sprintf(buf, "rcvd: %ld bytes", bytesRcvd);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }
    else
    {
        // must be data or some kind of indication -- can be dropped
        delete msg;
    }
}

void TCPSinkApp::finish()
{
}

