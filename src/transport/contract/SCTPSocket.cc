//
// Copyright (C) 2008 Irene Ruengeler
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <omnetpp.h>
#include "SCTPSocket.h"
#include "SCTP.h"
#include "SCTPAssociation.h"

SCTPSocket::SCTPSocket(bool type)
{
    sockstate = NOT_BOUND;
    localPrt = remotePrt = 0;
    cb = NULL;
    yourPtr = NULL;
    gateToSctp = NULL;
    lastStream=-1;
    oneToOne = type;
    if (oneToOne)
        assocId = SCTP::getNewConnId();
    else
        assocId = 0;
    sctpEV3 << "sockstate=" << sockstate << "\n";
}

SCTPSocket:: ~SCTPSocket()
{
    localAddresses.clear();
}

const char *SCTPSocket::stateName(int state)
{
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (state)
    {
        CASE(NOT_BOUND);
        CASE(CLOSED);
        CASE(LISTENING);
        CASE(CONNECTING);
        CASE(CONNECTED);
        CASE(PEER_CLOSED);
        CASE(LOCALLY_CLOSED);
        CASE(SOCKERROR);
    }
    return s;
#undef CASE
}

void SCTPSocket::sendToSCTP(cPacket *msg)
{
    if (!gateToSctp)
        opp_error("SCTPSocket: setOutputGate() must be invoked before socket can be used");
    check_and_cast<cSimpleModule *>(gateToSctp->getOwnerModule())->send(msg, gateToSctp);
}

void SCTPSocket::bind(int lPort)
{
    if (sockstate!=NOT_BOUND)
        opp_error("SCTPSocket::bind(): socket already bound");
    localAddresses.push_back(IPvXAddress("0.0.0.0"));
    localPrt = lPort;
    sockstate = CLOSED;
}

void SCTPSocket::bind(IPvXAddress lAddr, int lPort)
{
    sctpEV3 << "bind address " << lAddr << "\n";
    if (sockstate!=NOT_BOUND)
        opp_error("SCTPSocket::bind(): socket already bound");
    localAddresses.push_back(lAddr);
    localPrt = lPort;
    sockstate = CLOSED;
}

void SCTPSocket::addAddress(IPvXAddress addr)
{
    sctpEV3 << "add address " << addr << "\n";
    localAddresses.push_back(addr);
}

void SCTPSocket::bindx(AddressVector lAddresses, int lPort)
{
    IPvXAddress lAddr;
    for (AddressVector::iterator i=lAddresses.begin(); i!=lAddresses.end(); ++i)
    {
        ev << "bindx: bind address " << (*i) << "\n";
        localAddresses.push_back((*i));
    }
    localPrt = lPort;
    sockstate = CLOSED;
}

void SCTPSocket::listen(bool fork, uint32 requests, uint32 messagesToPush)
{
    if (sockstate!=CLOSED)
        opp_error(sockstate==NOT_BOUND ? "SCTPSocket: must call bind() before listen()"
                                       : "SCTPSocket::listen(): connect() or listen() already called");

    cPacket *msg = new cPacket("PassiveOPEN", SCTP_C_OPEN_PASSIVE);

    SCTPOpenCommand *openCmd = new SCTPOpenCommand();
    //openCmd->setLocalAddr(localAddr);
    openCmd->setLocalAddresses(localAddresses);
    openCmd->setLocalPort(localPrt);
    if (oneToOne)
        openCmd->setAssocId(assocId);
    else
        openCmd->setAssocId(SCTP::getNewConnId());
    openCmd->setFork(fork);
    openCmd->setInboundStreams(inboundStreams);
    openCmd->setOutboundStreams(outboundStreams);
    openCmd->setNumRequests(requests);
    openCmd->setMessagesToPush(messagesToPush);
    msg->setControlInfo(openCmd);
    sctpEV3 << "Assoc " << openCmd->getAssocId() << "::send PassiveOPEN to SCTP from socket:listen \n";

    sendToSCTP(msg);
    sockstate = LISTENING;
}

void SCTPSocket::connect(IPvXAddress remoteAddress, int32 remotePort, uint32 numRequests)
{
    sctpEV3 << "Socket connect. Assoc=" << assocId << ", sockstate=" << sockstate << "\n";
    if (oneToOne && sockstate!=NOT_BOUND && sockstate!=CLOSED)
        opp_error( "SCTPSocket::connect(): connect() or listen() already called");
    else if (!oneToOne && sockstate!=LISTENING)
        opp_error( "SCTPSocket::connect: One-to-many style socket must be listening");
    cPacket *msg = new cPacket("Associate", SCTP_C_ASSOCIATE);
    remoteAddr = remoteAddress;
    remotePrt = remotePort;
    SCTPOpenCommand *openCmd = new SCTPOpenCommand();
    if (oneToOne)
        openCmd->setAssocId(assocId);
    else
        openCmd->setAssocId(SCTP::getNewConnId());
    sctpEV3 << "Socket connect. Assoc=" << openCmd->getAssocId() << ", sockstate=" << stateName(sockstate) << "\n";
    //openCmd->setAssocId(assocId);
    openCmd->setLocalAddresses(localAddresses);
    openCmd->setLocalPort(localPrt);
    openCmd->setRemoteAddr(remoteAddr);
    openCmd->setRemotePort(remotePrt);
    openCmd->setOutboundStreams(outboundStreams);
    openCmd->setOutboundStreams(inboundStreams);
    openCmd->setNumRequests(numRequests);
    msg->setControlInfo(openCmd);
    sendToSCTP(msg);
    if (oneToOne)
        sockstate = CONNECTING;
}


void SCTPSocket::connectx(AddressVector remoteAddressList, int32 remotePort, uint32 numRequests)
{
    sctpEV3 << "Socket connectx.  sockstate=" << sockstate << "\n";
    /*if (sockstate!=NOT_BOUND && sockstate!=CLOSED)
        opp_error( "SCTPSocket::connect(): connect() or listen() already called");*/
    if (oneToOne && sockstate!=NOT_BOUND && sockstate!=CLOSED)
        opp_error( "SCTPSocket::connect(): connect() or listen() already called");
    else if (!oneToOne && sockstate!=LISTENING)
        opp_error( "SCTPSocket::connect: One-to-many style socket must be listening");
    cPacket *msg = new cPacket("Associate", SCTP_C_ASSOCIATE);
    remoteAddresses = remoteAddressList;
    remoteAddr = remoteAddresses.front();
    remotePrt = remotePort;
    SCTPOpenCommand *openCmd = new SCTPOpenCommand();
    openCmd->setAssocId(assocId);
    openCmd->setLocalAddresses(localAddresses);
    openCmd->setLocalPort(localPrt);
    openCmd->setRemoteAddr(remoteAddr);
    openCmd->setRemoteAddresses(remoteAddresses);
    openCmd->setRemotePort(remotePrt);
    openCmd->setOutboundStreams(outboundStreams);
    openCmd->setNumRequests(numRequests);
    msg->setControlInfo(openCmd);
    sendToSCTP(msg);
    if (oneToOne)
        sockstate = CONNECTING;
}

void SCTPSocket::send(cPacket *msg, bool last, bool primary)
{
    if (oneToOne && sockstate!=CONNECTED && sockstate!=CONNECTING && sockstate!=PEER_CLOSED) {
        opp_error("SCTPSocket::send(): not connected or connecting");
   }
    else if (!oneToOne && sockstate!=LISTENING) {
        opp_error( "SCTPSocket::send: One-to-many style socket must be listening");
   }
    SCTPSendCommand *cmd = new SCTPSendCommand();
    cmd->setAssocId(assocId);
    if (msg->getKind() == SCTP_C_SEND_ORDERED)
        cmd->setSendUnordered(COMPLETE_MESG_ORDERED);
    else
        cmd->setSendUnordered(COMPLETE_MESG_UNORDERED);
    lastStream=(lastStream+1)%outboundStreams;
    cmd->setSid(lastStream);
    cmd->setLast(last);
    cmd->setPrimary(primary);
    msg->setKind(SCTP_C_SEND);
    msg->setControlInfo(cmd);
    sendToSCTP(msg);
}


void SCTPSocket::sendNotification(cPacket *msg)
{
    if (oneToOne && sockstate!=CONNECTED && sockstate!=CONNECTING && sockstate!=PEER_CLOSED) {
        opp_error("SCTPSocket::sendNotification(%s): not connected or connecting", msg->getName());
   }
    else if (!oneToOne && sockstate!=LISTENING) {
        opp_error( "SCTPSocket::send: One-to-many style socket must be listening");
   }
    sendToSCTP(msg);
}

void SCTPSocket::sendRequest(cPacket *msg)
{
    sendToSCTP(msg);
}

void SCTPSocket::close()
{
    sctpEV3 << "SCTPSocket: close\n";

    cPacket *msg = new cPacket("CLOSE", SCTP_C_CLOSE);
    SCTPCommand *cmd = new SCTPCommand();
    cmd->setAssocId(assocId);
    msg->setControlInfo(cmd);
    sendToSCTP(msg);
    sockstate = sockstate==CONNECTED ? LOCALLY_CLOSED : CLOSED;
}

void SCTPSocket::shutdown()
{
    ev << "SCTPSocket: shutdown\n";

    cPacket *msg = new cPacket("Shutdown", SCTP_C_SHUTDOWN);
    SCTPCommand *cmd = new SCTPCommand();
    cmd->setAssocId(assocId);
    msg->setControlInfo(cmd);
    sendToSCTP(msg);
}

void SCTPSocket::abort()
{
    if (sockstate!=NOT_BOUND && sockstate!=CLOSED && sockstate!=SOCKERROR)
    {
        cPacket *msg = new cPacket("ABORT", SCTP_C_ABORT);
        SCTPCommand *cmd = new SCTPCommand();
        //sctpEV3 << "Message cmd=" << &cmd << "\n";
        cmd->setAssocId(assocId);
        msg->setControlInfo(cmd);
        sendToSCTP(msg);
    }
    sockstate = CLOSED;
}

void SCTPSocket::requestStatus()
{
    cPacket *msg = new cPacket("STATUS", SCTP_C_STATUS);
    SCTPCommand *cmd = new SCTPCommand();
    cmd->setAssocId(assocId);
    msg->setControlInfo(cmd);
    sendToSCTP(msg);
}

bool SCTPSocket::belongsToSocket(cPacket *msg)
{
    bool ret= dynamic_cast<SCTPCommand *>(msg->getControlInfo()) &&
           ((SCTPCommand *)(msg->getControlInfo()))->getAssocId()==assocId;
    sctpEV3 << "assoc=" << ((SCTPCommand *)(msg->getControlInfo()))->getAssocId() << "\n";
    return ret;
}

bool SCTPSocket::belongsToAnySCTPSocket(cPacket *msg)
{
    return dynamic_cast<SCTPCommand *>(msg->getControlInfo());
}

void SCTPSocket::setCallbackObject(CallbackInterface *callback, void *yourPointer)
{
    cb = callback;
    yourPtr = yourPointer;
}

void SCTPSocket::processMessage(cPacket *msg)
{
    SCTPStatusInfo *status;
    switch (msg->getKind())
    {
        case SCTP_I_DATA:
            sctpEV3 << "SCTP_I_DATA\n";
            if (cb)
                cb->socketDataArrived(assocId, yourPtr, msg, false);
            break;
        case SCTP_I_DATA_NOTIFICATION:
            sctpEV3 << "SCTP_I_NOTIFICATION\n";
            if (cb)
                cb->socketDataNotificationArrived(assocId, yourPtr, msg);
            break;
        case SCTP_I_SEND_MSG:
            if (cb)
                cb->sendRequestArrived();
            break;
        case SCTP_I_ESTABLISHED:
        {
            if (oneToOne)
                sockstate = CONNECTED;
            SCTPConnectInfo *connectInfo = check_and_cast<SCTPConnectInfo *>(msg->removeControlInfo());
            localAddr = connectInfo->getLocalAddr();
            remoteAddr = connectInfo->getRemoteAddr();
            localPrt = connectInfo->getLocalPort();
            remotePrt = connectInfo->getRemotePort();;
            fsmStatus = connectInfo->getStatus();
            inboundStreams = connectInfo->getInboundStreams();
            outboundStreams = connectInfo->getOutboundStreams();

            if (cb)
                cb->socketEstablished(assocId, yourPtr, connectInfo->getNumMsgs());
            delete connectInfo;
            break;
        }
        case SCTP_I_PEER_CLOSED:
            sctpEV3 << "peer closed\n";
            if (oneToOne)
                sockstate = sockstate==CONNECTED ? PEER_CLOSED : CLOSED;

            if (cb)
                cb->socketPeerClosed(assocId, yourPtr);
            break;
        case SCTP_I_ABORT:
        case SCTP_I_CONN_LOST:
        case SCTP_I_CLOSED:
            sctpEV3 << "SCTP_I_CLOSED called\n";
            sockstate = CLOSED;

            if (cb)
                cb->socketClosed(assocId, yourPtr);
            break;
        case SCTP_I_CONNECTION_REFUSED:
        case SCTP_I_CONNECTION_RESET:
        case SCTP_I_TIMED_OUT:
            sockstate = SOCKERROR;
            if (cb)
                cb->socketFailure(assocId, yourPtr, msg->getKind());

            break;
        case SCTP_I_STATUS:
            status = check_and_cast<SCTPStatusInfo *>(msg->removeControlInfo());

            if (cb)
                cb->socketStatusArrived(assocId, yourPtr, status);
            delete status;
            break;
        case SCTP_I_SHUTDOWN_RECEIVED:
            sctpEV3 << "SCTP_I_SHUTDOWN_RECEIVED\n";
        if (cb)
                cb->shutdownReceivedArrived(assocId);
            break;
        case SCTP_I_SENDQUEUE_FULL:
            if (cb)
                cb->sendqueueFullArrived(assocId);
            break;
        case SCTP_I_SENDQUEUE_ABATED:
        {
            SCTPCommand *cmd = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
            if (cb)
            {
                cb->sendqueueAbatedArrived(assocId, cmd->getNumMsgs());
            }
            delete cmd;
            break;
        }
        default:
            opp_error("SCTPSocket: invalid msg kind %d, one of the SCTP_I_xxx constants expected", msg->getKind());
    }

    delete msg;
}


