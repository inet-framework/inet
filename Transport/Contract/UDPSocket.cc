//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "UDPSocket.h"


UDPSocket::UDPSocket()
{
    // don't allow user-specified sockIds because they may conflict with
    // automatically assigned ones.
    sockId = ev.getUniqueNumber();
    usrId = -1;
    sockstate = NOT_BOUND;

    localPrt = -1;
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

void UDPSocket::sendToUDP(cMessage *msg)
{
    if (!gateToUdp)
        opp_error("UDPSocket: setOutputGate() must be invoked before socket can be used");

    check_and_cast<cSimpleModule *>(gateToUdp->ownerModule())->send(msg, gateToUdp);
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
    if (lPort<0 || lPort>65535)
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
    if (lPort<0 || lPort>65535)
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

void UDPSocket::bindAndConnect(IPvXAddress lAddr, int lPort, IPvXAddress remoteAddr, int remotePort)
{
    if (sockstate!=NOT_BOUND && sockstate!=BOUND)
        opp_error( "UDPSocket::connect(): connect() or listen() already called (need renewSocket()?)");
    if (lPort<0 || lPort>65535)
        opp_error("UDPSocket::bind(): invalid port number %d", lPort);
    if (remotePort<0 || remotePort>65535)
        opp_error("UDPSocket::connect(): invalid remote port number %d", remotePort);

    localAddr = lAddr;
    localPrt = lPort;

    UDPControlInfo *ctrl = new UDPControlInfo();
    ctrl->setSockId(sockId);
    ctrl->setUserId(usrId);
    ctrl->setSrcAddr(localAddr);
    ctrl->setSrcPort(localPrt);
    ctrl->setDestAddr(remoteAddr);
    ctrl->setDestPort(remotePort);
    cMessage *msg = new cMessage("BIND+CONNECT", UDP_C_BIND);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);

    sockstate = BOUND;
}

void UDPSocket::sendTo(cMessage *msg, IPvXAddress remoteAddr, int remotePort)
{
    msg->setKind(UDP_C_DATA);
    UDPControlInfo *ctrl = new UDPControlInfo();
    ctrl->setSockId(sockId);
    ctrl->setSrcAddr(localAddr);
    ctrl->setSrcPort(localPrt);
    ctrl->setDestAddr(remoteAddr);
    ctrl->setDestPort(remotePort);
    ctrl->setInterfaceId(mcastIfaceId);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
}

void UDPSocket::close()
{
    if (sockstate!=BOUND)
        opp_error("UDPSocket::close(): socket not bound or close() already called");

    cMessage *msg = new cMessage("UNBIND", UDP_C_UNBIND);
    UDPControlInfo *ctrl = new UDPControlInfo();
    ctrl->setSockId(sockId);
    msg->setControlInfo(ctrl);
    sendToUDP(msg);
    sockstate = NOT_BOUND;
}

bool UDPSocket::belongsToSocket(cMessage *msg)
{
    return dynamic_cast<UDPControlInfo *>(msg->controlInfo()) &&
           ((UDPControlInfo *)(msg->controlInfo()))->sockId()==sockId;
}

bool UDPSocket::belongsToAnyUDPSocket(cMessage *msg)
{
    return dynamic_cast<UDPControlInfo *>(msg->controlInfo());
}

void UDPSocket::setCallbackObject(CallbackInterface *callback, void *yourPointer)
{
    cb = callback;
    yourPtr = yourPointer;
}

void UDPSocket::processMessage(cMessage *msg)
{
    UDPControlInfo *ctrl = check_and_cast<UDPControlInfo *>(msg->removeControlInfo());
    ASSERT(ctrl->sockId()==sockId);

    switch (msg->kind())
    {
        case UDP_I_DATA:
             if (cb)
                 cb->socketDatagramArrived(sockId, yourPtr, msg, ctrl);
             else {
                 delete msg;
                 delete ctrl;
             }
             break;
        case UDP_I_PEER_CLOSED:
             sockstate = NOT_BOUND;
             delete msg;
             if (cb)
                 cb->socketPeerClosed(sockId, yourPtr);
             break;
        default:
             opp_error("UDPSocket: invalid msg kind %d, one of the TCP_I_xxx constants expected", msg->kind());
    }
}

