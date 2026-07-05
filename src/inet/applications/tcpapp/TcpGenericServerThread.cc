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
    Enter_Method("established");
    // no initialization needed
}

void TcpGenericServerThread::dataArrived(Packet *msg, bool)
{
    Enter_Method("dataArrived");
    // Read the request control from its GenericAppMsgReq region tag (see
    // GenericAppMsg.msg). This simple thread assumes one request per received
    // packet; it does not reassemble a segmented stream.
    const auto& data = msg->peekData();
    const GenericAppMsgReq *appmsg = nullptr;
    for (const auto& regionTag : data->getAllTags<GenericAppMsgReq>()) {
        if (regionTag.getOffset() == b(0)) {
            appmsg = regionTag.getTag().get();
            break;
        }
    }

    if (!appmsg)
        throw cRuntimeError("Message (%s)%s has no GenericAppMsgReq region tag -- either the wrong "
                "client app, or a transport that does not preserve region tags (e.g. TcpLwip)",
                msg->getClassName(), msg->getName());

    if (appmsg->getReplyDelay() > 0)
        throw cRuntimeError("Cannot process (%s)%s: %s class doesn't support the replyDelay field,"
                            " try to use TcpGenericServerApp instead",
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
    ASSERT(getSimulation()->getContext() == this);

    // no timers in this serverThread
}

} // namespace inet

