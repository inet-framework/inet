//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8022/Ieee8022LlcSocket.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocketCommand_m.h"

namespace inet {

void Ieee8022LlcSocket::sendOut(cMessage *msg)
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022llc);
    if (localSap != -1) {
        auto& sapReq = tags.addTagIfAbsent<Ieee802SapReq>();
        if (sapReq->getSsap() == -1)
            sapReq->setSsap(localSap);
    }
    if (remoteSap != -1) {
        auto& sapReq = tags.addTagIfAbsent<Ieee802SapReq>();
        if (sapReq->getDsap() == -1)
            sapReq->setDsap(remoteSap);
    }
    if (interfaceId != -1) {
        auto& interfaceReq = tags.addTagIfAbsent<InterfaceReq>();
        if (interfaceReq->getInterfaceId() == -1)
            interfaceReq->setInterfaceId(interfaceId);
    }
    SocketBase::sendOut(msg);
}

void Ieee8022LlcSocket::open(int interfaceId, int localSap, int remoteSap)
{
    if (localSap < -1 || localSap > 255)
        throw cRuntimeError("Invalid localSap value: %d", localSap);
    if (remoteSap < -1 || remoteSap > 255)
        throw cRuntimeError("Invalid remoteSap value: %d", remoteSap);
    this->interfaceId = interfaceId;
    this->localSap = localSap;
    this->remoteSap = remoteSap;
    auto request = new Request("LLC_OPEN", SOCKET_C_OPEN);
    Ieee8022LlcSocketOpenCommand *command = new Ieee8022LlcSocketOpenCommand();
    command->setLocalSap(localSap);
    request->setControlInfo(command);
    isOpen_ = true;
    sendOut(request);
}

void Ieee8022LlcSocket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));
    switch (msg->getKind()) {
        case SOCKET_I_DATA:
            if (callback)
                callback->socketDataArrived(this, check_and_cast<Packet *>(msg));
            else
                delete msg;
            break;
        case SOCKET_I_CLOSED:
            isOpen_ = false;
            if (callback)
                callback->socketClosed(this);
            delete msg;
            break;
        default:
            throw cRuntimeError("Ieee8022LlcSocket: invalid msg kind %d, one of the SOCKET_I_xxx constants expected", msg->getKind());
            break;
    }
}

} // namespace inet

