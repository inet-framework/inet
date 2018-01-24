//
// Copyright (C) 2008 - 2018 Irene Ruengeler
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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"

namespace inet {

int32 SctpSocket::nextAssocId = 0;

SctpSocket::SctpSocket(bool type)
{
    sockstate = NOT_BOUND;
    localPrt = remotePrt = 0;
    fsmStatus = -1;
    cb = nullptr;
    yourPtr = nullptr;
    gateToSctp = nullptr;
    lastStream = -1;
    oneToOne = type;
    sOptions = new SocketOptions();
    appOptions = new AppSocketOptions();
    appOptions->inboundStreams = -1;
    appOptions->outboundStreams = -1;
    appOptions->streamReset = 0;
    appLimited = false;
   // if (oneToOne)
        assocId = getNewAssocId();
  /*  else
        assocId = 0;*/
    EV_INFO << "sockstate=" << stateName(sockstate) << "  assocId=" << assocId << "\n";
}

SctpSocket::SctpSocket(cMessage *msg)
{
    SctpCommand *ind = dynamic_cast<SctpCommand *>(msg->getControlInfo());

    if (!ind)
        throw cRuntimeError("SctpSocket::SctpSocket(cMessage *): no SctpCommand control info in message (not from SCTP?)");

    assocId = ind->getSocketId();
    sockstate = CONNECTED;

    localPrt = remotePrt = -1;
    sOptions = new SocketOptions();
    appOptions = new AppSocketOptions();
    appOptions->inboundStreams = -1;
    appOptions->outboundStreams = -1;
    appLimited = false;
    cb = nullptr;
    yourPtr = nullptr;
    gateToSctp = nullptr;
    lastStream = -1;
    oneToOne = true;

    if (msg->getKind() == SCTP_I_ESTABLISHED) {
        // management of stockstate is left to processMessage() so we always
        // set it to CONNECTED in the ctor, whatever SCTP_I_xxx arrives.
        // However, for convenience we extract SctpConnectInfo already here, so that
        // remote address/port can be read already after the ctor call.

        SctpConnectInfo *connectInfo = check_and_cast<SctpConnectInfo *>(msg->getControlInfo());
        localAddr = connectInfo->getLocalAddr();
        remoteAddr = connectInfo->getRemoteAddr();
        localPrt = connectInfo->getLocalPort();
        remotePrt = connectInfo->getRemotePort();
        fsmStatus = connectInfo->getStatus();
        appOptions->inboundStreams = connectInfo->getInboundStreams();
        appOptions->outboundStreams = connectInfo->getOutboundStreams();
   }
}

SctpSocket::~SctpSocket()
{
    delete sOptions;
    delete appOptions;
    if (cb) {
        cb->socketDeleted(assocId, yourPtr);
    }
}

const char *SctpSocket::stateName(int state)
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

void SctpSocket::sendToSctp(cMessage *msg)
{
    if (!gateToSctp)
        throw cRuntimeError("SctpSocket::sendToSctp(): setOutputGate() must be invoked before socket can be used");
    EV_INFO << "sendToSctp SocketId is set to " << assocId << endl;
    msg->ensureTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
    msg->ensureTag<SocketReq>()->setSocketId(assocId == -1 ? this->assocId : assocId);
    check_and_cast<cSimpleModule *>(gateToSctp->getOwnerModule())->send(msg, gateToSctp);
}

void SctpSocket::getSocketOptions()
{
EV_INFO << "getSocketOptions\n";
    cMessage* cmsg = new cMessage("GetSocketOptions", SCTP_C_GETSOCKETOPTIONS);
    SctpSendInfo *cmd = new SctpSendInfo("getOptions");
    cmd->setSocketId(assocId);
    cmd->setSid(0);
    cmsg->setControlInfo(cmd);
    sendToSctp(cmsg);
  EV_INFO << "getSocketOptions sent\n";
}

void SctpSocket::bind(int lPort)
{
    if (sockstate != NOT_BOUND)
        throw cRuntimeError("SctpSocket::bind(): socket already bound");

    localAddresses.push_back(L3Address());    // Unspecified address
    localPrt = lPort;
    sockstate = CLOSED;
    EV_INFO << "bind: vor getSocketOptions\n";
    getSocketOptions();
    EV_INFO << "bind: nach getSocketOptions\n";
}

void SctpSocket::bind(L3Address lAddr, int lPort)
{
    EV_INFO << "bind address " << lAddr << "\n";
    if (sockstate != NOT_BOUND)
        throw cRuntimeError("SctpSocket::bind(): socket already bound");

    localAddresses.push_back(lAddr);
    localPrt = lPort;
    sockstate = CLOSED;
    getSocketOptions();
}

void SctpSocket::addAddress(L3Address addr)
{
    EV_INFO << "add address " << addr << "\n";
    localAddresses.push_back(addr);
}

void SctpSocket::bindx(AddressVector lAddresses, int lPort)
{
    L3Address lAddr;
    for (auto & lAddresse : lAddresses) {
        EV << "bindx: bind address " << (lAddresse) << "\n";
        localAddresses.push_back((lAddresse));
    }
    localPrt = lPort;
    sockstate = CLOSED;
    getSocketOptions();
}

void SctpSocket::listen(bool fork, bool reset, uint32 requests, uint32 messagesToPush)
{
    if (sockstate != CLOSED)
        throw cRuntimeError(sockstate == NOT_BOUND ?
                "SctpSocket::listen(): must call bind() before listen()" :
                "SctpSocket::listen(): connect() or listen() already called");

    SctpOpenCommand *openCmd = new SctpOpenCommand();
    openCmd->setLocalAddresses(localAddresses);
    openCmd->setLocalPort(localPrt);
    if (oneToOne)
        openCmd->setSocketId(assocId);
    else
        openCmd->setSocketId(getNewAssocId());
    openCmd->setFork(fork);
    openCmd->setOutboundStreams(appOptions->outboundStreams);
    openCmd->setInboundStreams(appOptions->inboundStreams);
    appOptions->streamReset = reset;
    openCmd->setNumRequests(requests);
    openCmd->setStreamReset(reset);
    openCmd->setMessagesToPush(messagesToPush);

    EV_INFO << "Assoc " << openCmd->getSocketId() << ": PassiveOPEN to SCTP from SctpSocket:listen()\n";
    cMessage *cmsg = new cMessage("PassiveOPEN", SCTP_C_OPEN_PASSIVE);
    cmsg->setControlInfo(openCmd);
    sendToSctp(cmsg);
    sockstate = LISTENING;
}

void SctpSocket::listen(uint32 requests, bool fork, uint32 messagesToPush, bool options, int32 fd)
{
    if (sockstate != CLOSED)
        throw cRuntimeError(sockstate == NOT_BOUND ?
                "SctpSocket::listen(): must call bind() before listen()" :
                "SctpSocket::listen(): connect() or listen() already called");

    SctpOpenCommand *openCmd = new SctpOpenCommand();
    openCmd->setLocalAddresses(localAddresses);
    openCmd->setLocalPort(localPrt);
    if (oneToOne)
        openCmd->setSocketId(assocId);
    else
        openCmd->setSocketId(getNewAssocId());
    openCmd->setFork(fork);
    openCmd->setFd(fd);
    openCmd->setInboundStreams(appOptions->inboundStreams);
    openCmd->setOutboundStreams(appOptions->outboundStreams);
    openCmd->setNumRequests(requests);
    openCmd->setMessagesToPush(messagesToPush);
    openCmd->setStreamReset(appOptions->streamReset);

    EV_INFO << "Assoc " << openCmd->getSocketId() << ": PassiveOPEN to SCTP from SctpSocket:listen()\n";
    cMessage *cmsg = new cMessage("PassiveOPEN", SCTP_C_OPEN_PASSIVE);
    cmsg->setControlInfo(openCmd);
    if (options)
        cmsg->setContextPointer((void*) sOptions);
    sendToSctp(cmsg);
    sockstate = LISTENING;
}

void SctpSocket::connect(L3Address remoteAddress, int32 remotePort, bool streamReset, int32 prMethod, uint32 numRequests)
{
    EV_INFO << "Socket connect. Assoc=" << assocId << ", sockstate=" << stateName(sockstate) << "\n";

    if (oneToOne && sockstate == NOT_BOUND) {
   EV_INFO << "Connect: call bind first\n";
       bind(0);
    }

    if (oneToOne && sockstate != CLOSED)
        throw cRuntimeError("SctpSocket::connect(): connect() or listen() already called");

    if (!oneToOne && sockstate != LISTENING)
        throw cRuntimeError("SctpSocket::connect(): one-to-many style socket must be listening");

    remoteAddr = remoteAddress;
    remotePrt = remotePort;

    SctpOpenCommand *openCmd = new SctpOpenCommand();
    if (oneToOne)
        openCmd->setSocketId(assocId);
    else
        openCmd->setSocketId(getNewAssocId());
    EV_INFO << "Socket connect. Assoc=" << openCmd->getSocketId() << ", sockstate=" << stateName(sockstate) << "\n";
    openCmd->setLocalAddresses(localAddresses);
    openCmd->setLocalPort(localPrt);
    openCmd->setRemoteAddr(remoteAddr);
    openCmd->setRemotePort(remotePrt);
    openCmd->setOutboundStreams(appOptions->outboundStreams);
    openCmd->setInboundStreams(appOptions->inboundStreams);
    appOptions->streamReset = streamReset;
    openCmd->setNumRequests(numRequests);
    openCmd->setPrMethod(prMethod);
    openCmd->setStreamReset(streamReset);

    cMessage *cmsg = new cMessage("Associate", SCTP_C_ASSOCIATE);
    cmsg->setControlInfo(openCmd);
    sendToSctp(cmsg);

    if (oneToOne)
        sockstate = CONNECTING;
}

void SctpSocket::connect(int32 fd, L3Address remoteAddress, int32 remotePort, uint32 numRequests, bool options)
{
    EV_INFO << "Socket connect. Assoc=" << assocId << ", sockstate=" << stateName(sockstate) << "\n";

    if (oneToOne && sockstate == NOT_BOUND)
       bind(0);

    if (oneToOne && sockstate != CLOSED)
        throw cRuntimeError("SctpSocket::connect(): connect() or listen() already called");

    if (!oneToOne && sockstate != LISTENING)
        throw cRuntimeError("SctpSocket::connect(): one-to-many style socket must be listening");

    remoteAddr = remoteAddress;
    remotePrt = remotePort;

    SctpOpenCommand *openCmd = new SctpOpenCommand();
    if (oneToOne)
        openCmd->setSocketId(assocId);
    else
        openCmd->setSocketId(getNewAssocId());
    EV_INFO << "Socket connect. Assoc=" << openCmd->getSocketId() << ", sockstate=" << stateName(sockstate) << "\n";
    openCmd->setLocalAddresses(localAddresses);
    openCmd->setLocalPort(localPrt);
    openCmd->setRemoteAddr(remoteAddr);
    openCmd->setRemotePort(remotePrt);
    openCmd->setOutboundStreams(appOptions->outboundStreams);
    openCmd->setInboundStreams(appOptions->inboundStreams);
    openCmd->setStreamReset(appOptions->streamReset);
    openCmd->setNumRequests(numRequests);
    openCmd->setFd(fd);
    openCmd->setAppLimited(appLimited);

    cMessage *cmsg = new cMessage("Associate", SCTP_C_ASSOCIATE);
    cmsg->setControlInfo(openCmd);
    if (options) {
        cmsg->setContextPointer((void*) sOptions);
    }
    sendToSctp(cmsg);

    if (oneToOne)
        sockstate = CONNECTING;
}

void SctpSocket::accept(int32 assId, int32 fd)
{
    SctpCommand *cmd = new SctpCommand();
    cmd->setLocalPort(localPrt);
    cmd->setRemoteAddr(remoteAddr);
    cmd->setRemotePort(remotePrt);
    cmd->setSocketId(assId);
    cmd->setFd(fd);
    cMessage *cmsg = new cMessage("Accept", SCTP_C_ACCEPT);
    cmsg->setControlInfo(cmd);
    sendToSctp(cmsg);
}

void SctpSocket::connectx(AddressVector remoteAddressList, int32 remotePort, bool streamReset, int32 prMethod, uint32 numRequests)
{
    EV_INFO << "Socket connectx.  sockstate=" << stateName(sockstate) << "\n";
    remoteAddresses = remoteAddressList;
    connect(remoteAddressList.front(), remotePort, streamReset, prMethod, numRequests);
}
#if 0
void SctpSocket::send(SctpSimpleMessage *msg, int32 prMethod, double prValue, int32 streamId, bool last, bool primary)
{
    if (oneToOne && sockstate != CONNECTED && sockstate != CONNECTING && sockstate != PEER_CLOSED) {
        throw cRuntimeError("SctpSocket::send(): not connected or connecting");
    }
    else if (!oneToOne && sockstate != LISTENING) {
        throw cRuntimeError("SctpSocket::send(): one-to-many style socket must be listening");
    }

#if 0   //FIXME KLUDGE
    SctpSendInfo *sendCommand = new SctpSendInfo();
    sendCommand->setSocketId(assocId);
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

    sendToSctp(cmsg);
#endif
}
#endif

void SctpSocket::sendMsg(cMessage *cmsg)
{
    SctpSendInfo *sendCommand;

    if (cmsg->getControlInfo()) {
        sendCommand = check_and_cast<SctpSendInfo *>(cmsg->removeControlInfo());
        if (sendCommand->getSid() == -1) {
            lastStream = (lastStream + 1) % appOptions->outboundStreams;
            sendCommand->setSid(lastStream);
        }
        if (sendCommand->getSocketId() == -1)
            sendCommand->setSocketId(assocId);
        cmsg->setControlInfo(sendCommand);
    } else {
        sendCommand = new SctpSendInfo();
        sendCommand->setSocketId(assocId);
        lastStream = (lastStream + 1) % appOptions->outboundStreams;
        sendCommand->setSid(lastStream);
        cmsg->setControlInfo(sendCommand);
    }
    cmsg->setKind(SCTP_C_SEND);
    sendToSctp(cmsg);
}

void SctpSocket::sendNotification(cMessage *msg)
{
    if (oneToOne && sockstate != CONNECTED && sockstate != CONNECTING && sockstate != PEER_CLOSED) {
        throw cRuntimeError("SctpSocket::sendNotification(%s): not connected or connecting", msg->getName());
    }
    else if (!oneToOne && sockstate != LISTENING) {
        throw cRuntimeError("SctpSocket::sendNotification(%s): one-to-many style socket must be listening", msg->getName());
    }

    sendToSctp(msg);
}

void SctpSocket::sendRequest(cMessage *msg)
{
    sendToSctp(msg);
}

void SctpSocket::close(int id)
{
    EV_INFO << "SctpSocket::close()\n";

    cMessage *msg = new cMessage("CLOSE", SCTP_C_CLOSE);
    SctpCommand *cmd = new SctpCommand();
    if (id == -1)
        cmd->setSocketId(assocId);
    else
        cmd->setFd(id);
    msg->setControlInfo(cmd);
    sendToSctp(msg);
    sockstate = (sockstate == CONNECTED) ? LOCALLY_CLOSED : CLOSED;
}

void SctpSocket::shutdown(int id)
{
    EV << "SctpSocket::shutdown()\n";

    cMessage *msg = new cMessage("SHUTDOWN", SCTP_C_SHUTDOWN);
    SctpCommand *cmd = new SctpCommand();
    if (id == -1)
        cmd->setSocketId(assocId);
    else
        cmd->setFd(id);
    msg->setControlInfo(cmd);
    sendToSctp(msg);
}

void SctpSocket::abort()
{
    if (sockstate != NOT_BOUND && sockstate != CLOSED && sockstate != SOCKERROR) {
        cMessage *msg = new cMessage("ABORT", SCTP_C_ABORT);
        SctpCommand *cmd = new SctpCommand();
        cmd->setSocketId(assocId);
        msg->setControlInfo(cmd);
        sendToSctp(msg);
    }
    sockstate = CLOSED;
}

void SctpSocket::requestStatus()
{
    cMessage *msg = new cMessage("STATUS", SCTP_C_STATUS);
    SctpCommand *cmd = new SctpCommand();
    cmd->setSocketId(assocId);
    msg->setControlInfo(cmd);
    sendToSctp(msg);
}

bool SctpSocket::belongsToSocket(cMessage *msg)
{
    bool ret = dynamic_cast<SctpCommand *>(msg->getControlInfo()) &&
        ((SctpCommand *)(msg->getControlInfo()))->getSocketId() == assocId;
    EV_INFO << "assoc=" << ((SctpCommand *)(msg->getControlInfo()))->getSocketId() << "\n";
    return ret;
}

bool SctpSocket::belongsToAnySctpSocket(cMessage *msg)
{
    return dynamic_cast<SctpCommand *>(msg->getControlInfo());
}

void SctpSocket::setCallbackObject(CallbackInterface *callback, void *yourPointer)
{
    cb = callback;
    yourPtr = yourPointer;
}

void SctpSocket::processMessage(cMessage *msg)
{
    SctpStatusInfo *status;
    switch (msg->getKind()) {
        case SCTP_I_DATA:
            EV_INFO << "SCTP_I_DATA\n";
            if (cb) {
                cb->socketDataArrived(assocId, yourPtr, check_and_cast<Packet *>(msg), false);
                msg = nullptr;
            }
            break;

        case SCTP_I_DATA_NOTIFICATION:
            EV_INFO << "SCTP_I_NOTIFICATION\n";
            if (cb) {
                cb->socketDataNotificationArrived(assocId, yourPtr, check_and_cast<Packet *>(msg));
            }
            break;

        case SCTP_I_SEND_MSG:
            if (cb) {
                cb->sendRequestArrived();
            }
            break;

        case SCTP_I_ESTABLISHED: {
        EV_INFO << "SCTP_I_ESTABLISHED\n";
            if (oneToOne)
                sockstate = CONNECTED;
            SctpConnectInfo *connectInfo = check_and_cast<SctpConnectInfo *>(msg->removeControlInfo());
            localAddr = connectInfo->getLocalAddr();
            remoteAddr = connectInfo->getRemoteAddr();
            localPrt = connectInfo->getLocalPort();
            remotePrt = connectInfo->getRemotePort();
            fsmStatus = connectInfo->getStatus();
            appOptions->inboundStreams = connectInfo->getInboundStreams();
            appOptions->outboundStreams = connectInfo->getOutboundStreams();
            assocId = msg->getTag<SocketInd>()->getSocketId();
           // assocId = msg->getMandatoryTag<SocketInd>()->getSocketId();

            if (cb) {
            EV_INFO << "call cb->socketEstablished\n";
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
            status = check_and_cast<SctpStatusInfo *>(msg->removeControlInfo());
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
            SctpCommand *cmd = check_and_cast<SctpCommand *>(msg->removeControlInfo());
            if (cb) {
                cb->sendqueueAbatedArrived(assocId, cmd->getNumMsgs());
            }
            delete cmd;
            break;
        }

        case SCTP_I_RCV_STREAMS_RESETTED:
        case SCTP_I_SEND_STREAMS_RESETTED:
        case SCTP_I_RESET_REQUEST_FAILED:
        case SCTP_I_SENDSOCKETOPTIONS:
            break;

        case SCTP_I_ADDRESS_ADDED: {
            SctpCommand *cmd = check_and_cast<SctpCommand *>(msg->removeControlInfo());
            if (cb) {
                cb->addressAddedArrived(assocId, cmd->getLocalAddr(), remoteAddr);
            }
            delete cmd;
            break;
        }

        default:
            throw cRuntimeError("SctpSocket::processMessage(): invalid msg kind %d, one of the SCTP_I_xxx constants expected", msg->getKind());
    }

    if (msg != nullptr) {
        delete msg;
    }
}

void SctpSocket::setStreamPriority(uint32 stream, uint32 priority)
{
    cMessage *msg = new cMessage("SET_STREAM_PRIO", SCTP_C_SET_STREAM_PRIO);
    SctpSendInfo *cmd = new SctpSendInfo();
    cmd->setSocketId(assocId);
    cmd->setSid(stream);
    cmd->setPpid(priority);
    msg->setControlInfo(cmd);
    sendToSctp(msg);
}

void SctpSocket::setRtoInfo(double initial, double max, double min)
{
    sOptions->rtoInitial = initial;
    sOptions->rtoMax = max;
    sOptions->rtoMin = min;
    if (sockstate == CONNECTED) {
        cMessage *msg = new cMessage("RtoInfo", SCTP_C_SET_RTO_INFO);
        SctpRtoInfo *cmd = new SctpRtoInfo();
        cmd->setSocketId(assocId);
        cmd->setRtoInitial(initial);
        cmd->setRtoMin(min);
        cmd->setRtoMax(max);
        msg->setControlInfo(cmd);
        sendToSctp(msg);
    }
}

} // namespace inet
