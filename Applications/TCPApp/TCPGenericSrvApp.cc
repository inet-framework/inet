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
#include "TCPCommand_m.h"
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
    msgsSent++;
    bytesSent += msg->byteLength();

    EV << "sending \"" << msg->name() << "\" to TCP, " << msg->byteLength() << " bytes\n";
    send(msg, "tcpOut");
}

void TCPGenericSrvApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        sendBack(msg);
    }
    else if (msg->kind()==TCP_I_PEER_CLOSED)
    {
        // we'll close too, but only after there's surely no message
        // pending to be sent back in this connection
        msg->setName("close");
        msg->setKind(TCP_C_CLOSE);
        sendOrSchedule(msg,delay+maxMsgDelay);
    }
    else if (msg->kind()==TCP_I_DATA || msg->kind()==TCP_I_URGENT_DATA)
    {
        msgsRcvd++;
        bytesRcvd += msg->byteLength();

        GenericAppMsg *appmsg = dynamic_cast<GenericAppMsg *>(msg);
        if (!appmsg)
            error("Message (%s)%s is not a GenericAppMsg -- "
                  "probably wrong client app, or wrong setting of TCP's "
                  "sendQueueClass/receiveQueueClass parameters "
                  "(try \"TCPMsgBasedSendQueue\" and \"TCPMsgBasedRcvQueue\")",
                  msg->className(), msg->name());

        long requestedBytes = appmsg->expectedReplyLength();

        simtime_t msgDelay = appmsg->replyDelay();
        if (msgDelay>maxMsgDelay)
            maxMsgDelay = msgDelay;

        bool doClose = appmsg->close();
        int connId = check_and_cast<TCPCommand *>(msg->controlInfo())->connId();

        if (requestedBytes==0)
        {
            delete msg;
        }
        else
        {
            delete msg->removeControlInfo();
            TCPSendCommand *cmd = new TCPSendCommand();
            cmd->setConnId(connId);
            msg->setControlInfo(cmd);

            // set length and send it back
            msg->setKind(TCP_C_SEND);
            msg->setByteLength(requestedBytes);
            sendOrSchedule(msg, delay+msgDelay);
        }

        if (doClose)
        {
            cMessage *msg = new cMessage("close");
            msg->setKind(TCP_C_CLOSE);
            TCPCommand *cmd = new TCPCommand();
            cmd->setConnId(connId);
            msg->setControlInfo(cmd);
            sendOrSchedule(msg, delay+maxMsgDelay);
        }
    }
    else
    {
        // some indication -- ignore
        delete msg;
    }

    if (ev.isGUI())
    {
        char buf[64];
        sprintf(buf, "rcvd: %ld pks %ld bytes\nsent: %ld pks %ld bytes", msgsRcvd, bytesRcvd, msgsSent, bytesSent);
        displayString().setTagArg("t",0,buf);
    }
}

void TCPGenericSrvApp::finish()
{
    EV << fullPath() << ": sent " << bytesSent << " bytes in " << msgsSent << " packets\n";
    EV << fullPath() << ": received " << bytesRcvd << " bytes in " << msgsRcvd << " packets\n";

    recordScalar("packets sent", msgsSent);
    recordScalar("packets rcvd", msgsRcvd);
    recordScalar("bytes sent", bytesSent);
    recordScalar("bytes rcvd", bytesRcvd);
}

