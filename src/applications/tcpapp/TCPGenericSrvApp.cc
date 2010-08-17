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


#include "TCPGenericSrvApp.h"
#include "TCPSocket.h"
#include "TCPCommand.h"
#include "GenericAppMsg_m.h"


Define_Module(TCPGenericSrvApp);

void TCPGenericSrvApp::initialize()
{
    const char *address = par("address");
    int port = par("port");
    delay = par("replyDelay");
    maxMsgDelay = 0;

    msgsRcvd = msgsSent = bytesRcvd = bytesSent = 0;
    WATCH(msgsRcvd);
    WATCH(msgsSent);
    WATCH(bytesRcvd);
    WATCH(bytesSent);

    TCPSocket socket;
    socket.setOutputGate(gate("tcpOut"));
    socket.setDataTransferMode(TCP_TRANSFER_OBJECT);
    socket.bind(address[0] ? IPvXAddress(address) : IPvXAddress(), port);
    socket.listen();
}

void TCPGenericSrvApp::sendOrSchedule(cMessage *msg, simtime_t delay)
{
    if (delay==0)
        sendBack(msg);
    else
        scheduleAt(simTime()+delay, msg);
}

void TCPGenericSrvApp::sendBack(cMessage *msg)
{
    GenericAppMsg *appmsg = dynamic_cast<GenericAppMsg*>(msg);

    if (appmsg)
    {
        msgsSent++;
        bytesSent += appmsg->getByteLength();

        EV << "sending \"" << appmsg->getName() << "\" to TCP, " << appmsg->getByteLength() << " bytes\n";
    }
    else
    {
        EV << "sending \"" << msg->getName() << "\" to TCP\n";
    }

    send(msg, "tcpOut");
}

void TCPGenericSrvApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        sendBack(msg);
    }
    else if (msg->getKind()==TCP_I_PEER_CLOSED)
    {
        // we'll close too, but only after there's surely no message
        // pending to be sent back in this connection
        msg->setName("close");
        msg->setKind(TCP_C_CLOSE);
        sendOrSchedule(msg,delay+maxMsgDelay);
    }
    else if (msg->getKind()==TCP_I_DATA || msg->getKind()==TCP_I_URGENT_DATA)
    {
        handleTCPDataMessage(msg);
    }
    else
    {
        // some indication -- ignore
    	EV << "drop msg: " << msg->getName() << ", kind:"<< msg->getKind() << endl;
        delete msg;
    }

    if (ev.isGUI())
    {
        char buf[64];
        sprintf(buf, "rcvd: %ld pks %ld bytes\nsent: %ld pks %ld bytes", msgsRcvd, bytesRcvd, msgsSent, bytesSent);
        getDisplayString().setTagArg("t",0,buf);
    }
}

void TCPGenericSrvApp::handleTCPDataMessage(cMessage *msg)
{
    TCPDataMsg *tcpMsg = check_and_cast<TCPDataMsg*>(msg);
    bytesRcvd += tcpMsg->getByteLength();

    if (tcpMsg->getIsBegin())
    {
        error("TCPGenericSrvApp doesn't work when object transmitted on first byte");
    }

    cPacket *appPacket = tcpMsg->removeDataObject();
    if (!appPacket)
    {
        delete msg;
        return;
    }

    GenericAppMsg *appmsg = dynamic_cast<GenericAppMsg *>(appPacket);
    if (!appmsg)
    {
        error("Message (%s)%s is not a GenericAppMsg -- "
              "probably wrong client app, or wrong setting of TCP's "
              "dataTransferMode parameters "
              "(try \"object\")",
              appPacket->getClassName(), appPacket->getName());
    }

    msgsRcvd++;

    int connId = check_and_cast<TCPCommand *>(tcpMsg->getControlInfo())->getConnId();
    delete msg;

    long requestedBytes = appmsg->getExpectedReplyLength();
    simtime_t msgDelay = appmsg->getReplyDelay();
    bool doClose = appmsg->getServerClose();

    if (requestedBytes > 0)
    {
        if (msgDelay > maxMsgDelay)
            maxMsgDelay = msgDelay;

        delete appmsg->removeControlInfo();
        TCPSendCommand *cmd = new TCPSendCommand();
        cmd->setConnId(connId);
        appmsg->setControlInfo(cmd);

        // set length and send it back
        appmsg->setKind(TCP_C_SEND);
        appmsg->setByteLength(requestedBytes);
        sendOrSchedule(appmsg, delay+msgDelay);
    }
    else
    {
        delete appmsg;
    }

    if (doClose)
    {
        cMessage *closemsg = new cMessage("close");
        closemsg->setKind(TCP_C_CLOSE);
        TCPCommand *cmd = new TCPCommand();
        cmd->setConnId(connId);
        closemsg->setControlInfo(cmd);
        sendOrSchedule(closemsg, delay+maxMsgDelay);
    }
}

void TCPGenericSrvApp::finish()
{
    EV << getFullPath() << ": sent " << bytesSent << " bytes in " << msgsSent << " packets\n";
    EV << getFullPath() << ": received " << bytesRcvd << " bytes in " << msgsRcvd << " packets\n";

    recordScalar("packets sent", msgsSent);
    recordScalar("packets rcvd", msgsRcvd);
    recordScalar("bytes sent", bytesSent);
    recordScalar("bytes rcvd", bytesRcvd);
}
