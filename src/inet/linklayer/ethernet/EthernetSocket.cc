//
// Copyright (C) OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ethernet/EthernetSocket.h"
#include "inet/linklayer/ethernet/EthernetCommand_m.h"

namespace inet {

EthernetSocket::EthernetSocket()
{
    // don't allow user-specified socketIds because they may conflict with
    // automatically assigned ones.
    socketId = getEnvir()->getUniqueNumber();
    gateToEthernet = nullptr;
}

void EthernetSocket::sendToEthernet(cMessage *msg)
{
    if (!gateToEthernet)
        throw cRuntimeError("EthernetSocket: setOutputGate() must be invoked before socket can be used");

    cObject *ctrl = msg->getControlInfo();
    EV_TRACE << "EthernetSocket: Send (" << msg->getClassName() << ")" << msg->getFullName();
    if (ctrl)
        EV_TRACE << "  control info: (" << ctrl->getClassName() << ")" << ctrl->getFullName();
    EV_TRACE << endl;

    auto& tags = getTags(msg);
    tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceEntry->getInterfaceId());
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);
    check_and_cast<cSimpleModule *>(gateToEthernet->getOwnerModule())->send(msg, gateToEthernet);
}

void EthernetSocket::bind(const MacAddress& sourceAddress, const MacAddress& destinationAddress, Protocol *protocol, int vlanId)
{
    auto request = new Request("BIND", ETHERNET_C_BIND);
    EthernetBindCommand *ctrl = new EthernetBindCommand();
    ctrl->setSourceAddress(sourceAddress);
    ctrl->setDestinationAddress(destinationAddress);
    ctrl->setProtocol(protocol);
    ctrl->setVlanId(vlanId);
    request->setControlInfo(ctrl);
    isOpen_ = true;
    sendToEthernet(request);
}

void EthernetSocket::send(Packet *packet)
{
    packet->setKind(ETHERNET_C_DATA);
    isOpen_ = true;
    sendToEthernet(packet);
}

void EthernetSocket::close()
{
    auto request = new Request("CLOSE", ETHERNET_C_CLOSE);
    auto *ctrl = new EthernetCloseCommand();
    request->setControlInfo(ctrl);
    isOpen_ = true;
    sendToEthernet(request);
}

void EthernetSocket::destroy()
{
    auto request = new Request("destroy", ETHERNET_C_DESTROY);
    auto *ctrl = new EthernetDestroyCommand();
    request->setControlInfo(ctrl);
    sendToEthernet(request);
    isOpen_ = false;
}

void EthernetSocket::setCallback(ICallback *callback)
{
    this->callback = callback;
}

bool EthernetSocket::belongsToSocket(cMessage *msg) const
{
    auto& tags = getTags(msg);
    auto socketInd = tags.findTag<SocketInd>();
    return socketInd != nullptr && socketInd->getSocketId() == socketId;
}

void EthernetSocket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));
    switch (msg->getKind()) {
        case ETHERNET_I_DATA:
            if (callback)
                callback->socketDataArrived(this, check_and_cast<Packet *>(msg));
            else
                delete msg;
            break;
        case ETHERNET_I_SOCKET_CLOSED:
            isOpen_ = false;
            if (callback)
                callback->socketClosed(this);
            delete msg;
            break;
        default:
            throw cRuntimeError("EthernetSocket: invalid msg kind %d, one of the ETHERNNET_I_xxx constants expected", msg->getKind());
            break;
    }
}

} // namespace inet

