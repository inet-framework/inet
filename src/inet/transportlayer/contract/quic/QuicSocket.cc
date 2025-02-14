//
// Copyright (C) 2018 CaDS HAW Hamburg BCK, Denis Lugowski, Marvin Butkereit
// Copyright (C) 2005,2011 Andras Varga
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

#include "QuicSocket.h"

#include "inet/common/Protocol.h"
#include "QuicCommand_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/ProtocolTag_m.h"
//#ifdef WITH_IPv4
#include "inet/common/packet/Message.h"
#include "inet/common/packet/tag/ITaggedObject.h"
//#endif // ifdef WITH_IPv4

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

void QuicSocket::bind(int localPort)
{
    bind(L3Address(), localPort);
}

void QuicSocket::bind(L3Address localAddr, int localPort)
{
    if (localPort < -1 || localPort > 65535) // -1: ephemeral port
        throw cRuntimeError("QuicSocket::bind(): invalid port number %d", localPort);

    this->localAddr = localAddr;
    this->localPort = localPort;

    QuicBindCommand *ctrl = new QuicBindCommand();
    //ctrl->setConnId(connectionID);
    ctrl->setLocalAddr(localAddr);
    ctrl->setLocalPort(localPort);
    Request *request = new Request("BIND", QUIC_C_CREATE_PCB);
    request->setControlInfo(ctrl);
    sendToQuic(request);
    socketState = BOUND;
}

void QuicSocket::listen()
{
    QuicOpenCommand *ctrl = new QuicOpenCommand();
    //ctrl->setConnId(connectionID);
    ctrl->setLocalAddr(localAddr);
    ctrl->setLocalPort(localPort);
    //ctrl->setMaxNumStreams(maxNumStreams);
    Request *request = new Request("LISTEN", QUIC_C_OPEN_PASSIVE);
    request->setControlInfo(ctrl);
    sendToQuic(request);
    socketState = LISTENING;
}

void QuicSocket::connect(L3Address addr, int port)
{
    if (addr.isUnspecified())
        throw cRuntimeError("QuicSocket::connect(): unspecified remote address");
    if (port <= 0 || port > 65535)
        throw cRuntimeError("QuicSocket::connect(): invalid remote port number %d", port);

    QuicOpenCommand *ctrl = new QuicOpenCommand();
    //ctrl->setConnId(connectionID);
    ctrl->setRemoteAddr(addr);
    ctrl->setRemotePort(port);
    //ctrl->setMaxNumStreams(maxNumStreams);
    Request *request = new Request("CONNECT", QUIC_C_OPEN_ACTIVE);
    request->setControlInfo(ctrl);
    sendToQuic(request);

    socketState = CONNECTING;
}

void QuicSocket::send(Packet *msg)
{
    //QuicSendCommand* sendCmd = (QuicSendCommand*)(pk->removeControlInfo());

     msg->setKind(QUIC_C_SEND);

    /*
    QuicSendCommand *ctrl = new QuicSendCommand();
    //ctrl->setConnId(connectionID);

    if (sendCmd->getStreamID() == -1) {
        // No stream id specified - take next available stream
        lastStream = (lastStream + 1) % maxNumStreams;
        ctrl->setStreamID(lastStream);
    }
    else {
        ctrl->setStreamID(sendCmd->getStreamID());
    }

    pk->setControlInfo(ctrl);
    */
    sendToQuic(msg);
}

void QuicSocket::recv(QuicRecvCommand *ctrlInfo)
{
    Request *request = new Request("RECV", QUIC_C_RECEIVE);
    request->setControlInfo(ctrlInfo);
    sendToQuic(request);
}

void QuicSocket::recv(long length, long streamId)
{
    QuicRecvCommand *ctrlInfo = new QuicRecvCommand();
    ctrlInfo->setStreamID(streamId);
    ctrlInfo->setExpectedDataSize(length);
    this->recv(ctrlInfo);
}

/*
void QuicSocket::setStreamPriority(uint64_t streamID, uint32_t priority)
{
    cMessage *msg = new cMessage("SET_STREAM_PRIO", QUIC_C_SET_STREAM_PRIO);
    QUICSendCommand* ctrl = new QUICSendCommand();
    ctrl->setConnId(connectionID);
    ctrl->setStreamID(streamID);
    ctrl->setStreamPriority(priority);
    msg->setControlInfo(ctrl);
    sendToQUIC(msg);
}
*/
void QuicSocket::close()
{
    Request *msg = new Request("CLOSE", QUIC_C_SEND_APP_CLOSE);
    QuicCloseCommand *ctrl = new QuicCloseCommand();
    //ctrl->setConnId(connectionID);
    msg->setControlInfo(ctrl);
    sendToQuic(msg);
    socketState = CLOSED;
}
/*
void QuicSocket::sendRequest(cMessage *msg)
{
    sendToQUIC(msg);
}
*/
bool QuicSocket::isOpen() const
{
    switch (socketState) {
    case BOUND:
    case LISTENING:
    case CONNECTING:
    case CONNECTED:
    //case PEER_CLOSED:
    //case LOCALLY_CLOSED:
    //case SOCKERROR: //TODO check SOCKERROR is opened or is closed socket
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
    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    return tags.getTag<SocketInd>()->getSocketId() == socketId;
}

void QuicSocket::destroy()
{
    throw cRuntimeError("QuicSocket::destroy not implemented");
}

void QuicSocket::accept(int socketId)
{
    Request *request = new Request("ACCEPT", QUIC_C_ACCEPT);
    QuicAcceptCommand *acceptCmd = new QuicAcceptCommand();
    request->setControlInfo(acceptCmd);
    sendToQuic(request);
}

void QuicSocket::processMessage(cMessage *msg) {
    ASSERT(belongsToSocket(msg));

    switch (msg->getKind()) {
        case QUIC_I_AVAILABLE: {
            QuicAvailableInfo *availableInfo = check_and_cast<QuicAvailableInfo *>(msg->getControlInfo());
            if (cb) {
                cb->socketAvailable(this, availableInfo);
            } else {
                accept(availableInfo->getNewSocketId());
            }
            delete msg;
            break;
        }
        case QUIC_I_ESTABLISHED: {
            // Note: this code is only for sockets doing active open, and nonforking
            // listening sockets. For a forking listening sockets, TCP_I_ESTABLISHED
            // carries a new connId which won't match the connId of this TcpSocket,
            // so you won't get here. Rather, when you see TCP_I_ESTABLISHED, you'll
            // want to create a new TcpSocket object via new TcpSocket(msg).
            socketState = CONNECTED;
            QuicConnectInfo *connectInfo = check_and_cast<QuicConnectInfo *>(msg->getControlInfo());
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
        case QUIC_I_DATA_NOTIFICATION: {
            delete msg;
            break;
        }
        case QUIC_I_DATA: {
            if (cb) {
                cb->socketDataArrived(this, check_and_cast<Packet*>(msg));
            } else {
                delete msg;
            }
            break;
        }
        /*
        case QUIC_I_PEER_CLOSED:
            sockstate = PEER_CLOSED;
            delete msg;
            break;
        */
        case QUIC_I_CLOSED: {
            socketState = CLOSED;
            if (cb) {
                cb->socketClosed(this);
            }
            delete msg;
            break;
        }
        /*
        case TCP_I_CONNECTION_REFUSED:
        case TCP_I_CONNECTION_RESET:
        case TCP_I_TIMED_OUT:
            sockstate = SOCKERROR;
            delete msg;
            break;

        case TCP_I_STATUS:
            status = check_and_cast<TcpStatusInfo *>(msg->getControlInfo());
            delete msg;
            break;
         */
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

