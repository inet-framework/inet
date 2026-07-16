//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tcpapp/TcpGenericServerApp.h"

#include "inet/applications/tcpapp/GenericAppMsg_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/SimulationContinuation.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"

namespace inet {

Define_Module(TcpGenericServerApp);

void TcpGenericServerApp::initialize(int stage)
{
    SimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        delay = par("replyDelay");
        maxMsgDelay = 0;

        // statistics
        msgsRcvd = msgsSent = bytesRcvd = bytesSent = 0;

        WATCH(maxMsgDelay);
        WATCH(msgsRcvd);
        WATCH(msgsSent);
        WATCH(bytesRcvd);
        WATCH(bytesSent);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        autoRead = par("autoRead");
        socket.setOutputGate(gate("socketOut"));
        socket.setCallback(this); // to learn the socket ids of forked connections
        DispatchProtocolReq dispatchProtocolReq;
        dispatchProtocolReq.setProtocol(&Protocol::tcp);
        dispatchProtocolReq.setServicePrimitive(SP_REQUEST);
        socketOutSink.reference(gate("socketOut"), true, &dispatchProtocolReq);
        tcp.reference(gate("socketOut"), true);
        socket.bind(localAddress[0] ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
        socket.setAutoRead(autoRead);
        socket.listen();

        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void TcpGenericServerApp::sendOrSchedule(cMessage *msg, simtime_t delay)
{
    if (delay == 0)
        sendBack(msg);
    else
        scheduleAfter(delay, msg);
}

void TcpGenericServerApp::sendBack(cMessage *msg)
{
    if (auto packet = dynamic_cast<Packet *>(msg)) {
        msgsSent++;
        bytesSent += packet->getByteLength();
        emit(packetSentSignal, packet);
        EV_INFO << "sending \"" << packet->getName() << "\" to TCP, " << packet->getByteLength() << " bytes\n";
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::tcp);
        yieldBeforePush();
        socketOutSink.pushPacket(packet);
    }
    else {
        // socket commands are direct method calls into TCP
        auto request = check_and_cast<Request *>(msg);
        int connId = request->getTags().getTag<SocketReq>()->getSocketId();
        EV_INFO << "sending \"" << msg->getName() << "\" to TCP\n";
        switch (request->getKind()) {
            case TCP_C_CLOSE:
                tcp->close(connId);
                break;
            case TCP_C_READ: {
                auto readCommand = check_and_cast<TcpReadCommand *>(request->getControlInfo());
                tcp->read(connId, readCommand->getMaxByteCount());
                break;
            }
            default:
                throw cRuntimeError("Unknown command to TCP: kind=%d", request->getKind());
        }
        delete msg;
    }
}

void TcpGenericServerApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        sendBack(msg);
    }
    else if (msg->getKind() == TCP_I_PEER_CLOSED) {
        // we'll close too, but only after there's surely no message
        // pending to be sent back in this connection
        int connId = check_and_cast<Indication *>(msg)->getTag<SocketInd>()->getSocketId();
        delete msg;
        auto request = new Request("close", TCP_C_CLOSE);
        request->addTag<SocketReq>()->setSocketId(connId);
        sendOrSchedule(request, delay + maxMsgDelay);
    }
    else if (msg->getKind() == TCP_I_DATA || msg->getKind() == TCP_I_URGENT_DATA) {
        Packet *packet = check_and_cast<Packet *>(msg);
        int connId = packet->getTag<SocketInd>()->getSocketId();
        ChunkQueue& queue = socketQueue[connId];
        auto chunk = packet->peekDataAt(B(0), packet->getDataLength());
        queue.push(chunk);
        emit(packetReceivedSignal, packet);
        sendOrScheduleReadCommandIfNeeded(connId);

        bool doClose = false;
        // Frame the received byte stream using the GenericAppMsgReq region tags the
        // client attached to each request (see GenericAppMsg.msg). The request length
        // is read from the tag's requestLength field rather than from the tag's span,
        // so framing survives TCP segmentation splitting a request across segments.
        while (queue.getLength() > b(0)) {
            const auto& data = queue.peek(queue.getLength());
            const GenericAppMsgReq *req = nullptr;
            for (const auto& regionTag : data->getAllTags<GenericAppMsgReq>()) {
                if (regionTag.getOffset() == b(0)) {
                    req = regionTag.getTag().get();
                    break;
                }
            }
            if (req == nullptr)
                throw cRuntimeError("Received %s of TCP data without a GenericAppMsgReq region tag "
                        "at its front. GenericApp request/reply needs a transport that preserves "
                        "region tags; it cannot run over TcpLwip or emulation, where application "
                        "data crosses as raw bytes.", queue.getLength().str().c_str());
            B requestLength = req->getRequestLength();
            if (queue.getLength() < requestLength)
                break; // the whole request has not been received yet
            B requestedBytes = req->getExpectedReplyLength();
            simtime_t msgDelay = req->getReplyDelay();
            bool serverClose = req->getServerClose();
            queue.pop(requestLength);
            msgsRcvd++;
            bytesRcvd += requestLength.get<B>();
            if (msgDelay > maxMsgDelay)
                maxMsgDelay = msgDelay;

            if (requestedBytes > B(0)) {
                Packet *outPacket = new Packet(msg->getName(), TCP_C_SEND);
                outPacket->addTag<SocketReq>()->setSocketId(connId);
                const auto& payload = makeShared<ByteCountChunk>(requestedBytes);
                payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
                outPacket->insertAtBack(payload);
                sendOrSchedule(outPacket, delay + msgDelay);
            }
            if (serverClose) {
                doClose = true;
                break;
            }
        }
        delete msg;

        if (doClose) {
            auto request = new Request("close", TCP_C_CLOSE);
            TcpCommand *cmd = new TcpCommand();
            request->addTag<SocketReq>()->setSocketId(connId);
            request->setControlInfo(cmd);
            sendOrSchedule(request, delay + maxMsgDelay);
        }
    }
    else if (msg->getKind() == TCP_I_AVAILABLE) {
        socket.processMessage(msg);
    }
    else if (msg->getKind() == TCP_I_ESTABLISHED) {
        auto connectInfo = check_and_cast<TcpConnectInfo *>(msg->getControlInfo());
        ASSERT(autoRead == connectInfo->getAutoRead());
        if (!autoRead) {
            int connId = check_and_cast<Indication *>(msg)->getTag<SocketInd>()->getSocketId();
            sendOrScheduleReadCommandIfNeeded(connId);
        }
        delete msg;
    }
    else {
        // some indication -- ignore
        EV_WARN << "drop msg: " << msg->getName() << ", kind:" << msg->getKind() << "(" << cEnum::get("inet::TcpStatusInd")->getStringFor(msg->getKind()) << ")\n";
        delete msg;
    }
}


void TcpGenericServerApp::socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo)
{
    // new connection: create a socket object for it so TCP indications arrive
    // through the callbacks below, and accept it
    TcpSocket *newSocket = new TcpSocket(availableInfo);
    newSocket->setOutputGate(gate("socketOut"));
    newSocket->setCallback(this);
    socketMap.addSocket(newSocket);
    socket->accept(availableInfo->getNewSocketId());
}

void TcpGenericServerApp::socketEstablished(TcpSocket *socket, Indication *indication)
{
    // note: the indication is owned and deleted by TcpSocket::processMessage
    if (!autoRead)
        sendOrScheduleReadCommandIfNeeded(socket->getSocketId());
}

void TcpGenericServerApp::socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent)
{
    // feed the legacy message-style processing
    packet->setKind(TCP_I_DATA);
    handleMessage(packet);
}

void TcpGenericServerApp::socketPeerClosed(TcpSocket *socket)
{
    // we'll close too, but only after there's surely no message
    // pending to be sent back in this connection
    auto request = new Request("close", TCP_C_CLOSE);
    request->addTag<SocketReq>()->setSocketId(socket->getSocketId());
    sendOrSchedule(request, delay + maxMsgDelay);
}

void TcpGenericServerApp::socketDeleted(TcpSocket *socket)
{
    // called from the TcpSocket destructor: drop references, but don't delete
    socketMap.removeSocket(socket);
    socketQueue.erase(socket->getSocketId());
}

void TcpGenericServerApp::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    packet->setArrival(getId(), gate->getId());
    auto socketInd = packet->getTag<SocketInd>();
    if (auto connectionSocket = socketMap.findSocketById(socketInd->getSocketId()))
        connectionSocket->processMessage(packet);
    else
        socket.processMessage(packet);
}

cGate *TcpGenericServerApp::lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction)
{
    Enter_Method("lookupModuleInterface");
    EV_TRACE << "Looking up module interface" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
    if (gate->isName("socketIn")) {
        if (type == typeid(queueing::IPassivePacketSink)) {
            auto socketInd = dynamic_cast<const SocketInd *>(arguments);
            if (socketInd != nullptr && (socketInd->getSocketId() == socket.getSocketId() || socketMap.findSocketById(socketInd->getSocketId()) != nullptr))
                return gate;
            auto packetServiceTag = dynamic_cast<const PacketServiceTag *>(arguments);
            if (packetServiceTag != nullptr && packetServiceTag->getProtocol() == &Protocol::tcp)
                return gate;
        }
    }
    return nullptr;
}

TcpGenericServerApp::~TcpGenericServerApp()
{
    // socketMap and socketQueue are destroyed before the socket member,
    // whose destructor would otherwise call back into socketDeleted()
    socket.setCallback(nullptr);
    // each delete calls back socketDeleted(), which erases the map entry,
    // so don't iterate the map while deleting (cf. SocketMap::deleteSockets)
    auto& sockets = socketMap.getMap();
    while (!sockets.empty())
        delete sockets.begin()->second;
}

void TcpGenericServerApp::finish()
{
    EV_INFO << getFullPath() << ": sent " << bytesSent << " bytes in " << msgsSent << " packets\n";
    EV_INFO << getFullPath() << ": received " << bytesRcvd << " bytes in " << msgsRcvd << " packets\n";
}

void TcpGenericServerApp::sendOrScheduleReadCommandIfNeeded(int connId)
{
    if (!autoRead) {
        simtime_t delay = par("readDelay");
        auto request = new Request("Read", TCP_C_READ);

        TcpReadCommand *readCmd = new TcpReadCommand();
        readCmd->setMaxByteCount(par("readSize"));
        request->setControlInfo(readCmd);
        request->addTagIfAbsent<SocketReq>()->setSocketId(connId);

        if (delay >= SIMTIME_ZERO) {
            scheduleAfter(delay, request);
        }
        else
            sendBack(request);
    }
}

} // namespace inet

