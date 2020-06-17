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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/INETDefs.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"

namespace inet {

int32 SctpSocket::nextAssocId = 0;

SctpSocket::SctpSocket(bool type)
{
    sockstate = NOT_BOUND;
    localPrt = remotePrt = 0;
    fsmStatus = -1;
    cb = nullptr;
    userData = nullptr;
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
    auto& tags = getTags(msg);
    if (tags.getNumTags() == 0)
        throw cRuntimeError("SctpSocket::SctpSocket(cMessage *): no SctpCommandReq in message (not from SCTP?)");

    assocId = tags.getTag<SocketInd>()->getSocketId();
    sockstate = CONNECTED;

    localPrt = remotePrt = -1;
    sOptions = new SocketOptions();
    appOptions = new AppSocketOptions();
    appOptions->inboundStreams = -1;
    appOptions->outboundStreams = -1;
    appLimited = false;
    cb = nullptr;
    userData = nullptr;
    gateToSctp = nullptr;
    lastStream = -1;
    oneToOne = true;

    if (msg->getKind() == SCTP_I_AVAILABLE) {
        const auto& connectInfo = tags.findTag<SctpAvailableReq>();
        assocId = connectInfo->getNewSocketId();
        localAddr = connectInfo->getLocalAddr();
        remoteAddr = connectInfo->getRemoteAddr();
        localPrt = connectInfo->getLocalPort();
        remotePrt = connectInfo->getRemotePort();
    }
    else if (msg->getKind() == SCTP_I_ESTABLISHED) {
        // management of stockstate is left to processMessage() so we always
        // set it to CONNECTED in the ctor, whatever SCTP_I_xxx arrives.
        // However, for convenience we extract SctpConnectInfo already here, so that
        // remote address/port can be read already after the ctor call.

        const auto& connectInfo = tags.findTag<SctpConnectReq>();
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
        cb->socketDeleted(this);
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
    auto& tags = getTags(msg);
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
    tags.addTagIfAbsent<SocketReq>()->setSocketId(assocId == -1 ? this->assocId : assocId);
    if (interfaceIdToTun != -1) {
        tags.addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceIdToTun);
    }
    check_and_cast<cSimpleModule *>(gateToSctp->getOwnerModule())->send(msg, gateToSctp);
}

void SctpSocket::getSocketOptions()
{
    EV_INFO << "getSocketOptions\n";
    Request* cmsg = new Request("GetSocketOptions", SCTP_C_GETSOCKETOPTIONS);
    auto sctpSendReq = cmsg->addTag<SctpSendReq>();
    sctpSendReq->setSocketId(assocId);
    sctpSendReq->setSid(0);
    sendToSctp(cmsg);
}

void SctpSocket::bind(int lPort)
{
    if (sockstate != NOT_BOUND)
        throw cRuntimeError("SctpSocket::bind(): socket already bound");

    localAddresses.push_back(L3Address());    // Unspecified address
    localPrt = lPort;
    sockstate = CLOSED;
    getSocketOptions();
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

    Request *cmsg = new Request("PassiveOPEN", SCTP_C_OPEN_PASSIVE);
    auto openCmd = cmsg->addTag<SctpOpenReq>();
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
    sendToSctp(cmsg);
    sockstate = LISTENING;
}

void SctpSocket::listen(uint32 requests, bool fork, uint32 messagesToPush, bool options, int32 fd)
{
    if (sockstate != CLOSED)
        throw cRuntimeError(sockstate == NOT_BOUND ?
                "SctpSocket::listen(): must call bind() before listen()" :
                "SctpSocket::listen(): connect() or listen() already called");

    Request *cmsg = new Request("PassiveOPEN", SCTP_C_OPEN_PASSIVE);
    auto openCmd = cmsg->addTag<SctpOpenReq>();
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

    EV_INFO<< "Assoc " << openCmd->getSocketId() << ": PassiveOPEN to SCTP from SctpSocket:listen2()\n";
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

    Request *cmsg = new Request("Associate", SCTP_C_ASSOCIATE);
    auto openCmd = cmsg->addTag<SctpOpenReq>();

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

    Request *cmsg = new Request("Associate", SCTP_C_ASSOCIATE);
    auto openCmd = cmsg->addTag<SctpOpenReq>();
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

    if (options) {
        cmsg->setContextPointer((void*) sOptions);
    }
    sendToSctp(cmsg);

    if (oneToOne)
        sockstate = CONNECTING;
}

void SctpSocket::accept(int32 assId, int32 fd)
{
    Request *cmsg = new Request("Accept", SCTP_C_ACCEPT);
    auto cmd = cmsg->addTag<SctpCommandReq>();
    cmd->setLocalPort(localPrt);
    cmd->setRemoteAddr(remoteAddr);
    cmd->setRemotePort(remotePrt);
    cmd->setSocketId(assId);
    cmd->setFd(fd);
    sendToSctp(cmsg);
}

void SctpSocket::acceptSocket(int newSockId)
{
    Request *cmsg = new Request("SCTP_C_ACCEPT_SOCKET_ID", SCTP_C_ACCEPT_SOCKET_ID);
    cmsg->addTag<SctpAvailableReq>()->setSocketId(newSockId);
    cmsg->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
    cmsg->addTag<SocketReq>()->setSocketId(newSockId);
    check_and_cast<cSimpleModule *>(gateToSctp->getOwnerModule())->send(cmsg, gateToSctp);
}

void SctpSocket::connectx(AddressVector remoteAddressList, int32 remotePort, bool streamReset, int32 prMethod, uint32 numRequests)
{
    EV_INFO << "Socket connectx.  sockstate=" << stateName(sockstate) << "\n";
    remoteAddresses = remoteAddressList;
    connect(remoteAddressList.front(), remotePort, streamReset, prMethod, numRequests);
}


void SctpSocket::send(Packet *packet)
{
    auto sendReq = packet->findTagForUpdate<SctpSendReq>();
    if (sendReq) {
        if (sendReq->getSid() == -1) {
            lastStream = (lastStream + 1) % appOptions->outboundStreams;
            sendReq->setSid(lastStream);
        }
        if (sendReq->getSocketId() == -1) {
            sendReq->setSocketId(assocId);
        }
    } else {
        auto sctpReq = packet->addTagIfAbsent<SctpSendReq>();
        sctpReq->setSocketId(assocId);
        lastStream = (lastStream + 1) % appOptions->outboundStreams;
        sctpReq->setSid(lastStream);
    }
    packet->setKind(SCTP_C_SEND);
    sendToSctp(packet);
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

    Request *msg = new Request("CLOSE", SCTP_C_CLOSE);
    auto cmd = msg->addTag<SctpCommandReq>();
    if (id == -1)
        cmd->setSocketId(assocId);
    else
        cmd->setFd(id);
    sendToSctp(msg);
    sockstate = (sockstate == CONNECTED) ? LOCALLY_CLOSED : CLOSED;
}

void SctpSocket::shutdown(int id)
{
    EV << "SctpSocket::shutdown()\n";

    Request *msg = new Request("SHUTDOWN", SCTP_C_SHUTDOWN);
    auto cmd = msg->addTag<SctpCommandReq>();
    if (id == -1)
        cmd->setSocketId(assocId);
    else
        cmd->setFd(id);
    sendToSctp(msg);
}

void SctpSocket::destroy()
{
    EV << "SctpSocket::destroy()\n";

    Request *msg = new Request("DESTROY", SCTP_C_DESTROY);
    auto cmd = msg->addTag<SctpCommandReq>();
    cmd->setSocketId(assocId);
    sendToSctp(msg);
}

void SctpSocket::abort()
{
    if (sockstate != NOT_BOUND && sockstate != CLOSED && sockstate != SOCKERROR) {
        Request *msg = new Request("ABORT", SCTP_C_ABORT);
        auto cmd = msg->addTag<SctpCommandReq>();
        cmd->setSocketId(assocId);
        sendToSctp(msg);
    }
    sockstate = CLOSED;
}

void SctpSocket::requestStatus()
{
    Request *msg = new Request("STATUS", SCTP_C_STATUS);
    auto cmd = msg->addTag<SctpCommandReq>();
    cmd->setSocketId(assocId);
    sendToSctp(msg);
}

bool SctpSocket::belongsToSocket(cMessage *msg) const
{
    bool ret = (check_and_cast<Indication *>(msg)->getTag<SocketInd>()->getSocketId() == assocId);
    return ret;
}

void SctpSocket::setCallback(ICallback *callback)
{
    cb = callback;
}

void SctpSocket::processMessage(cMessage *msg)
{
    switch (msg->getKind()) {
        case SCTP_I_DATA:
            EV_INFO << "SCTP_I_DATA\n";
            if (cb) {
                cb->socketDataArrived(this, check_and_cast<Packet *>(msg), false);
                msg = nullptr;
            }
            break;

        case SCTP_I_DATA_NOTIFICATION:
            EV_INFO << "SCTP_I_NOTIFICATION\n";
            if (cb) {
                cb->socketDataNotificationArrived(this, check_and_cast<Message *>(msg));
            }
            break;

        case SCTP_I_SEND_MSG:
            if (cb) {
                cb->sendRequestArrived(this);
            }
            break;

        case SCTP_I_AVAILABLE: {
            auto indication = check_and_cast<Indication *>(msg);
            if (cb)
                cb->socketAvailable(this, indication);
            else {
                int newSocketId = indication->getTag<SctpAvailableReq>()->getNewSocketId();
                acceptSocket(newSocketId);
            }
            delete msg;
            break;
        }

        case SCTP_I_ESTABLISHED: {
            EV_INFO << "SCTP_I_ESTABLISHED\n";
            if (oneToOne)
                sockstate = CONNECTED;
            auto indication = check_and_cast<Indication *>(msg);
            auto connectInfo = indication->getTag<SctpConnectReq>();
            localAddr = connectInfo->getLocalAddr();
            remoteAddr = connectInfo->getRemoteAddr();
            localPrt = connectInfo->getLocalPort();
            remotePrt = connectInfo->getRemotePort();
            fsmStatus = connectInfo->getStatus();
            appOptions->inboundStreams = connectInfo->getInboundStreams();
            appOptions->outboundStreams = connectInfo->getOutboundStreams();
            assocId = indication->getTag<SocketInd>()->getSocketId();

            if (cb) {
                cb->socketEstablished(this, connectInfo->getNumMsgs());
            }
            delete msg;
            break;
        }

        case SCTP_I_PEER_CLOSED:
            EV_INFO << "peer closed\n";
            if (oneToOne)
                sockstate = (sockstate == CONNECTED) ? PEER_CLOSED : CLOSED;

            if (cb) {
                cb->socketPeerClosed(this);
            }
            delete msg;
            break;

        case SCTP_I_ABORT:
        case SCTP_I_CONN_LOST:
        case SCTP_I_CLOSED:
            EV_INFO << "SCTP_I_CLOSED called\n";
            sockstate = CLOSED;
            if (cb) {
                cb->socketClosed(this);
            }
            delete msg;
            break;

        case SCTP_I_CONNECTION_REFUSED:
        case SCTP_I_CONNECTION_RESET:
        case SCTP_I_TIMED_OUT:
            sockstate = SOCKERROR;
            if (cb) {
                cb->socketFailure(this, msg->getKind());
            }
            break;

        case SCTP_I_STATUS: {
            auto *message = check_and_cast<Indication *>(msg);
            auto status = message->getTagForUpdate<SctpStatusReq>();
            if (cb) {
                cb->socketStatusArrived(this, status.get());
            }
            break;
        }

        case SCTP_I_ABANDONED:
            if (cb) {
                cb->msgAbandonedArrived(this);
            }
            delete msg;
            break;

        case SCTP_I_SHUTDOWN_RECEIVED:
            EV_INFO << "SCTP_I_SHUTDOWN_RECEIVED\n";
            if (cb) {
                cb->shutdownReceivedArrived(this);
            }
            delete msg;
            break;

        case SCTP_I_SENDQUEUE_FULL:
            if (cb) {
                cb->sendqueueFullArrived(this);
            }
            break;

        case SCTP_I_SENDQUEUE_ABATED: {
            auto& tags = getTags(msg);
            auto cmd = tags.getTag<SctpCommandReq>();
            if (cb) {
                cb->sendqueueAbatedArrived(this, cmd->getNumMsgs());
            }
            break;
        }

        case SCTP_I_RCV_STREAMS_RESETTED:
        case SCTP_I_SEND_STREAMS_RESETTED:
        case SCTP_I_RESET_REQUEST_FAILED:
            delete msg;
            break;
        case SCTP_I_SENDSOCKETOPTIONS: {
            auto *indication = check_and_cast<Indication *>(msg);
            if (cb)
                cb->socketOptionsArrived(this, indication);
            else
                delete indication;
            break;
        }

        case SCTP_I_ADDRESS_ADDED: {
            EV_INFO << "SCTP_I_ADDRESS_ADDED\n";
            auto& tags = getTags(msg);
            auto cmd = tags.getTag<SctpCommandReq>();
            if (cb) {
                cb->addressAddedArrived(this, cmd->getLocalAddr(), remoteAddr);
            }
           // delete cmd;
            break;
        }

        default: {
            throw cRuntimeError("SctpSocket::processMessage(): invalid msg kind %d, one of the SCTP_I_xxx constants expected", msg->getKind());
        }
    }
}

void SctpSocket::setStreamPriority(uint32 stream, uint32 priority)
{
    Request *msg = new Request("SET_STREAM_PRIO", SCTP_C_SET_STREAM_PRIO);
    auto cmd = msg->addTag<SctpSendReq>();
    cmd->setSocketId(assocId);
    cmd->setSid(stream);
    cmd->setPpid(priority);
    sendToSctp(msg);
}

void SctpSocket::setRtoInfo(double initial, double max, double min)
{
    sOptions->rtoInitial = initial;
    sOptions->rtoMax = max;
    sOptions->rtoMin = min;
    if (sockstate == CONNECTED) {
        Request *msg = new Request("RtoInfo", SCTP_C_SET_RTO_INFO);
        auto cmd = msg->addTag<SctpRtoReq>();
        cmd->setSocketId(assocId);
        cmd->setRtoInitial(initial);
        cmd->setRtoMin(min);
        cmd->setRtoMax(max);
        sendToSctp(msg);
    }
}

bool SctpSocket::isOpen() const
{
    switch (sockstate) {
    case LISTENING:
    case CONNECTING:
    case CONNECTED:
    case PEER_CLOSED:
    case LOCALLY_CLOSED:
    case SOCKERROR: //TODO check SOCKERROR is opened or is closed socket
        return true;
    case NOT_BOUND:
    case CLOSED:
        return false;
    default:
        throw cRuntimeError("invalid SctpSocket state: %d", sockstate);
    }
}

} // namespace inet
