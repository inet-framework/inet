//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "QuicSocket.h"
#include "QuicCommand_m.h"
#include "inet/common/Protocol.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/tag/ITaggedObject.h"

namespace inet {

QuicSocket::QuicSocket()
{
    gateToQuic = nullptr;
    cb = nullptr;
    socketState = NOT_BOUND;
    socketId = generateSocketId();
}

QuicSocket::~QuicSocket()
{
    if (cb) {
        cb->socketDeleted(this);
        cb = nullptr;
    }
}

int QuicSocket::generateSocketId()
{
    return getEnvir()->getUniqueNumber();
}

void QuicSocket::sendToQuic(cMessage *msg)
{
    if (!gateToQuic)
        throw cRuntimeError("QuicSocket: setOutputGate() must be invoked before socket can be used");

    cObject *ctrl = msg->getControlInfo();
    EV_TRACE << "QuicSocket: Send (" << msg->getClassName() << ")" << msg->getFullName();
    if (ctrl)
        EV_TRACE << "  control info: (" << ctrl->getClassName() << ")" << ctrl->getFullName();
    EV_TRACE << endl;

    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::quic);
    tags.addTagIfAbsent<SocketReq>()->setSocketId(socketId);

    check_and_cast<cSimpleModule *>(gateToQuic->getOwnerModule())->send(msg, gateToQuic);
}

void QuicSocket::bind(L3Address localAddr, uint16_t localPort)
{
    this->localAddr = localAddr;
    this->localPort = localPort;

    QuicBindCommand *ctrl = new QuicBindCommand();
    ctrl->setLocalAddr(localAddr);
    ctrl->setLocalPort(localPort);
    Request *request = new Request("BIND", QUIC_C_CREATE_PCB);
    request->setControlInfo(ctrl);
    sendToQuic(request);
    socketState = BOUND;
}

void QuicSocket::listen()
{
    Request *request = new Request("LISTEN", QUIC_C_OPEN_PASSIVE);
    sendToQuic(request);
    socketState = LISTENING;
}

void QuicSocket::connect(L3Address addr, uint16_t port)
{
    if (addr.isUnspecified())
        throw cRuntimeError("QuicSocket::connect(): unspecified remote address");

    QuicOpenCommand *cmd = new QuicOpenCommand();
    cmd->setRemoteAddr(addr);
    cmd->setRemotePort(port);
    Request *request = new Request("CONNECT", QUIC_C_OPEN_ACTIVE);
    request->setControlInfo(cmd);
    sendToQuic(request);

    socketState = CONNECTING;
}

void QuicSocket::send(Packet *msg, uint64_t streamId)
{
    if (streamId >= ( ((uint64_t)1) << 62)) {
        throw cRuntimeError("QuicSocket::send(): streamId too large");
    }

    msg->setKind(QUIC_C_SEND);

    // set stream ID
    auto& tags = msg->getTags();
    tags.addTagIfAbsent<QuicStreamReq>()->setStreamID(streamId);

    sendToQuic(msg);
}

void QuicSocket::send(Packet *msg)
{
    send(msg, 0);
}

void QuicSocket::recv(int64_t length, uint64_t streamId)
{
    if (length < 0) {
        throw cRuntimeError("QuicSocket::recv(): negative length not allowed");
    }
    QuicRecvCommand *cmd = new QuicRecvCommand();
    cmd->setStreamID(streamId);
    cmd->setExpectedDataSize(length);
    Request *request = new Request("RECV", QUIC_C_RECEIVE);
    request->setControlInfo(cmd);
    sendToQuic(request);
}

void QuicSocket::close()
{
    Request *msg = new Request("CLOSE", QUIC_C_CLOSE);
    sendToQuic(msg);
    socketState = CLOSED;
}

bool QuicSocket::isOpen() const
{
    switch (socketState) {
    case BOUND:
    case LISTENING:
    case CONNECTING:
    case CONNECTED:
        return true;
    case NOT_BOUND:
    case CLOSED:
        return false;
    default:
        throw cRuntimeError("invalid QuicSocket state: %d", socketState);
    }
}

bool QuicSocket::belongsToSocket(cMessage *msg) const
{
    ITaggedObject *taggedObject = dynamic_cast<ITaggedObject *>(msg);
    if (taggedObject == nullptr) {
        return false;
    }
    auto& tags = taggedObject->getTags();
    if (!tags.hasTag<SocketInd>()) {
        return false;
    }
    return tags.getTag<SocketInd>()->getSocketId() == socketId;
}

void QuicSocket::destroy()
{
    throw cRuntimeError("QuicSocket::destroy not implemented");
}

QuicSocket *QuicSocket::accept()
{
    QuicSocket *newSocket = new QuicSocket();
    newSocket->setOutputGate(gateToQuic);
    newSocket->bind(localAddr, localPort);
    EV_DEBUG << "QuicSocket::accept(): new socket created with socket ID " << newSocket->getSocketId() << endl;
    Request *request = new Request("ACCEPT", QUIC_C_ACCEPT);
    QuicAcceptCommand *cmd = new QuicAcceptCommand();
    cmd->setNewSocketId(newSocket->getSocketId());
    request->setControlInfo(cmd);
    sendToQuic(request);
    return newSocket;
}

void QuicSocket::processMessage(cMessage *msg) {
    ASSERT(belongsToSocket(msg));

    switch (msg->getKind()) {
        case QUIC_I_CONNECTION_AVAILABLE: {
            if (cb) {
                cb->socketConnectionAvailable(this);
            }
            delete msg;
            break;
        }
        case QUIC_I_ESTABLISHED: {
            socketState = CONNECTED;
            QuicConnectionInfo *connectInfo = check_and_cast<QuicConnectionInfo *>(msg->getControlInfo());
            localAddr = connectInfo->getLocalAddr();
            remoteAddr = connectInfo->getRemoteAddr();
            localPort = connectInfo->getLocalPort();
            remotePort = connectInfo->getRemotePort();
            if (cb) {
                cb->socketEstablished(this);
            }
            delete msg;
            break;
        }
        case QUIC_I_DATA_AVAILABLE: {
            if (cb) {
                QuicDataInfo *dataInfo = check_and_cast<QuicDataInfo *>(msg->getControlInfo());
                cb->socketDataAvailable(this, dataInfo);
            }
            delete msg;
            break;
        }
        case QUIC_I_DATA: {
            if (cb) {
                Packet *packet = check_and_cast<Packet*>(msg);
                cb->socketDataArrived(this, packet);
            } else {
                delete msg;
            }
            break;
        }
        case QUIC_I_CLOSED: {
            socketState = CLOSED;
            if (cb) {
                cb->socketClosed(this);
            }
            delete msg;
            break;
        }
        case QUIC_I_SENDQUEUE_FULL: {
            if (cb) {
                cb->socketSendQueueFull(this);
            }
            delete msg;
            break;
        }
        case QUIC_I_SENDQUEUE_DRAIN: {
            if (cb) {
                cb->socketSendQueueDrain(this);
            }
            delete msg;
            break;
        }
        case QUIC_I_MSG_REJECTED: {
            if (cb) {
                cb->socketMsgRejected(this);
            }
            delete msg;
            break;
        }
        default: {
            throw cRuntimeError("QuicSocket: invalid msg kind %d, one of the QUIC_I_xxx constants expected", msg->getKind());
        }
    }
}

} // namespace inet

