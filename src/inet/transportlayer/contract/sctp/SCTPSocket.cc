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

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/contract/sctp/SCTPSocket.h"

#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"

#ifdef WITH_SCTP
#include "inet/transportlayer/sctp/SCTP.h"
#else // ifdef WITH_SCTP
//#define sctpEV3 (!SCTP::testing==true)?std::cerr:std::cerr
#define sctpEV3    EV
#endif // ifdef WITH_SCTP

namespace inet {

using namespace sctp;

static inline int32_t getNewAssocId()
{
#ifdef WITH_SCTP
    return SCTP::getNewAssocId();
#else // ifdef WITH_SCTP
    return -1;
#endif // ifdef WITH_SCTP
}

SCTPSocket::SCTPSocket(bool type)
{
    sockstate = NOT_BOUND;
    localPrt = remotePrt = 0;
    cb = NULL;
    yourPtr = NULL;
    gateToSctp = NULL;
    lastStream = -1;
    oneToOne = type;
    if (oneToOne)
        assocId = getNewAssocId();
    else
        assocId = 0;
    EV_INFO << "sockstate=" << sockstate << "\n";
}

SCTPSocket::~SCTPSocket()
{
    localAddresses.clear();
}

const char *SCTPSocket::stateName(int state)
{
#define CASE(x)    case x: \
        s = #x; break
    const char *s = "unknown";
    switch (state) {
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
        throw cRuntimeError("SCTPSocket: setOutputGate() must be invoked before socket can be used");

    check_and_cast<cSimpleModule *>(gateToSctp->getOwnerModule())->send(msg, gateToSctp);
}

void SCTPSocket::bind(int lPort)
{
    if (sockstate != NOT_BOUND)
        throw cRuntimeError("SCTPSocket::bind(): socket already bound");

    localAddresses.push_back(L3Address());    // Unspecified address
    localPrt = lPort;
    sockstate = CLOSED;
}

void SCTPSocket::bind(L3Address lAddr, int lPort)
{
    EV_INFO << "bind address " << lAddr << "\n";
    if (sockstate != NOT_BOUND)
        throw cRuntimeError("SCTPSocket::bind(): socket already bound");

    localAddresses.push_back(lAddr);
    localPrt = lPort;
    sockstate = CLOSED;
}

void SCTPSocket::addAddress(L3Address addr)
{
    EV_INFO << "add address " << addr << "\n";
    localAddresses.push_back(addr);
}

void SCTPSocket::bindx(AddressVector lAddresses, int lPort)
{
    L3Address lAddr;
    for (AddressVector::iterator i = lAddresses.begin(); i != lAddresses.end(); ++i) {
        EV << "bindx: bind address " << (*i) << "\n";
        localAddresses.push_back((*i));
    }
    localPrt = lPort;
    sockstate = CLOSED;
}

void SCTPSocket::listen(bool fork, bool reset, uint32 requests, uint32 messagesToPush)
{
    if (sockstate != CLOSED)
        throw cRuntimeError(sockstate == NOT_BOUND ? "SCTPSocket: must call bind() before listen()"
                : "SCTPSocket::listen(): connect() or listen() already called");

    cPacket *msg = new cPacket("PassiveOPEN", SCTP_C_OPEN_PASSIVE);

    SCTPOpenCommand *openCmd = new SCTPOpenCommand();
    //openCmd->setLocalAddr(localAddr);
    openCmd->setLocalAddresses(localAddresses);
    openCmd->setLocalPort(localPrt);
    if (oneToOne)
        openCmd->setAssocId(assocId);
    else
        openCmd->setAssocId(getNewAssocId());
    openCmd->setFork(fork);
    openCmd->setInboundStreams(inboundStreams);
    openCmd->setOutboundStreams(outboundStreams);
    openCmd->setNumRequests(requests);
    openCmd->setStreamReset(reset);
    openCmd->setMessagesToPush(messagesToPush);
    msg->setControlInfo(openCmd);
    EV_INFO << "Assoc " << openCmd->getAssocId() << "::send PassiveOPEN to SCTP from socket:listen \n";

    sendToSCTP(msg);
    sockstate = LISTENING;
}

void SCTPSocket::connect(L3Address remoteAddress, int32 remotePort, bool streamReset, int32 prMethod, uint32 numRequests)
{
    EV_INFO << "Socket connect. Assoc=" << assocId << ", sockstate=" << sockstate << "\n";
    if (oneToOne && sockstate != NOT_BOUND && sockstate != CLOSED)
        throw cRuntimeError("SCTPSocket::connect(): connect() or listen() already called");

    if (!oneToOne && sockstate != LISTENING)
        throw cRuntimeError("SCTPSocket::connect: One-to-many style socket must be listening");

    cPacket *msg = new cPacket("Associate", SCTP_C_ASSOCIATE);
    remoteAddr = remoteAddress;
    remotePrt = remotePort;
    SCTPOpenCommand *openCmd = new SCTPOpenCommand();
    if (oneToOne)
        openCmd->setAssocId(assocId);
    else
        openCmd->setAssocId(getNewAssocId());
    EV_INFO << "Socket connect. Assoc=" << openCmd->getAssocId() << ", sockstate=" << stateName(sockstate) << "\n";
    //openCmd->setAssocId(assocId);
    openCmd->setLocalAddresses(localAddresses);
    openCmd->setLocalPort(localPrt);
    openCmd->setRemoteAddr(remoteAddr);
    openCmd->setRemotePort(remotePrt);
    openCmd->setOutboundStreams(outboundStreams);
    openCmd->setInboundStreams(inboundStreams);
    openCmd->setNumRequests(numRequests);
    openCmd->setPrMethod(prMethod);
    openCmd->setStreamReset(streamReset);
    msg->setControlInfo(openCmd);
    sendToSCTP(msg);
    if (oneToOne)
        sockstate = CONNECTING;
}

void SCTPSocket::connectx(AddressVector remoteAddressList, int32 remotePort, bool streamReset, int32 prMethod, uint32 numRequests)
{
    EV_INFO << "Socket connectx.  sockstate=" << sockstate << "\n";
    /*if (sockstate!=NOT_BOUND && sockstate!=CLOSED)
        throw cRuntimeError( "SCTPSocket::connect(): connect() or listen() already called");*/

    if (oneToOne && sockstate != NOT_BOUND && sockstate != CLOSED)
        throw cRuntimeError("SCTPSocket::connect(): connect() or listen() already called");

    if (!oneToOne && sockstate != LISTENING)
        throw cRuntimeError("SCTPSocket::connect: One-to-many style socket must be listening");

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
    openCmd->setInboundStreams(inboundStreams);
    openCmd->setNumRequests(numRequests);
    openCmd->setPrMethod(prMethod);
    openCmd->setStreamReset(streamReset);
    msg->setControlInfo(openCmd);
    sendToSCTP(msg);
    if (oneToOne)
        sockstate = CONNECTING;
}

void SCTPSocket::send(cPacket *msg, bool last, bool primary)
{
    if (oneToOne && sockstate != CONNECTED && sockstate != CONNECTING && sockstate != PEER_CLOSED) {
        throw cRuntimeError("SCTPSocket::send(): not connected or connecting");
    }
    else if (!oneToOne && sockstate != LISTENING) {
        throw cRuntimeError("SCTPSocket::send: One-to-many style socket must be listening");
    }

    SCTPSendCommand *cmd = new SCTPSendCommand();
    cmd->setAssocId(assocId);
    if (msg->getKind() == SCTP_C_SEND_ORDERED)
        cmd->setSendUnordered(COMPLETE_MESG_ORDERED);
    else
        cmd->setSendUnordered(COMPLETE_MESG_UNORDERED);
    lastStream = (lastStream + 1) % outboundStreams;
    cmd->setSid(lastStream);
    cmd->setLast(last);
    cmd->setPrimary(primary);
    msg->setKind(SCTP_C_SEND);
    msg->setControlInfo(cmd);
    sendToSCTP(msg);
}

void SCTPSocket::send(cPacket *msg, int32 prMethod, double prValue, bool last)
{
    send(msg, prMethod, prValue, last, -1);
}

void SCTPSocket::send(cPacket *msg, int32 prMethod, double prValue, bool last, int32 streamId)
{
    if (oneToOne && sockstate != CONNECTED && sockstate != CONNECTING && sockstate != PEER_CLOSED) {
        throw cRuntimeError("SCTPSocket::send(): not connected or connecting");
    }
    else if (!oneToOne && sockstate != LISTENING) {
        throw cRuntimeError("SCTPSocket::send: One-to-many style socket must be listening");
    }

    SCTPSendCommand *cmd = new SCTPSendCommand();
    cmd->setAssocId(assocId);
    if (msg->getKind() == SCTP_C_SEND_ORDERED)
        cmd->setSendUnordered(COMPLETE_MESG_ORDERED);
    else
        cmd->setSendUnordered(COMPLETE_MESG_UNORDERED);
    if (streamId >= 0) {
        cmd->setSid(streamId);
    }
    else {
        lastStream = (lastStream + 1) % outboundStreams;
        cmd->setSid(lastStream);
    }
    cmd->setPrValue(prValue);
    cmd->setPrMethod(prMethod);
    cmd->setLast(last);
    msg->setKind(SCTP_C_SEND);
    msg->setControlInfo(cmd);
    sendToSCTP(msg);
}

void SCTPSocket::sendNotification(cPacket *msg)
{
    if (oneToOne && sockstate != CONNECTED && sockstate != CONNECTING && sockstate != PEER_CLOSED) {
        throw cRuntimeError("SCTPSocket::sendNotification(%s): not connected or connecting", msg->getName());
    }
    else if (!oneToOne && sockstate != LISTENING) {
        throw cRuntimeError("SCTPSocket::send: One-to-many style socket must be listening");
    }

    sendToSCTP(msg);
}

void SCTPSocket::sendRequest(cPacket *msg)
{
    sendToSCTP(msg);
}

void SCTPSocket::close()
{
    EV_INFO << "SCTPSocket: close\n";

    cPacket *msg = new cPacket("CLOSE", SCTP_C_CLOSE);
    SCTPCommand *cmd = new SCTPCommand();
    cmd->setAssocId(assocId);
    msg->setControlInfo(cmd);
    sendToSCTP(msg);
    sockstate = sockstate == CONNECTED ? LOCALLY_CLOSED : CLOSED;
}

void SCTPSocket::shutdown()
{
    EV << "SCTPSocket: shutdown\n";

    cPacket *msg = new cPacket("Shutdown", SCTP_C_SHUTDOWN);
    SCTPCommand *cmd = new SCTPCommand();
    cmd->setAssocId(assocId);
    msg->setControlInfo(cmd);
    sendToSCTP(msg);
}

void SCTPSocket::abort()
{
    if (sockstate != NOT_BOUND && sockstate != CLOSED && sockstate != SOCKERROR) {
        cPacket *msg = new cPacket("ABORT", SCTP_C_ABORT);
        SCTPCommand *cmd = new SCTPCommand();
        //sctpEV3<<"Message cmd="<<&cmd<<"\n";
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
    bool ret = dynamic_cast<SCTPCommand *>(msg->getControlInfo()) &&
        ((SCTPCommand *)(msg->getControlInfo()))->getAssocId() == assocId;
    EV_INFO << "assoc=" << ((SCTPCommand *)(msg->getControlInfo()))->getAssocId() << "\n";
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
    switch (msg->getKind()) {
        case SCTP_I_DATA:
            EV_INFO << "SCTP_I_DATA\n";
            if (cb)
                cb->socketDataArrived(assocId, yourPtr, msg, false);
            break;

        case SCTP_I_DATA_NOTIFICATION:
            EV_INFO << "SCTP_I_NOTIFICATION\n";
            if (cb)
                cb->socketDataNotificationArrived(assocId, yourPtr, msg);
            break;

        case SCTP_I_SEND_MSG:
            if (cb)
                cb->sendRequestArrived();
            break;

        case SCTP_I_ESTABLISHED: {
            if (oneToOne)
                sockstate = CONNECTED;
            SCTPConnectInfo *connectInfo = check_and_cast<SCTPConnectInfo *>(msg->removeControlInfo());
            localAddr = connectInfo->getLocalAddr();
            remoteAddr = connectInfo->getRemoteAddr();
            localPrt = connectInfo->getLocalPort();
            remotePrt = connectInfo->getRemotePort();
            ;
            fsmStatus = connectInfo->getStatus();
            inboundStreams = connectInfo->getInboundStreams();
            outboundStreams = connectInfo->getOutboundStreams();

            if (cb)
                cb->socketEstablished(assocId, yourPtr, connectInfo->getNumMsgs());
            delete connectInfo;
            break;
        }

        case SCTP_I_PEER_CLOSED:
            EV_INFO << "peer closed\n";
            if (oneToOne)
                sockstate = sockstate == CONNECTED ? PEER_CLOSED : CLOSED;

            if (cb)
                cb->socketPeerClosed(assocId, yourPtr);
            break;

        case SCTP_I_ABORT:
        case SCTP_I_CONN_LOST:
        case SCTP_I_CLOSED:
            EV_INFO << "SCTP_I_CLOSED called\n";
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

        case SCTP_I_ABANDONED:
            if (cb)
                cb->msgAbandonedArrived(assocId);
            break;

        case SCTP_I_SHUTDOWN_RECEIVED:
            EV_INFO << "SCTP_I_SHUTDOWN_RECEIVED\n";
            if (cb)
                cb->shutdownReceivedArrived(assocId);
            break;

        case SCTP_I_SENDQUEUE_FULL:
            if (cb)
                cb->sendqueueFullArrived(assocId);
            break;

        case SCTP_I_SENDQUEUE_ABATED: {
            SCTPCommand *cmd = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
            if (cb) {
                cb->sendqueueAbatedArrived(assocId, cmd->getNumMsgs());
            }
            delete cmd;
            break;
        }

        case SCTP_I_RCV_STREAMS_RESETTED:
        case SCTP_I_SEND_STREAMS_RESETTED:
        case SCTP_I_RESET_REQUEST_FAILED:
            break;

        case SCTP_I_ADDRESS_ADDED: {
            SCTPCommand *cmd = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
            if (cb)
                cb->addressAddedArrived(assocId, cmd->getLocalAddr(), remoteAddr);
            delete cmd;
            break;
        }

        default:
            throw cRuntimeError("SCTPSocket: invalid msg kind %d, one of the SCTP_I_xxx constants expected", msg->getKind());
    }

    delete msg;
}

void SCTPSocket::setStreamPriority(uint32 stream, uint32 priority)
{
    cPacket *msg = new cPacket("SET_STREAM_PRIO", SCTP_C_SET_STREAM_PRIO);
    SCTPSendCommand *cmd = new SCTPSendCommand();
    cmd->setAssocId(assocId);
    cmd->setSid(stream);
    cmd->setPpid(priority);
    msg->setControlInfo(cmd);
    sendToSCTP(msg);
}

} // namespace inet

