//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/tun/TunSocket.h"

#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/tun/TunControlInfo_m.h"

namespace inet {

TunSocket::TunSocket()
{
    socketId = getActiveSimulationOrEnvir()->getUniqueNumber();
}

void TunSocket::setCallback(ICallback *callback)
{
    this->callback = callback;
}

bool TunSocket::belongsToSocket(cMessage *msg) const
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    int msgSocketId = tags.getTag<SocketInd>()->getSocketId();
    return socketId == msgSocketId;
}

void TunSocket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));

    switch (msg->getKind()) {
        case TUN_I_DATA:
            if (callback)
                callback->socketDataArrived(this, check_and_cast<Packet *>(msg));
            else
                delete msg;
            break;
        case TUN_I_CLOSED:
            isOpen_ = false;
            if (callback)
                callback->socketClosed(this);
            delete msg;
            break;
        default:
            throw cRuntimeError("TunSocket: invalid msg kind %d, one of the TUN_I_xxx constants expected", msg->getKind());
    }
}

void TunSocket::open(int interfaceId)
{
    isOpen_ = true;
    this->interfaceId = interfaceId;
    auto request = new Request("OPEN", TUN_C_OPEN);
    TunOpenCommand *command = new TunOpenCommand();
    request->setControlInfo(command);
    sendToTun(request);
}

void TunSocket::send(Packet *packet)
{
    if (interfaceId == -1)
        throw cRuntimeError("Socket is closed");
//    TunSendCommand *command = new TunSendCommand();
//    packet->setControlInfo(command);
    packet->setKind(TUN_C_DATA);
    sendToTun(packet);
}

void TunSocket::close()
{
    auto request = new Request("CLOSE", TUN_C_CLOSE);
    TunCloseCommand *command = new TunCloseCommand();
    request->setControlInfo(command);
    sendToTun(request);
    this->interfaceId = -1;
}

void TunSocket::destroy()
{
    auto request = new Request("DESTROY", TUN_C_DESTROY);
    auto command = new TunDestroyCommand();
    request->setControlInfo(command);
    sendToTun(request);
    this->interfaceId = -1;
}

void TunSocket::sendToTun(cMessage *msg)
{
    if (!outputGate)
        throw cRuntimeError("TunSocket: setOutputGate() must be invoked before socket can be used");
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
    check_and_cast<cSimpleModule *>(outputGate->getOwnerModule())->send(msg, outputGate);
}

} // namespace inet

