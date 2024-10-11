//
// Copyright (C) 2004, 2009 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/common/EchoProtocol.h"

#include <string.h>

#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

namespace inet {

Define_Module(EchoProtocol);

void EchoProtocol::handleMessage(cMessage *msg)
{
    cGate *arrivalGate = msg->getArrivalGate();
    if (!strcmp(arrivalGate->getName(), "ipIn"))
        processPacket(check_and_cast<Packet *>(msg));
}

void EchoProtocol::processPacket(Packet *msg)
{
    const auto& echoHdr = msg->peekAtFront<EchoPacket>();
    switch (echoHdr->getType()) {
        case ECHO_PROTOCOL_REQUEST:
            processEchoRequest(msg);
            break;

        case ECHO_PROTOCOL_REPLY:
            processEchoReply(msg);
            break;

        default:
            throw cRuntimeError("Unknown type %d", echoHdr->getType());
    }
}

void EchoProtocol::processEchoRequest(Packet *request)
{
    // turn request into a reply
    const auto& echoReq = request->popAtFront<EchoPacket>();
    Packet *reply = new Packet((std::string(request->getName()) + "-reply").c_str());
    const auto& echoReply = makeShared<EchoPacket>();
    echoReply->setChunkLength(echoReq->getChunkLength());
    echoReply->setType(ECHO_PROTOCOL_REPLY);
    echoReply->setIdentifier(echoReq->getIdentifier());
    echoReply->setSeqNumber(echoReq->getSeqNumber());
    reply->insertAtBack(echoReply);
    reply->insertAtBack(request->peekData());
    auto addressInd = request->getTag<L3AddressInd>();

    // swap src and dest
    // TODO check what to do if dest was multicast etc?
    auto addressReq = reply->addTag<L3AddressReq>();
    addressReq->setSrcAddress(addressInd->getDestAddress());
    addressReq->setDestAddress(addressInd->getSrcAddress());

    reply->addTag<DispatchProtocolReq>()->setProtocol(request->getTag<NetworkProtocolInd>()->getProtocol());
    reply->addTag<PacketProtocolTag>()->setProtocol(&Protocol::echo);
    send(reply, "ipOut");
    delete request;
}

void EchoProtocol::processEchoReply(Packet *reply)
{
    delete reply;
}

} // namespace inet

