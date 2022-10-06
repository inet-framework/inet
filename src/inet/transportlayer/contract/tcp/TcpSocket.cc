//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/contract/tcp/TcpSocket.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"

namespace inet {

TcpSocket::TcpSocket()
{
    // don't allow user-specified connIds because they may conflict with
    // automatically assigned ones.
    connId = getActiveSimulationOrEnvir()->getUniqueNumber();
}

TcpSocket::TcpSocket(cMessage *msg)
{
    connId = check_and_cast<Indication *>(msg)->getTag<SocketInd>()->getSocketId();
    sockstate = CONNECTED;

    if (msg->getKind() == TCP_I_AVAILABLE) {
        TcpAvailableInfo *availableInfo = check_and_cast<TcpAvailableInfo *>(msg->getControlInfo());
        connId = availableInfo->getNewSocketId();
        localAddr = availableInfo->getLocalAddr();
        remoteAddr = availableInfo->getRemoteAddr();
        localPrt = availableInfo->getLocalPort();
        remotePrt = availableInfo->getRemotePort();
    }
    else if (msg->getKind() == TCP_I_ESTABLISHED) {
        // management of stockstate is left to processMessage() so we always
        // set it to CONNECTED in the ctor, whatever TCP_I_xxx arrives.
        // However, for convenience we extract TcpConnectInfo already here, so that
        // remote address/port can be read already after the ctor call.

        TcpConnectInfo *connectInfo = check_and_cast<TcpConnectInfo *>(msg->getControlInfo());
        localAddr = connectInfo->getLocalAddr();
        remoteAddr = connectInfo->getRemoteAddr();
        localPrt = connectInfo->getLocalPort();
        remotePrt = connectInfo->getRemotePort();
    }
}

TcpSocket::TcpSocket(TcpAvailableInfo *availableInfo)
{
    connId = availableInfo->getNewSocketId();
    sockstate = CONNECTED;
    localAddr = availableInfo->getLocalAddr();
    remoteAddr = availableInfo->getRemoteAddr();
    localPrt = availableInfo->getLocalPort();
    remotePrt = availableInfo->getRemotePort();
}

TcpSocket::~TcpSocket()
{
    if (cb) {
        cb->socketDeleted(this);
        cb = nullptr;
    }
    delete receiveQueue;
}

void TcpSocket::bind(int lPort)
{
    if (sockstate != NOT_BOUND)
        throw cRuntimeError("TcpSocket::bind(): socket already bound");

    if (lPort < 0 || lPort > 65535)
        throw cRuntimeError("TcpSocket::bind(): invalid port number %d", lPort);

    localPrt = lPort;
    sockstate = BOUND;
}

void TcpSocket::bind(L3Address lAddr, int lPort)
{
    if (sockstate != NOT_BOUND)
        throw cRuntimeError("TcpSocket::bind(): socket already bound");

    // allow -1 here, to make it possible to specify address only
    if ((lPort < 0 || lPort > 65535) && lPort != -1)
        throw cRuntimeError("TcpSocket::bind(): invalid port number %d", lPort);

    localAddr = lAddr;
    localPrt = lPort;
    sockstate = BOUND;
}

void TcpSocket::listen(bool fork)
{
    if (sockstate != BOUND)
        throw cRuntimeError(sockstate == NOT_BOUND ? "TcpSocket: must call bind() before listen()"
                : "TcpSocket::listen(): connect() or listen() already called");

    auto request = new Request("PassiveOPEN", TCP_C_OPEN_PASSIVE);

    TcpOpenCommand *openCmd = new TcpOpenCommand();
    openCmd->setLocalAddr(localAddr);
    openCmd->setLocalPort(localPrt);
    openCmd->setFork(fork);
    openCmd->setTcpAlgorithmClass(tcpAlgorithmClass.c_str());

    request->setControlInfo(openCmd);
    sendToTcp(request);
    sockstate = LISTENING;
}

void TcpSocket::accept(int socketId)
{
    auto request = new Request("ACCEPT", TCP_C_ACCEPT);
    TcpAcceptCommand *acceptCmd = new TcpAcceptCommand();
    request->setControlInfo(acceptCmd);
    sendToTcp(request, socketId);
}

void TcpSocket::connect(L3Address remoteAddress, int remotePort)
{
    if (sockstate != NOT_BOUND && sockstate != BOUND)
        throw cRuntimeError("TcpSocket::connect(): connect() or listen() already called (need renewSocket()?)");

    if (remotePort < 0 || remotePort > 65535)
        throw cRuntimeError("TcpSocket::connect(): invalid remote port number %d", remotePort);

    auto request = new Request("ActiveOPEN", TCP_C_OPEN_ACTIVE);

    remoteAddr = remoteAddress;
    remotePrt = remotePort;

    TcpOpenCommand *openCmd = new TcpOpenCommand();
    openCmd->setLocalAddr(localAddr);
    openCmd->setLocalPort(localPrt);
    openCmd->setRemoteAddr(remoteAddr);
    openCmd->setRemotePort(remotePrt);
    openCmd->setTcpAlgorithmClass(tcpAlgorithmClass.c_str());

    request->setControlInfo(openCmd);
    sendToTcp(request);
    sockstate = CONNECTING;
}

void TcpSocket::send(Packet *msg)
{
    if (sockstate != CONNECTED && sockstate != CONNECTING && sockstate != PEER_CLOSED)
        throw cRuntimeError("TcpSocket::send(): socket not connected or connecting, state is %s", stateName(sockstate));

    msg->setKind(TCP_C_SEND);
    sendToTcp(msg);
}

void TcpSocket::sendCommand(Request *msg)
{
    sendToTcp(msg);
}

void TcpSocket::close()
{
    if (sockstate != CONNECTED && sockstate != PEER_CLOSED && sockstate != CONNECTING && sockstate != LISTENING) {
//        throw cRuntimeError("TcpSocket::close(): not connected or close() already called (sockstate=%s)", stateName(sockstate));
    }
    else {
        auto request = new Request("CLOSE", TCP_C_CLOSE);
        TcpCommand *cmd = new TcpCommand();
        request->setControlInfo(cmd);
        sendToTcp(request);
        sockstate = (sockstate == CONNECTED) ? LOCALLY_CLOSED : CLOSED;
    }
}

void TcpSocket::abort()
{
    if (sockstate != NOT_BOUND && sockstate != BOUND && sockstate != CLOSED && sockstate != SOCKERROR) {
        auto request = new Request("ABORT", TCP_C_ABORT);
        TcpCommand *cmd = new TcpCommand();
        request->setControlInfo(cmd);
        sendToTcp(request);
    }
    sockstate = CLOSED;
}

void TcpSocket::destroy()
{
    auto request = new Request("DESTROY", TCP_C_DESTROY);
    TcpCommand *cmd = new TcpCommand();
    request->setControlInfo(cmd);
    sendToTcp(request);
    sockstate = CLOSED;
}

void TcpSocket::requestStatus()
{
    auto request = new Request("STATUS", TCP_C_STATUS);
    TcpCommand *cmd = new TcpCommand();
    request->setControlInfo(cmd);
    sendToTcp(request);
}

// ########################
// TCP Socket Options Start
// ########################

void TcpSocket::setTimeToLive(int ttl)
{
    auto request = new Request("setTTL", TCP_C_SETOPTION);
    TcpSetTimeToLiveCommand *cmd = new TcpSetTimeToLiveCommand();
    cmd->setTtl(ttl);
    request->setControlInfo(cmd);
    sendToTcp(request);
}

void TcpSocket::setDscp(short dscp)
{
    auto request = new Request("setDscp", TCP_C_SETOPTION);
    auto *cmd = new TcpSetDscpCommand();
    cmd->setDscp(dscp);
    request->setControlInfo(cmd);
    sendToTcp(request);
}

void TcpSocket::setTos(short dscp)
{
    auto request = new Request("setTOS", TCP_C_SETOPTION);
    auto *cmd = new TcpSetTosCommand();
    cmd->setTos(dscp);
    request->setControlInfo(cmd);
    sendToTcp(request);
}

// ######################
// TCP Socket Options End
// ######################

void TcpSocket::sendToTcp(cMessage *msg, int connId)
{
    if (!gateToTcp)
        throw cRuntimeError("TcpSocket: setOutputGate() must be invoked before socket can be used");

    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::tcp);
    tags.addTagIfAbsent<SocketReq>()->setSocketId(connId == -1 ? this->connId : connId);
    check_and_cast<cSimpleModule *>(gateToTcp->getOwnerModule())->send(msg, gateToTcp);
}

void TcpSocket::renewSocket()
{
    connId = getActiveSimulationOrEnvir()->getUniqueNumber();
    remoteAddr = localAddr = L3Address();
    remotePrt = localPrt = -1;
    sockstate = NOT_BOUND;
}

bool TcpSocket::isOpen() const
{
    switch (sockstate) {
        case BOUND:
        case LISTENING:
        case CONNECTING:
        case CONNECTED:
        case PEER_CLOSED:
        case LOCALLY_CLOSED:
        case SOCKERROR: // TODO check SOCKERROR is opened or is closed socket
            return true;
        case NOT_BOUND:
        case CLOSED:
            return false;
        default:
            throw cRuntimeError("invalid TcpSocket state: %d", sockstate);
    }
}

void TcpSocket::setCallback(ICallback *callback)
{
    cb = callback;
}

void TcpSocket::processMessage(cMessage *msg)
{
    ASSERT(belongsToSocket(msg));

    TcpStatusInfo *status;
    TcpAvailableInfo *availableInfo;
    TcpConnectInfo *connectInfo;

    switch (msg->getKind()) {
        case TCP_I_DATA:
            if (cb)
                cb->socketDataArrived(this, check_and_cast<Packet *>(msg), false);
            else
                delete msg;
            break;

        case TCP_I_URGENT_DATA:
            if (cb)
                cb->socketDataArrived(this, check_and_cast<Packet *>(msg), true);
            else
                delete msg;
            break;

        case TCP_I_AVAILABLE:
            availableInfo = check_and_cast<TcpAvailableInfo *>(msg->getControlInfo());
            if (cb)
                cb->socketAvailable(this, availableInfo);
            else
                accept(availableInfo->getNewSocketId());
            delete msg;
            break;

        case TCP_I_ESTABLISHED:
            // Note: this code is only for sockets doing active open, and nonforking
            // listening sockets. For a forking listening sockets, TCP_I_ESTABLISHED
            // carries a new connId which won't match the connId of this TcpSocket,
            // so you won't get here. Rather, when you see TCP_I_ESTABLISHED, you'll
            // want to create a new TcpSocket object via new TcpSocket(msg).
            sockstate = CONNECTED;
            connectInfo = check_and_cast<TcpConnectInfo *>(msg->getControlInfo());
            localAddr = connectInfo->getLocalAddr();
            remoteAddr = connectInfo->getRemoteAddr();
            localPrt = connectInfo->getLocalPort();
            remotePrt = connectInfo->getRemotePort();
            if (cb)
                cb->socketEstablished(this);
            delete msg;
            break;

        case TCP_I_PEER_CLOSED:
            sockstate = PEER_CLOSED;
            if (cb)
                cb->socketPeerClosed(this);
            delete msg;
            break;

        case TCP_I_CLOSED:
            sockstate = CLOSED;
            if (cb)
                cb->socketClosed(this);
            delete msg;
            break;

        case TCP_I_CONNECTION_REFUSED:
        case TCP_I_CONNECTION_RESET:
        case TCP_I_TIMED_OUT:
            sockstate = SOCKERROR;
            if (cb)
                cb->socketFailure(this, msg->getKind());
            delete msg;
            break;

        case TCP_I_STATUS:
            status = check_and_cast<TcpStatusInfo *>(msg->getControlInfo());
            if (cb)
                cb->socketStatusArrived(this, status);
            delete msg;
            break;

        default:
            throw cRuntimeError("TcpSocket: invalid msg kind %d, one of the TCP_I_xxx constants expected", msg->getKind());
    }
}

bool TcpSocket::belongsToSocket(cMessage *msg) const
{
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    const auto& socketInd = tags.findTag<SocketInd>();
    return socketInd != nullptr && socketInd->getSocketId() == connId;
}

const char *TcpSocket::stateName(TcpSocket::State state)
{
#define CASE(x)    case x: \
        s = #x; break
    const char *s = "unknown";
    switch (state) {
        CASE(NOT_BOUND);
        CASE(BOUND);
        CASE(LISTENING);
        CASE(CONNECTING);
        CASE(CONNECTED);
        CASE(PEER_CLOSED);
        CASE(LOCALLY_CLOSED);
        CASE(CLOSED);
        CASE(SOCKERROR);
    }
    return s;
#undef CASE
}

} // namespace inet

