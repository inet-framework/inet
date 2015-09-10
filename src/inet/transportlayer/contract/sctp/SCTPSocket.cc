//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2015 Thomas Dreibholz
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

namespace inet {

int32 SCTPSocket::nextAssocId = 0;

SCTPSocket::SCTPSocket(bool type)
{
    sockstate = NOT_BOUND;
    localPrt = remotePrt = 0;
    inboundStreams = outboundStreams = 1;
    cb = nullptr;
    yourPtr = nullptr;
    gateToSctp = nullptr;
    lastStream = -1;
    oneToOne = type;
    if (oneToOne)
        assocId = getNewAssocId();
    else
        assocId = 0;
    EV_INFO << "sockstate=" << stateName(sockstate) << "\n";
}

SCTPSocket::SCTPSocket(cMessage *msg)
{
    SCTPCommand *ind = dynamic_cast<SCTPCommand *>(msg->getControlInfo());

    if (!ind)
        throw cRuntimeError("SCTPSocket::SCTPSocket(cMessage *): no SCTPCommand control info in message (not from SCTP?)");

    assocId = ind->getAssocId();
    sockstate = CONNECTED;

    localPrt = remotePrt = -1;
    inboundStreams = outboundStreams = 0;
    cb = nullptr;
    yourPtr = nullptr;
    gateToSctp = nullptr;
    lastStream = -1;
    oneToOne = true;

    if (msg->getKind() == SCTP_I_ESTABLISHED) {
        // management of stockstate is left to processMessage() so we always
        // set it to CONNECTED in the ctor, whatever SCTP_I_xxx arrives.
        // However, for convenience we extract SCTPConnectInfo already here, so that
        // remote address/port can be read already after the ctor call.

        SCTPConnectInfo *connectInfo = check_and_cast<SCTPConnectInfo *>(msg->getControlInfo());
        localAddr = connectInfo->getLocalAddr();
        remoteAddr = connectInfo->getRemoteAddr();
        localPrt = connectInfo->getLocalPort();
        remotePrt = connectInfo->getRemotePort();
        fsmStatus = connectInfo->getStatus();
        inboundStreams = connectInfo->getInboundStreams();
        outboundStreams = connectInfo->getOutboundStreams();
   }
}

SCTPSocket::~SCTPSocket()
{
    if (cb) {
        cb->socketDeleted(assocId, yourPtr);
    }
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

void SCTPSocket::sendToSCTP(cMessage *msg)
{
    if (!gateToSctp)
        throw cRuntimeError("SCTPSocket::sendToSCTP(): setOutputGate() must be invoked before socket can be used");

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
    for (auto i = lAddresses.begin(); i != lAddresses.end(); ++i) {
        EV << "bindx: bind address " << (*i) << "\n";
        localAddresses.push_back((*i));
    }
    localPrt = lPort;
    sockstate = CLOSED;
}

void SCTPSocket::listen(bool fork, bool reset, uint32 requests, uint32 messagesToPush)
{
    if (sockstate != CLOSED)
        throw cRuntimeError(sockstate == NOT_BOUND ?
                "SCTPSocket::listen(): must call bind() before listen()" :
                "SCTPSocket::listen(): connect() or listen() already called");

    SCTPOpenCommand *openCmd = new SCTPOpenCommand();
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

    EV_INFO << "Assoc " << openCmd->getAssocId() << ": PassiveOPEN to SCTP from SCTPSocket:listen()\n";
    cMessage *cmsg = new cMessage("PassiveOPEN", SCTP_C_OPEN_PASSIVE);
    cmsg->setControlInfo(openCmd);
    sendToSCTP(cmsg);
    sockstate = LISTENING;
}

void SCTPSocket::connect(L3Address remoteAddress, int32 remotePort, bool streamReset, int32 prMethod, uint32 numRequests)
{
    EV_INFO << "Socket connect. Assoc=" << assocId << ", sockstate=" << stateName(sockstate) << "\n";

    if (oneToOne && sockstate == NOT_BOUND)
       bind(0);

    if (oneToOne && sockstate != CLOSED)
        throw cRuntimeError("SCTPSocket::connect(): connect() or listen() already called");

    if (!oneToOne && sockstate != LISTENING)
        throw cRuntimeError("SCTPSocket::connect(): one-to-many style socket must be listening");

    remoteAddr = remoteAddress;
    remotePrt = remotePort;

    SCTPOpenCommand *openCmd = new SCTPOpenCommand();
    if (oneToOne)
        openCmd->setAssocId(assocId);
    else
        openCmd->setAssocId(getNewAssocId());
    EV_INFO << "Socket connect. Assoc=" << openCmd->getAssocId() << ", sockstate=" << stateName(sockstate) << "\n";
    openCmd->setLocalAddresses(localAddresses);
    openCmd->setLocalPort(localPrt);
    openCmd->setRemoteAddr(remoteAddr);
    openCmd->setRemotePort(remotePrt);
    openCmd->setOutboundStreams(outboundStreams);
    openCmd->setInboundStreams(inboundStreams);
    openCmd->setNumRequests(numRequests);
    openCmd->setPrMethod(prMethod);
    openCmd->setStreamReset(streamReset);

    cMessage *cmsg = new cMessage("Associate", SCTP_C_ASSOCIATE);
    cmsg->setControlInfo(openCmd);
    sendToSCTP(cmsg);

    if (oneToOne)
        sockstate = CONNECTING;
}

void SCTPSocket::connectx(AddressVector remoteAddressList, int32 remotePort, bool streamReset, int32 prMethod, uint32 numRequests)
{
    EV_INFO << "Socket connectx.  sockstate=" << stateName(sockstate) << "\n";
    remoteAddresses = remoteAddressList;
    connect(remoteAddressList.front(), remotePort, streamReset, prMethod, numRequests);
}

void SCTPSocket::send(SCTPSimpleMessage *msg, int32 prMethod, double prValue, int32 streamId, bool last, bool primary)
{
    if (oneToOne && sockstate != CONNECTED && sockstate != CONNECTING && sockstate != PEER_CLOSED) {
        throw cRuntimeError("SCTPSocket::send(): not connected or connecting");
    }
    else if (!oneToOne && sockstate != LISTENING) {
        throw cRuntimeError("SCTPSocket::send(): one-to-many style socket must be listening");
    }

    SCTPSendInfo *sendCommand = new SCTPSendInfo();
    sendCommand->setAssocId(assocId);
    sendCommand->setSid(streamId);
    sendCommand->setPrValue(prValue);
    sendCommand->setPrMethod(prMethod);
    sendCommand->setLast(last);
    sendCommand->setPrimary(primary);
    sendCommand->setSendUnordered( (msg->getKind() == SCTP_C_SEND_UNORDERED) ?
                                   COMPLETE_MESG_UNORDERED : COMPLETE_MESG_ORDERED );

    cPacket* cmsg = new cPacket("SCTP_C_SEND");
    cmsg->setKind(SCTP_C_SEND);
    cmsg->encapsulate(msg);
    cmsg->setControlInfo(sendCommand);

    sendToSCTP(cmsg);
}

void SCTPSocket::sendMsg(cMessage *cmsg)
{
    SCTPSendInfo *sendCommand;

    if (cmsg->getControlInfo()) {
        sendCommand = check_and_cast<SCTPSendInfo *>(cmsg->removeControlInfo());
        if (sendCommand->getSid() == -1) {
            lastStream = (lastStream + 1) % outboundStreams;
            sendCommand->setSid(lastStream);
        }
        sendCommand->setAssocId(assocId);
        cmsg->setControlInfo(sendCommand);
    } else {
        sendCommand = new SCTPSendInfo();
        sendCommand->setAssocId(assocId);
        lastStream = (lastStream + 1) % outboundStreams;
        sendCommand->setSid(lastStream);
        cmsg->setControlInfo(sendCommand);
    }
    cmsg->setKind(SCTP_C_SEND);
    sendToSCTP(cmsg);
}

void SCTPSocket::sendNotification(cMessage *msg)
{
    if (oneToOne && sockstate != CONNECTED && sockstate != CONNECTING && sockstate != PEER_CLOSED) {
        throw cRuntimeError("SCTPSocket::sendNotification(%s): not connected or connecting", msg->getName());
    }
    else if (!oneToOne && sockstate != LISTENING) {
        throw cRuntimeError("SCTPSocket::sendNotification(%s): one-to-many style socket must be listening", msg->getName());
    }

    sendToSCTP(msg);
}

void SCTPSocket::sendRequest(cMessage *msg)
{
    sendToSCTP(msg);
}

void SCTPSocket::close()
{
    EV_INFO << "SCTPSocket::close()\n";

    cMessage *msg = new cMessage("CLOSE", SCTP_C_CLOSE);
    SCTPCommand *cmd = new SCTPCommand();
    cmd->setAssocId(assocId);
    msg->setControlInfo(cmd);
    sendToSCTP(msg);
    sockstate = (sockstate == CONNECTED) ? LOCALLY_CLOSED : CLOSED;
}

void SCTPSocket::shutdown()
{
    EV << "SCTPSocket::shutdown()\n";

    cMessage *msg = new cMessage("SHUTDOWN", SCTP_C_SHUTDOWN);
    SCTPCommand *cmd = new SCTPCommand();
    cmd->setAssocId(assocId);
    msg->setControlInfo(cmd);
    sendToSCTP(msg);
}

void SCTPSocket::abort()
{
    if (sockstate != NOT_BOUND && sockstate != CLOSED && sockstate != SOCKERROR) {
        cMessage *msg = new cMessage("ABORT", SCTP_C_ABORT);
        SCTPCommand *cmd = new SCTPCommand();
        cmd->setAssocId(assocId);
        msg->setControlInfo(cmd);
        sendToSCTP(msg);
    }
    sockstate = CLOSED;
}

void SCTPSocket::requestStatus()
{
    cMessage *msg = new cMessage("STATUS", SCTP_C_STATUS);
    SCTPCommand *cmd = new SCTPCommand();
    cmd->setAssocId(assocId);
    msg->setControlInfo(cmd);
    sendToSCTP(msg);
}

bool SCTPSocket::belongsToSocket(cMessage *msg)
{
    bool ret = dynamic_cast<SCTPCommand *>(msg->getControlInfo()) &&
        ((SCTPCommand *)(msg->getControlInfo()))->getAssocId() == assocId;
    EV_INFO << "assoc=" << ((SCTPCommand *)(msg->getControlInfo()))->getAssocId() << "\n";
    return ret;
}

bool SCTPSocket::belongsToAnySCTPSocket(cMessage *msg)
{
    return dynamic_cast<SCTPCommand *>(msg->getControlInfo());
}

void SCTPSocket::setCallbackObject(CallbackInterface *callback, void *yourPointer)
{
    cb = callback;
    yourPtr = yourPointer;
}

void SCTPSocket::processMessage(cMessage *msg)
{
    SCTPStatusInfo *status;
    switch (msg->getKind()) {
        case SCTP_I_DATA:
            EV_INFO << "SCTP_I_DATA\n";
            if (cb) {
                cb->socketDataArrived(assocId, yourPtr, PK(msg), false);
                msg = NULL;
            }
            break;

        case SCTP_I_DATA_NOTIFICATION:
            EV_INFO << "SCTP_I_NOTIFICATION\n";
            if (cb) {
                cb->socketDataNotificationArrived(assocId, yourPtr, PK(msg));
            }
            break;

        case SCTP_I_SEND_MSG:
            if (cb) {
                cb->sendRequestArrived();
            }
            break;

        case SCTP_I_ESTABLISHED: {
            if (oneToOne)
                sockstate = CONNECTED;
            SCTPConnectInfo *connectInfo = check_and_cast<SCTPConnectInfo *>(msg->removeControlInfo());
            localAddr = connectInfo->getLocalAddr();
            remoteAddr = connectInfo->getRemoteAddr();
            localPrt = connectInfo->getLocalPort();
            remotePrt = connectInfo->getRemotePort();
            fsmStatus = connectInfo->getStatus();
            inboundStreams = connectInfo->getInboundStreams();
            outboundStreams = connectInfo->getOutboundStreams();

            if (cb) {
                cb->socketEstablished(assocId, yourPtr, connectInfo->getNumMsgs());
            }
            delete connectInfo;
            break;
        }

        case SCTP_I_PEER_CLOSED:
            EV_INFO << "peer closed\n";
            if (oneToOne)
                sockstate = (sockstate == CONNECTED) ? PEER_CLOSED : CLOSED;

            if (cb) {
                cb->socketPeerClosed(assocId, yourPtr);
            }
            break;

        case SCTP_I_ABORT:
        case SCTP_I_CONN_LOST:
        case SCTP_I_CLOSED:
            EV_INFO << "SCTP_I_CLOSED called\n";
            sockstate = CLOSED;
            if (cb) {
                cb->socketClosed(assocId, yourPtr);
            }
            break;

        case SCTP_I_CONNECTION_REFUSED:
        case SCTP_I_CONNECTION_RESET:
        case SCTP_I_TIMED_OUT:
            sockstate = SOCKERROR;
            if (cb) {
                cb->socketFailure(assocId, yourPtr, msg->getKind());
            }
            break;

        case SCTP_I_STATUS:
            status = check_and_cast<SCTPStatusInfo *>(msg->removeControlInfo());
            if (cb) {
                cb->socketStatusArrived(assocId, yourPtr, status);
            }
            break;

        case SCTP_I_ABANDONED:
            if (cb) {
                cb->msgAbandonedArrived(assocId);
            }
            break;

        case SCTP_I_SHUTDOWN_RECEIVED:
            EV_INFO << "SCTP_I_SHUTDOWN_RECEIVED\n";
            if (cb) {
                cb->shutdownReceivedArrived(assocId);
            }
            break;

        case SCTP_I_SENDQUEUE_FULL:
            if (cb) {
                cb->sendqueueFullArrived(assocId);
            }
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
            if (cb) {
                cb->addressAddedArrived(assocId, cmd->getLocalAddr(), remoteAddr);
            }
            delete cmd;
            break;
        }

        default:
            throw cRuntimeError("SCTPSocket::processMessage(): invalid msg kind %d, one of the SCTP_I_xxx constants expected", msg->getKind());
    }

    if (msg != NULL) {
        delete msg;
    }
}

void SCTPSocket::setStreamPriority(uint32 stream, uint32 priority)
{
    cMessage *msg = new cMessage("SET_STREAM_PRIO", SCTP_C_SET_STREAM_PRIO);
    SCTPSendInfo *cmd = new SCTPSendInfo();
    cmd->setAssocId(assocId);
    cmd->setSid(stream);
    cmd->setPpid(priority);
    msg->setControlInfo(cmd);
    sendToSCTP(msg);
}

} // namespace inet
