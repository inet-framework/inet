//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/socket/SocketBase.h"

#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketCommand_m.h"
#include "inet/common/socket/SocketTag_m.h"

namespace inet {

SocketBase::SocketBase()
{
    // don't allow user-specified socketIds because they may conflict with
    // automatically assigned ones.
    socketId = getActiveSimulationOrEnvir()->getUniqueNumber();
    outputGate = nullptr;
}

void SocketBase::sendOut(cMessage *msg)
{
    if (!isOpen_)
        throw cRuntimeError("Socket is closed");
    if (!outputGate)
        throw cRuntimeError("Socket output gate is not set");
    EV_DEBUG << "Sending message" << EV_FIELD(msg) << EV_ENDL;
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    check_and_cast<cSimpleModule *>(outputGate->getOwnerModule())->send(msg, outputGate);
}

void SocketBase::sendOut(Request *request)
{
    sendOut(static_cast<cMessage *>(request));
}

void SocketBase::sendOut(Packet *packet)
{
    sendOut(static_cast<cMessage *>(packet));
}

void SocketBase::send(Packet *packet)
{
    packet->setKind(SOCKET_C_DATA);
    isOpen_ = true;
    sendOut(packet);
}

void SocketBase::close()
{
    auto request = new Request("CLOSE", SOCKET_C_CLOSE);
    auto *ctrl = new SocketCloseCommand();
    request->setControlInfo(ctrl);
    isOpen_ = true;
    sendOut(request);
}

void SocketBase::destroy()
{
    auto request = new Request("destroy", SOCKET_C_DESTROY);
    auto *ctrl = new SocketDestroyCommand();
    request->setControlInfo(ctrl);
    sendOut(request);
    isOpen_ = false;
}

bool SocketBase::belongsToSocket(cMessage *msg) const
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    const auto& socketInd = tags.findTag<SocketInd>();
    return socketInd != nullptr && socketInd->getSocketId() == socketId;
}

} // namespace inet

