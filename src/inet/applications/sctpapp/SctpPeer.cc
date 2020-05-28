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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <stdlib.h>
#include <stdio.h>

#include "inet/applications/sctpapp/SctpPeer.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {

#define MSGKIND_CONNECT    0
#define MSGKIND_SEND       1
#define MSGKIND_ABORT      2
#define MSGKIND_PRIMARY    3
#define MSGKIND_RESET      4
#define MSGKIND_STOP       5

Define_Module(SctpPeer);

simsignal_t SctpPeer::echoedPkSignal = registerSignal("echoedPk");

SctpPeer::SctpPeer()
{
    timeoutMsg = nullptr;
    timeMsg = nullptr;
    connectTimer = nullptr;
    delay = 0;
    echo = false;
    ordered = true;
    schedule = false;
    queueSize = 0;
    outboundStreams = 1;
    inboundStreams = 17;
    shutdownReceived = false;
    sendAllowed = true;
    serverAssocId = 0;
    numRequestsToSend = 0;
    lastStream = 0;
    numPacketsToReceive = 0;
    bytesSent = 0;
    echoedBytesSent = 0;
    packetsSent = 0;
    bytesRcvd = 0;
    packetsRcvd = 0;
    notificationsReceived = 0;
    numSessions = 0;
    chunksAbandoned = 0;
}

SctpPeer::~SctpPeer()
{
    cancelAndDelete(timeMsg);
    cancelAndDelete(timeoutMsg);
    cancelAndDelete(connectTimer);
    for (auto & elem : bytesPerAssoc)
        delete elem.second;
    bytesPerAssoc.clear();

    for (auto & elem : endToEndDelay)
        delete elem.second;
    endToEndDelay.clear();

    for (auto & elem : histEndToEndDelay)
        delete elem.second;
    histEndToEndDelay.clear();

    rcvdPacketsPerAssoc.clear();
    sentPacketsPerAssoc.clear();
    rcvdBytesPerAssoc.clear();
}

void SctpPeer::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        WATCH(numSessions);
        WATCH(packetsSent);
        WATCH(packetsRcvd);
        WATCH(bytesSent);
        WATCH(numRequestsToSend);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        // parameters
        const char *addressesString = par("localAddress");
        AddressVector addresses = L3AddressResolver().resolve(cStringTokenizer(addressesString).asVector());
        int port = par("localPort");
        echo = par("echo");
        delay = par("echoDelay");
        outboundStreams = par("outboundStreams");
        inboundStreams = par("inboundStreams");
        ordered = par("ordered");
        queueSize = par("queueSize");
        timeoutMsg = new cMessage("SrvAppTimer");
        listeningSocket.setOutputGate(gate("socketOut"));
        listeningSocket.setOutboundStreams(outboundStreams);
        listeningSocket.setInboundStreams(inboundStreams);

        if (addresses.size() == 0) {
            listeningSocket.bind(port);
            clientSocket.bind(port);
        }
        else {
            listeningSocket.bindx(addresses, port);
            clientSocket.bindx(addresses, port);
        }
        listeningSocket.listen(true, par("streamReset"), par("numPacketsToSendPerClient"));
        EV_DEBUG << "SctpPeer::initialized listen port=" << port << "\n";
        clientSocket.setCallback(this);
        clientSocket.setOutputGate(gate("socketOut"));

        if (simtime_t(par("startTime")) > SIMTIME_ZERO) {    //FIXME is invalid the startTime == 0 ????
            connectTimer = new cMessage("ConnectTimer", MSGKIND_CONNECT);
            scheduleAt(par("startTime"), connectTimer);
        }

        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void SctpPeer::sendOrSchedule(cMessage *msg)
{
    if (delay == 0) {
        send(msg, "socketOut");
    }
    else {
        scheduleAt(simTime() + delay, msg);
    }
}

void SctpPeer::generateAndSend()
{
    auto applicationPacket = new Packet("ApplicationPacket");
    auto applicationData = makeShared<BytesChunk>();
    int numBytes = par("requestLength");
    std::vector<uint8_t> vec;
    vec.resize(numBytes);
    for (int i = 0; i < numBytes; i++)
        vec[i] = (bytesSent + i) & 0xFF;
    applicationData->setBytes(vec);
    applicationData->addTag<CreationTimeTag>()->setCreationTime(simTime());
    applicationPacket->insertAtBack(applicationData);
    auto sctpSendReq = applicationPacket->addTag<SctpSendReq>();
    sctpSendReq->setLast(true);
    sctpSendReq->setPrMethod(par("prMethod"));
    sctpSendReq->setPrValue(par("prValue"));
    lastStream = (lastStream + 1) % outboundStreams;
    sctpSendReq->setSid(lastStream);
    sctpSendReq->setSocketId(serverAssocId);
    applicationPacket->setKind(ordered ? SCTP_C_SEND_ORDERED : SCTP_C_SEND_UNORDERED);
    applicationPacket->addTag<SocketReq>()->setSocketId(serverAssocId);
    applicationPacket->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
    bytesSent += numBytes;
    packetsSent++;
    sendOrSchedule(applicationPacket);
}

void SctpPeer::connect()
{
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");
    int outStreams = par("outboundStreams");
    clientSocket.setOutboundStreams(outStreams);

    EV_INFO << "issuing OPEN command\n";
    EV_INFO << "Assoc " << clientSocket.getSocketId() << "::connect to address " << connectAddress << ", port " << connectPort << "\n";
    numSessions++;
    bool streamReset = par("streamReset");
    L3Address destination;
    L3AddressResolver().tryResolve(connectAddress, destination);
    if (destination.isUnspecified())
        EV << "cannot resolve destination address: " << connectAddress << endl;
    else {
        clientSocket.connect(destination, connectPort, streamReset, par("prMethod"), par("numRequestsPerSession"));
    }

    if (streamReset) {
        cMessage *cmsg = new cMessage("StreamReset", MSGKIND_RESET);
        EV_INFO << "StreamReset Timer scheduled at " << simTime() << "\n";
        scheduleAt(simTime() + par("streamRequestTime"), cmsg);
    }

    unsigned int streamNum = 0;
    cStringTokenizer tokenizer(par("streamPriorities"));
    while (tokenizer.hasMoreTokens()) {
        const char *token = tokenizer.nextToken();
        clientSocket.setStreamPriority(streamNum, (unsigned int)atoi(token));

        streamNum++;
    }
}

void SctpPeer::handleMessage(cMessage *msg)
{
    int id = -1;

    if (msg->isSelfMessage())
        handleTimer(msg);

    switch (msg->getKind()) {
        case SCTP_I_PEER_CLOSED:
        case SCTP_I_ABORT: {
            Message *message = check_and_cast<Message *>(msg);
            auto& intags = getTags(message);
            const auto& ind = intags.findTag<SctpCommandReq>();
            Request *cmsg = new Request("SCTP_C_ABORT", SCTP_C_ABORT);
            auto& cmd = cmsg->addTag<SctpSendReq>();
            id = ind->getSocketId();
            cmd->setSocketId(id);
            cmd->setSid(ind->getSid());
            cmd->setNumMsgs(ind->getNumMsgs());
            delete msg;
            sendOrSchedule(cmsg);
            break;
        }

        case SCTP_I_ESTABLISHED: {
            if (clientSocket.getState() == SctpSocket::CONNECTING)
                clientSocket.processMessage(PK(msg));
            else {
                Message *message = check_and_cast<Message *>(msg);
                auto& tags = getTags(message);
                const auto& connectInfo = tags.findTag<SctpConnectReq>();
                numSessions++;
                serverAssocId = connectInfo->getSocketId();
                id = serverAssocId;
                outboundStreams = connectInfo->getOutboundStreams();
                rcvdPacketsPerAssoc[serverAssocId] = par("numPacketsToReceivePerClient");
                sentPacketsPerAssoc[serverAssocId] = par("numPacketsToSendPerClient");
                char text[50];
                sprintf(text, "App: Received Bytes of assoc %d", serverAssocId);
                bytesPerAssoc[serverAssocId] = new cOutVector(text);
                rcvdBytesPerAssoc[serverAssocId] = 0;
                sprintf(text, "App: EndToEndDelay of assoc %d", serverAssocId);
                endToEndDelay[serverAssocId] = new cOutVector(text);
                sprintf(text, "Hist: EndToEndDelay of assoc %d", serverAssocId);
                histEndToEndDelay[serverAssocId] = new cHistogram(text);

                //delete connectInfo;
                delete msg;

                if (par("numPacketsToSendPerClient").intValue() > 0) {
                    auto i = sentPacketsPerAssoc.find(serverAssocId);
                    numRequestsToSend = i->second;
                    if (par("thinkTime").doubleValue() > 0) {
                        generateAndSend();
                        timeoutMsg->setKind(SCTP_C_SEND);
                        scheduleAt(simTime() + par("thinkTime"), timeoutMsg);
                        numRequestsToSend--;
                        i->second = numRequestsToSend;
                    }
                    else {
                        if (queueSize == 0) {
                            while (numRequestsToSend > 0) {
                                generateAndSend();
                                numRequestsToSend--;
                                i->second = numRequestsToSend;
                            }
                        }
                        else if (queueSize > 0) {
                            int count = 0;
                            while (numRequestsToSend > 0 && count++ < queueSize * 2) {
                                generateAndSend();
                                numRequestsToSend--;
                                i->second = numRequestsToSend;
                            }

                            Request *cmsg = new Request("SCTP_C_QUEUE_MSGS_LIMIT", SCTP_C_QUEUE_MSGS_LIMIT);
                            auto& qinfo = cmsg->addTag<SctpInfoReq>();
                            qinfo->setText(queueSize);
                            qinfo->setSocketId(id);
                            sendOrSchedule(cmsg);
                        }

                        EV_INFO << "!!!!!!!!!!!!!!!All data sent from Server !!!!!!!!!!\n";

                        auto j = rcvdPacketsPerAssoc.find(serverAssocId);
                        if (j->second == 0 && par("waitToClose").doubleValue() > 0) {
                            char as[5];
                            sprintf(as, "%d", serverAssocId);
                            cMessage *abortMsg = new cMessage(as, SCTP_I_ABORT);
                            scheduleAt(simTime() + par("waitToClose"), abortMsg);
                        }
                        else {
                            EV_INFO << "no more packets to send, call shutdown for assoc " << serverAssocId << "\n";
                            Request *cmsg = new Request("ShutdownRequest", SCTP_C_SHUTDOWN);
                            auto& cmd = cmsg->addTag<SctpCommandReq>();
                            cmd->setSocketId(serverAssocId);
                            sendOrSchedule(cmsg);
                        }
                    }
                }
            }
            break;
        }

        case SCTP_I_DATA_NOTIFICATION: {
            notificationsReceived++;
            Message *message = check_and_cast<Message *>(msg);
            auto& intags = getTags(message);
            const auto& ind = intags.findTag<SctpCommandReq>();
            id = ind->getSocketId();
            Request *cmsg = new Request("ReceiveRequest", SCTP_C_RECEIVE);
            auto cmd = cmsg->addTag<SctpSendReq>();
            cmsg->addTag<SocketReq>()->setSocketId(id);
            cmsg->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
            cmd->setSocketId(id);
            cmd->setSid(ind->getSid());
            cmd->setNumMsgs(ind->getNumMsgs());
            delete msg;
            if (!cmsg->isScheduled() && schedule == false) {
                scheduleAt(simTime() + par("delayFirstRead"), cmsg);
            }
            else if (schedule == true)
                sendOrSchedule(cmsg);
            break;
        }

        case SCTP_I_DATA: {
            Packet *message = check_and_cast<Packet *>(msg);
            auto& tags = getTags(message);
            const auto& ind = tags.findTag<SctpRcvReq>();
            id = ind->getSocketId();
            auto j = rcvdBytesPerAssoc.find(id);
            if (j == rcvdBytesPerAssoc.end() && (clientSocket.getState() == SctpSocket::CONNECTED))
                clientSocket.processMessage(msg);
            else if (j != rcvdBytesPerAssoc.end()) {
                j->second += PK(msg)->getByteLength();
                auto k = bytesPerAssoc.find(id);
                k->second->record(j->second);
                packetsRcvd++;

                if (!echo) {
                    if (par("numPacketsToReceivePerClient").intValue() > 0) {
                        auto i = rcvdPacketsPerAssoc.find(id);
                        i->second--;
                        SctpSimpleMessage *smsg = check_and_cast<SctpSimpleMessage *>(msg);
                        auto j = endToEndDelay.find(id);
                        j->second->record(simTime() - smsg->getCreationTime());
                        auto k = histEndToEndDelay.find(id);
                        k->second->collect(simTime() - smsg->getCreationTime());

                        if (i->second == 0) {
                            Request *cmsg = new Request("SCTP_C_NO_OUTSTANDING", SCTP_C_NO_OUTSTANDING);
                            auto& qinfo = cmsg->addTag<SctpCommandReq>();
                            qinfo->setSocketId(id);
                            sendOrSchedule(cmsg);
                        }
                    }
                    delete msg;
                }
                else {
                    auto m = endToEndDelay.find(id);
                    auto k = histEndToEndDelay.find(id);
                    const auto& smsg = message->peekData();

                    for (auto& region : smsg->getAllTags<CreationTimeTag>()) {
                        m->second->record(simTime() - region.getTag()->getCreationTime());
                        k->second->collect(simTime() - region.getTag()->getCreationTime());
                    }

                    auto cmsg = new Packet("ApplicationPacket");
                    cmsg->insertAtBack(smsg);
                    auto cmd = cmsg->addTag<SctpSendReq>();
                    lastStream = (lastStream + 1) % outboundStreams;
                    cmd->setLast(true);
                    cmd->setSocketId(id);
                    cmd->setPrValue(0);
                    cmd->setSid(lastStream);
                    cmsg->setKind(cmd->getSendUnordered() ? SCTP_C_SEND_UNORDERED : SCTP_C_SEND_ORDERED);
                    bytesSent += B(smsg->getChunkLength()).get();
                    packetsSent++;
                    sendOrSchedule(cmsg);
                }
            } else {
                delete msg;
            }
            break;
        }

        case SCTP_I_SHUTDOWN_RECEIVED: {
            Message *message = check_and_cast<Message *>(msg);
            id = message->getTag<SocketInd>()->getSocketId();
            EV_INFO << "server: SCTP_I_SHUTDOWN_RECEIVED for assoc " << id << "\n";
            auto i = rcvdPacketsPerAssoc.find(id);

            if (i == rcvdPacketsPerAssoc.end() && (clientSocket.getState() == SctpSocket::CONNECTED))
                clientSocket.processMessage(PK(msg));
            else if (i != rcvdPacketsPerAssoc.end()) {
                if (i->second == 0) {
                    Request *cmsg = new Request("SCTP_C_NO_OUTSTANDING", SCTP_C_NO_OUTSTANDING);
                    auto& qinfo = cmsg->addTag<SctpCommandReq>();
                    qinfo->setSocketId(id);
                    sendOrSchedule(cmsg);
                }

                shutdownReceived = true;
            }
            delete msg;
            break;
        }

        case SCTP_I_SEND_STREAMS_RESETTED:
        case SCTP_I_RCV_STREAMS_RESETTED: {
            EV_INFO << "Streams have been resetted\n";
            break;
        }

        case SCTP_I_CLOSED:
            delete msg;
            break;
    }

    if (hasGUI()) {
        char buf[32];
        auto l = rcvdBytesPerAssoc.find(id);
        sprintf(buf, "rcvd: %ld bytes\nsent: %ld bytes", l->second, bytesSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void SctpPeer::handleTimer(cMessage *msg)
{
    EV_TRACE << "SctpPeer::handleTimer\n";

    switch (msg->getKind()) {
        case MSGKIND_CONNECT:
            EV_INFO << "starting session call connect\n";
            connect();
            break;

        case SCTP_C_SEND:
            if (numRequestsToSend > 0) {
                generateAndSend();
                if (par("thinkTime").doubleValue() > 0)
                    scheduleAt(simTime() + par("thinkTime"), timeoutMsg);
                numRequestsToSend--;
            }
            break;

        case SCTP_I_ABORT: {
            Request *cmsg = new Request("SCTP_C_CLOSE", SCTP_C_CLOSE);
            auto& cmd = cmsg->addTag<SctpCommandReq>();
            int id = atoi(msg->getName());
            cmd->setSocketId(id);
            sendOrSchedule(cmsg);
        }
        break;

        case SCTP_C_RECEIVE:
            schedule = true;
            sendOrSchedule(PK(msg));
            break;

        default:
            break;
    }
}

void SctpPeer::socketDataNotificationArrived(SctpSocket *socket, Message *msg)
{
    Message *message = check_and_cast<Message *>(msg);
    auto& intags = getTags(message);
    const auto& ind = intags.findTag<SctpCommandReq>();
    Request *cmesg = new Request("SCTP_C_RECEIVE", SCTP_C_RECEIVE);
    auto& cmd = cmesg->addTag<SctpSendReq>();
    cmd->setSocketId(ind->getSocketId());
    cmd->setSid(ind->getSid());
    cmd->setNumMsgs(ind->getNumMsgs());
    clientSocket.sendNotification(cmesg);
}

void SctpPeer::socketPeerClosed(SctpSocket *socket)
{
    // close the connection (if not already closed)
    if (clientSocket.getState() == SctpSocket::PEER_CLOSED) {
        EV_INFO << "remote SCTP closed, closing here as well\n";
        setStatusString("closing");
        clientSocket.close();
    }
}

void SctpPeer::socketClosed(SctpSocket *socket)
{
    // *redefine* to start another session etc.
    EV_INFO << "connection closed\n";
    setStatusString("closed");
}

void SctpPeer::socketFailure(SctpSocket *socket, int code)
{
    // subclasses may override this function, and add code try to reconnect after a delay.
    EV_WARN << "connection broken\n";
    setStatusString("broken");
    // reconnect after a delay
    timeMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime() + par("reconnectInterval"), timeMsg);
}

void SctpPeer::socketStatusArrived(SctpSocket *socket, SctpStatusReq *status)
{
    struct PathStatus ps;
    auto i = sctpPathStatus.find(status->getPathId());

    if (i != sctpPathStatus.end()) {
        ps = i->second;
        ps.active = status->getActive();
    }
    else {
        ps.active = status->getActive();
        ps.primaryPath = false;
        sctpPathStatus[ps.pid] = ps;
    }
}

void SctpPeer::setStatusString(const char *s)
{
    if (hasGUI())
        getDisplayString().setTagArg("t", 0, s);
}

void SctpPeer::sendRequest(bool last)
{
    EV_INFO << "sending request, " << numRequestsToSend - 1 << " more to go\n";
    long numBytes = par("requestLength");

    if (numBytes < 1)
        numBytes = 1;

    EV_INFO << "SctpClient: sending " << numBytes << " data bytes\n";

    auto cmsg = new Packet("ApplicationPacket");
    auto msg = makeShared<BytesChunk>();
    std::vector<uint8_t> vec;
    vec.resize(numBytes);
    for (int i = 0; i < numBytes; i++)
        vec[i] = (bytesSent + i) & 0xFF;
    msg->setBytes(vec);
    msg->addTag<CreationTimeTag>()->setCreationTime(simTime());
    cmsg->insertAtBack(msg);
    cmsg->setKind(ordered ? SCTP_C_SEND_ORDERED : SCTP_C_SEND_UNORDERED);
    auto sendCommand = cmsg->addTag<SctpSendReq>();
    sendCommand->setLast(true);
    // send SctpMessage with SctpSimpleMessage enclosed
    clientSocket.send(cmsg);
    bytesSent += numBytes;
}

void SctpPeer::socketEstablished(SctpSocket *socket, unsigned long int buffer)
{
    ASSERT(socket == &clientSocket);
    int count = 0;
    // *redefine* to perform or schedule first sending
    EV_INFO << "SctpClient: connected\n";
    setStatusString("connected");
    // determine number of requests in this session
    numRequestsToSend = par("numRequestsPerSession");
    numPacketsToReceive = par("numPacketsToReceive");

    if (numRequestsToSend < 1)
        numRequestsToSend = 0;

    // perform first request (next one will be sent when reply arrives)
    if (numRequestsToSend > 0) {
        if (par("thinkTime").doubleValue() > 0) {
            if (sendAllowed) {
                sendRequest();
                numRequestsToSend--;
            }

            timeMsg->setKind(MSGKIND_SEND);
            scheduleAt(simTime() + par("thinkTime"), timeMsg);
        }
        else {
            if (queueSize > 0) {
                while (numRequestsToSend > 0 && count++ < queueSize * 2 && sendAllowed) {
                    sendRequest(count == queueSize * 2);
                    numRequestsToSend--;
                }

                if (numRequestsToSend > 0 && sendAllowed)
                    sendQueueRequest();
            }
            else {
                while (numRequestsToSend > 0 && sendAllowed) {
                    sendRequest();
                    numRequestsToSend--;
                }
            }

            if (numPacketsToReceive == 0 && par("waitToClose").doubleValue() > 0) {
                timeMsg->setKind(MSGKIND_ABORT);
                scheduleAt(simTime() + par("waitToClose"), timeMsg);
            }

            if (numRequestsToSend == 0 && par("waitToClose").doubleValue() == 0) {
                EV_INFO << "socketEstablished:no more packets to send, call shutdown\n";
                clientSocket.shutdown();
            }
        }
    }
}

void SctpPeer::sendQueueRequest()
{
    Request *cmsg = new Request("SCTP_C_QUEUE_MSGS_LIMIT", SCTP_C_QUEUE_MSGS_LIMIT);
    auto& qinfo = cmsg->addTag<SctpInfoReq>();
    qinfo->setText(queueSize);
    qinfo->setSocketId(clientSocket.getSocketId());
    clientSocket.sendRequest(cmsg);
}

void SctpPeer::sendRequestArrived(SctpSocket *socket)
{
    ASSERT(socket == &clientSocket);
    int count = 0;

    EV_INFO << "sendRequestArrived numRequestsToSend=" << numRequestsToSend << "\n";

    while (numRequestsToSend > 0 && count++ < queueSize && sendAllowed) {
        numRequestsToSend--;
        sendRequest(count == queueSize || numRequestsToSend == 0);
        if (numRequestsToSend == 0) {
            EV_INFO << "no more packets to send, call shutdown\n";
            clientSocket.shutdown();
        }
    }
}

void SctpPeer::socketDataArrived(SctpSocket *socket, Packet *msg, bool)
{
    // *redefine* to perform or schedule next sending
    packetsRcvd++;

    EV_INFO << "Client received packet Nr " << packetsRcvd << " from SCTP\n";

    auto& tags = getTags(msg);
    const auto& ind = tags.findTag<SctpRcvReq>();

    emit(packetReceivedSignal, msg);
    bytesRcvd += msg->getByteLength();

    if (echo) {
        const auto& smsg = msg->peekData();
        auto cmsg = new Packet("ApplicationPacket");
        cmsg->setKind(ind->getSendUnordered() ? SCTP_C_SEND_UNORDERED : SCTP_C_SEND_ORDERED);
        cmsg->insertAtBack(smsg);
        auto cmd = cmsg->addTag<SctpSendReq>();
        cmd->setLast(true);
        cmd->setSocketId(ind->getSocketId());
        cmd->setPrValue(0);
        cmd->setSid(ind->getSid());
        packetsSent++;
        clientSocket.send(cmsg);
    }

    if (par("numPacketsToReceive").intValue() > 0) {
        numPacketsToReceive--;
        if (numPacketsToReceive == 0) {
            setStatusString("closing");
            clientSocket.close();
        }
    }
}

void SctpPeer::shutdownReceivedArrived(SctpSocket *socket)
{
    if (numRequestsToSend == 0) {
        Message *cmsg = new Message("SCTP_C_NO_OUTSTANDING", SCTP_C_NO_OUTSTANDING);
        auto& qinfo = cmsg->addTag<SctpCommandReq>();
        qinfo->setSocketId(socket->getSocketId());
        clientSocket.sendNotification(cmsg);
    }
}

void SctpPeer::msgAbandonedArrived(SctpSocket *socket)
{
    chunksAbandoned++;
}

void SctpPeer::sendqueueFullArrived(SctpSocket *socket)
{
    sendAllowed = false;
}

void SctpPeer::finish()
{
    EV_INFO << getFullPath() << ": opened " << numSessions << " sessions\n";
    EV_INFO << getFullPath() << ": sent " << bytesSent << " bytes in " << packetsSent << " packets\n";

    for (auto & elem : rcvdBytesPerAssoc)
        EV_DETAIL << getFullPath() << ": received " << elem.second << " bytes in assoc " << elem.first << "\n";

    EV_INFO << getFullPath() << "Over all " << packetsRcvd << " packets received\n ";
    EV_INFO << getFullPath() << "Over all " << notificationsReceived << " notifications received\n ";
}

} // namespace inet

