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

    msgsRcvd = msgsSent = bytesRcvd = bytesSent = 0;
    WATCH(msgsRcvd);
    WATCH(msgsSent);
    WATCH(bytesRcvd);
    WATCH(bytesSent);

    TCPSocket socket;
    socket.setOutputGate(gate("tcpOut"));
    socket.bind(address[0] ? IPAddress(address) : IPAddress(), port);
    socket.listen(true);
}

void TCPGenericSrvApp::sendOrSchedule(cMessage *msg)
{
    if (delay==0)
    {
        msgsSent++;
        bytesSent += msg->length()/8;
        send(msg, "tcpOut");
    }
    else
    {
        scheduleAt(simTime()+delay, msg);
    }
}

void TCPGenericSrvApp::handleMessage(cMessage *msg)
{
    // FIXME todo: handle "close" and "replyDelay" fields of GenericAppMsg!
    if (msg->isSelfMessage())
    {
        msgsSent++;
        bytesSent += msg->length()/8;
        send(msg, "tcpOut");
    }
    else if (msg->kind()==TCP_I_PEER_CLOSED)
    {
        // we'll close too
        msg->setKind(TCP_C_CLOSE);
        sendOrSchedule(msg);
    }
    else if (msg->kind()==TCP_I_DATA || msg->kind()==TCP_I_URGENT_DATA)
    {
        msgsRcvd++;
        bytesRcvd += msg->length()/8;

        GenericAppMsg *appmsg = dynamic_cast<GenericAppMsg *>(msg);
        if (!appmsg)
            error("Message (%s)%s is not a GenericAppMsg -- "
                  "probably wrong client app, or wrong setting of TCP's "
                  "sendQueueClass/receiveQueueClass parameters "
                  "(try \"TCPMsgBasedSendQueue\" and \"TCPMsgBasedRcvQueue\")",
                  msg->className(), msg->name());

        long requestedBytes = appmsg->expectedReplyLength();

        if (requestedBytes==0)
        {
            delete msg;
        }
        else
        {
            // reverse direction, set length, and send it back
            msg->setKind(TCP_C_SEND);
            TCPCommand *ind = check_and_cast<TCPCommand *>(msg->removeControlInfo());
            TCPSendCommand *cmd = new TCPSendCommand();
            cmd->setConnId(ind->connId());
            msg->setControlInfo(cmd);
            delete ind;

            msg->setLength(requestedBytes*8);

            sendOrSchedule(msg);
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
}

