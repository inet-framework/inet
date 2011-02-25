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


#include "TCPEchoApp.h"
#include "TCPSocket.h"
#include "TCPCommand_m.h"


Define_Module(TCPEchoApp);

void TCPEchoApp::initialize()
{
    const char *address = par("address");
    int port = par("port");
    delay = par("echoDelay");
    echoFactor = par("echoFactor");

    bytesRcvd = bytesSent = 0;
    WATCH(bytesRcvd);
    WATCH(bytesSent);

    TCPSocket socket;
    socket.setOutputGate(gate("tcpOut"));
    socket.bind(address[0] ? IPvXAddress(address) : IPvXAddress(), port);
    socket.listen();
}

void TCPEchoApp::sendDown(cMessage *msg)
{
    if (msg->isPacket())
        bytesSent += ((cPacket *)msg)->getByteLength();
    send(msg, "tcpOut");
}

void TCPEchoApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        sendDown(msg);
    }
    else if (msg->getKind()==TCP_I_PEER_CLOSED)
    {
        // we'll close too
        msg->setKind(TCP_C_CLOSE);
        if (delay==0)
            sendDown(msg);
        else
            scheduleAt(simTime()+delay, msg); // send after a delay
    }
    else if (msg->getKind()==TCP_I_DATA || msg->getKind()==TCP_I_URGENT_DATA)
    {
        cPacket *pkt = check_and_cast<cPacket *>(msg);
        bytesRcvd += pkt->getByteLength();

        if (echoFactor==0)
        {
            delete pkt;
        }
        else
        {
            // reverse direction, modify length, and send it back
            pkt->setKind(TCP_C_SEND);
            TCPCommand *ind = check_and_cast<TCPCommand *>(pkt->removeControlInfo());
            TCPSendCommand *cmd = new TCPSendCommand();
            cmd->setConnId(ind->getConnId());
            pkt->setControlInfo(cmd);
            delete ind;

            long byteLen = pkt->getByteLength()*echoFactor;
            if (byteLen<1) byteLen=1;
            pkt->setByteLength(byteLen);

            if (delay==0)
                sendDown(pkt);
            else
                scheduleAt(simTime()+delay, pkt); // send after a delay
        }
    }
    else
    {
        // some indication -- ignore
        delete msg;
    }

    if (ev.isGUI())
    {
        char buf[80];
        sprintf(buf, "rcvd: %ld bytes\nsent: %ld bytes", bytesRcvd, bytesSent);
        getDisplayString().setTagArg("t",0,buf);
    }
}

void TCPEchoApp::finish()
{
    recordScalar("bytesRcvd", bytesRcvd);
    recordScalar("bytesSent", bytesSent);
}
