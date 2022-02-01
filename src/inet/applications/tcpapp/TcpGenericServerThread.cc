//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpGenericServerThread.h"

#include "inet/applications/tcpapp/GenericAppMsg_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"

namespace inet {

Register_Class(TcpGenericServerThread);

void TcpGenericServerThread::established()
{
    // no initialization needed
}

void TcpGenericServerThread::dataArrived(Packet *msg, bool)
{
    const auto& appmsg = msg->peekData<GenericAppMsg>();

    if (!appmsg)
        throw cRuntimeError("Message (%s)%s is not a GenericAppMsg -- probably wrong client app",
                msg->getClassName(), msg->getName());

    if (appmsg->getReplyDelay() > 0)
        throw cRuntimeError("Cannot process (%s)%s: %s class doesn't support replyDelay field"
                            " of GenericAppMsg, try to use TcpGenericServerApp instead",
                msg->getClassName(), msg->getName(), getClassName());

    // process message: send back requested number of bytes, then close
    // connection if that was requested too
    B requestedBytes = appmsg->getExpectedReplyLength();
    bool doClose = appmsg->getServerClose();

    if (requestedBytes > B(0)) {
        Packet *outPacket = new Packet(msg->getName());
        const auto& payload = makeShared<ByteCountChunk>(requestedBytes);
        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        outPacket->insertAtBack(payload);
        getSocket()->send(outPacket);
    }

    if (doClose)
        getSocket()->close();
    delete msg;
}

void TcpGenericServerThread::timerExpired(cMessage *timer)
{
    // no timers in this serverThread
}

} // namespace inet

