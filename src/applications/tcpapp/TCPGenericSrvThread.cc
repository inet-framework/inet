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
    // no initialization needed
}

void TCPGenericSrvThread::dataArrived(cMessage *msg, bool)
{
    GenericAppMsg *appmsg = dynamic_cast<GenericAppMsg *>(msg);

    if (!appmsg)
        throw cRuntimeError("Message (%s)%s is not a GenericAppMsg -- "
                  "probably wrong client app, or wrong setting of TCP's "
                  "dataTransferMode parameters "
                  "(try \"object\")",
                  msg->getClassName(), msg->getName());

    if (appmsg->getReplyDelay() > 0)
        throw cRuntimeError("Cannot process (%s)%s: %s class doesn't support replyDelay field"
                  " of GenericAppMsg, try to use TCPGenericSrvApp instead",
                  msg->getClassName(), msg->getName(), getClassName());

    // process message: send back requested number of bytes, then close
    // connection if that was requested too
    long requestedBytes = appmsg->getExpectedReplyLength();
    bool doClose = appmsg->getServerClose();

    if (requestedBytes == 0)
    {
        delete appmsg;
    }
    else
    {
        appmsg->setByteLength(requestedBytes);
        delete appmsg->removeControlInfo();
        getSocket()->send(appmsg);
    }

    if (doClose)
        getSocket()->close();
}

void TCPGenericSrvThread::timerExpired(cMessage *timer)
{
    // no timers in this serverThread
}

