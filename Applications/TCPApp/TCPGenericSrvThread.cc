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


Register_Class(TCPGenericSrvThread);


void TCPGenericSrvThread::established()
{
    // some initialization
    maxMsgDelay = 0;
}

void TCPGenericSrvThread::dataArrived(cMessage *msg, bool)
{
    GenericAppMsg *appmsg = dynamic_cast<GenericAppMsg *>(msg);
    if (!appmsg)
        opp_error("Message (%s)%s is not a GenericAppMsg -- "
                  "probably wrong client app, or wrong setting of TCP's "
                  "sendQueueClass/receiveQueueClass parameters "
                  "(try \"TCPMsgBasedSendQueue\" and \"TCPMsgBasedRcvQueue\")",
                  msg->className(), msg->name());

    long requestedBytes = appmsg->expectedReplyLength();

    simtime_t msgDelay = appmsg->replyDelay();
    if (msgDelay>maxMsgDelay)
        maxMsgDelay = msgDelay;

    bool doClose = appmsg->close();

    if (requestedBytes==0)
    {
        delete msg;
    }
    else
    {
        msg->setLength(requestedBytes*8);
//        sendOrSchedule(msg, delay+msgDelay);
    }

    if (doClose)
    {
        socket()->close();
        //sendOrSchedule(msg, delay+maxMsgDelay);
    }
}

void TCPGenericSrvThread::timerExpired(cMessage *timer)
{
}


