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
    Packet *packet = dynamic_cast<Packet *>(msg);

    if (packet) {
        msgsSent++;
        bytesSent += packet->getByteLength();
        emit(packetSentSignal, packet);

        EV_INFO << "sending \"" << packet->getName() << "\" to TCP, " << packet->getByteLength() << " bytes\n";
    }
    else {
        EV_INFO << "sending \"" << msg->getName() << "\" to TCP\n";
    }

    auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
    tags.addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::tcp);
    send(msg, "socketOut");
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
        else {
            // send read message to TCP
            request->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::tcp);
            EV_INFO << "sending \"" << request->getName() << "\" to TCP\n";
            send(request, "socketOut");
        }
    }
}

} // namespace inet

