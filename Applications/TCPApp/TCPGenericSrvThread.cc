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


#include "TCPGenericSrvThread.h"
#include "GenericAppMsg_m.h"


Define_Module(TCPGenericSrvThread);


void TCPGenericSrvThread::socketEstablished(int, void *)
{
    // nothing to do here
}

void TCPGenericSrvThread::socketDataArrived(int, void *, cMessage *msg, bool urgent)
{
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
        msg->setLength(requestedBytes*8);
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

void TCPGenericSrvThread::socketPeerClosed(int, void *)
{
    // we close too
    socket()->close();
}

