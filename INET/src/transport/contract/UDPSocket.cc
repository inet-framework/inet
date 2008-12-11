//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "UDPSocket.h"


UDPSocket::UDPSocket()
{
    // don't allow user-specified sockIds because they may conflict with
    // automatically assigned ones.
    sockId = generateSocketId();
    usrId = -1;
    sockstate = NOT_BOUND;

    localPrt = remotePrt = 0;
    mcastIfaceId = -1;
    cb = NULL;
    yourPtr = NULL;

    gateToUdp = NULL;
}

const char *UDPSocket::stateName(int state)
{
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (state)
    {
        CASE(NOT_BOUND);
        CASE(BOUND);
    }
    return s;
#undef CASE
}

int UDPSocket::generateSocketId()
{
    return ev.getUniqueNumber();
}

void UDPSocket::sendToUDP(cMessage *msg)
{
    if (!gateToUdp)
        opp_error("UDPSocket: setOutputGate() must be invoked before socket can be used");

    check_and_cast<cSimpleModule *>(gateToUdp->getOwnerModule())->send(msg, gateToUdp);
}

void UDPSocket::setUserId(int userId)
{
    if (sockstate!=NOT_BOUND)
        opp_error("UDPSocket::setUserId(): cannot change userId after socket is bound");
    usrId = userId;
}

void UDPSocket::bind(int lPort)
{
    if (sockstate!=NOT_BOUND)
        opp_error("UDPSocket::bind(): socket already bound");
    if (lPort<=0 || lPort>65535)
        opp_error("UDPSocket::bind(): invalid port number %d", lPort);

    localPrt = lPort;

    UDPControlInfo *ctrl = new UDPControlInfo();
    ctrl->setSockId(sockId);
    ctrl->setUserId(usrId);
    ctrl->setSrcPort(localPrt);
    cMessage *msg = new cMessage("BIND", UDP_C_BIND);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);

    sockstate = BOUND;
}

void UDPSocket::bind(IPvXAddress lAddr, int lPort)
{
    if (sockstate!=NOT_BOUND)
        opp_error("UDPSocket::bind(): socket already bound");
    if (lPort<=0 || lPort>65535)
        opp_error("UDPSocket::bind(): invalid port number %d", lPort);

    localAddr = lAddr;
    localPrt = lPort;

    UDPControlInfo *ctrl = new UDPControlInfo();
    ctrl->setSockId(sockId);
    ctrl->setUserId(usrId);
    ctrl->setSrcAddr(localAddr);
    ctrl->setSrcPort(localPrt);
    cMessage *msg = new cMessage("BIND", UDP_C_BIND);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);

    sockstate = BOUND;
}

void UDPSocket::connect(IPvXAddress addr, int port)
{
    if (sockstate!=BOUND)
        opp_error( "UDPSocket::connect(): socket must be bound before connect() can be called");
    if (addr.isUnspecified())
        opp_error("UDPSocket::connect(): unspecified remote address");
    if (port<=0 || port>65535)
        opp_error("UDPSocket::connect(): invalid remote port number %d", port);

    remoteAddr = addr;
    remotePrt = port;

    UDPControlInfo *ctrl = new UDPControlInfo();
    ctrl->setSockId(sockId);
    ctrl->setDestAddr(remoteAddr);
    ctrl->setDestPort(remotePrt);
    cMessage *msg = new cMessage("CONNECT", UDP_C_CONNECT);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::sendTo(cMessage *msg, IPvXAddress destAddr, int destPort)
{
    msg->setKind(UDP_C_DATA);
    UDPControlInfo *ctrl = new UDPControlInfo();
    ctrl->setSockId(sockId);
    ctrl->setSrcAddr(localAddr);
    ctrl->setSrcPort(localPrt);
    ctrl->setDestAddr(destAddr);
    ctrl->setDestPort(destPort);
    ctrl->setInterfaceId(mcastIfaceId);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::send(cMessage *msg)
{
    if (remoteAddr.isUnspecified() || remotePrt==0)
        opp_error("UDPSocket::send(): must call connect() before using send()");
    sendTo(msg, remoteAddr, remotePrt);
}

void UDPSocket::close()
{
    if (sockstate!=BOUND)
        return;

    cMessage *msg = new cMessage("UNBIND", UDP_C_UNBIND);
    UDPControlInfo *ctrl = new UDPControlInfo();
    ctrl->setSockId(sockId);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
    sockstate = NOT_BOUND;
}

bool UDPSocket::belongsToSocket(cMessage *msg)
{
    return dynamic_cast<UDPControlInfo *>(msg->getControlInfo()) &&
           ((UDPControlInfo *)(msg->getControlInfo()))->getSockId()==sockId;
}

bool UDPSocket::belongsToAnyUDPSocket(cMessage *msg)
{
    return dynamic_cast<UDPControlInfo *>(msg->getControlInfo());
}

void UDPSocket::setCallbackObject(CallbackInterface *callback, void *yourPointer)
{
    cb = callback;
    yourPtr = yourPointer;
}

void UDPSocket::processMessage(cMessage *msg)
{
    UDPControlInfo *ctrl = check_and_cast<UDPControlInfo *>(msg->removeControlInfo());
    ASSERT(ctrl->getSockId()==sockId);

    switch (msg->getKind())
    {
        case UDP_I_DATA:
             if (cb)
                 cb->socketDatagramArrived(sockId, yourPtr, msg, ctrl);
             else {
                 delete msg;
                 delete ctrl;
             }
             break;
        case UDP_I_ERROR:
             sockstate = NOT_BOUND;
             delete msg;
             if (cb)
                 cb->socketPeerClosed(sockId, yourPtr);
             break;
        default:
             opp_error("UDPSocket: invalid msg kind %d, one of the UDP_I_xxx constants expected", msg->getKind());
    }
}

