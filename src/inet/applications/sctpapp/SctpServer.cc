//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2015 Thomas Dreibholz
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

#include "inet/applications/sctpapp/SctpServer.h"

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {

Define_Module(SctpServer);

void SctpServer::initialize(int stage)
{
    EV_DEBUG << "initialize SCTP Server stage " << stage << endl;

    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        WATCH(numSessions);
        WATCH(packetsSent);
        WATCH(packetsRcvd);
        WATCH(bytesSent);
        WATCH(numRequestsToSend);

        // parameters
        finishEndsSimulation = par("finishEndsSimulation");

        inboundStreams = par("inboundStreams");
        outboundStreams = par("outboundStreams");
        ordered = par("ordered");
        queueSize = par("queueSize");
        timeoutMsg = new cMessage("SrvAppTimer");
        delayTimer = new cMessage("delayTimer");
        delayTimer->setContextPointer(this);
        delayFirstReadTimer = new cMessage("delayFirstReadTimer");

        echo = par("echo");
        delay = par("echoDelay");
        delayFirstRead = par("delayFirstRead");
        cPar *delT = &par("readingInterval");
        if (delT->isNumeric() && delT->doubleValue() == 0)      //FIXME why used the isNumeric() ?
            readInt = false;
        else
            readInt = true;
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        const char *addressesString = par("localAddress");
        AddressVector addresses = L3AddressResolver().resolve(cStringTokenizer(addressesString).asVector());
        int port = par("localPort");
        int messagesToPush = par("messagesToPush");

        socket = new SctpSocket();
        socket->setOutputGate(gate("socketOut"));
        socket->setInboundStreams(inboundStreams);
        socket->setOutboundStreams(outboundStreams);

        if (addresses.size() == 0)
            socket->bind(port);
        else
            socket->bindx(addresses, port);

        socket->listen(true, par("streamReset"), par("numPacketsToSendPerClient"), messagesToPush);
        EV_INFO << "SctpServer::initialized listen port=" << port << "\n";
        cStringTokenizer tokenizer(par("streamPriorities"));
        for (unsigned int streamNum = 0; tokenizer.hasMoreTokens(); streamNum++) {
            const char *token = tokenizer.nextToken();
            socket->setStreamPriority(streamNum, (unsigned int)atoi(token));
        }

        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void SctpServer::sendOrSchedule(cMessage *msg)
{
    if (delay == 0)
        send(msg, "socketOut");
    else
        scheduleAfter(delay, msg);
}

void SctpServer::sendOrSchedule(Message *msg)
{
    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
    msg->addTagIfAbsent<SocketReq>()->setSocketId(assocId);
    if (delay == 0)
        send(msg, "socketOut");
    else
        scheduleAfter(delay, msg);
}

void SctpServer::sendOrSchedule(Packet *pkt)
{
    pkt->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
    pkt->addTagIfAbsent<SocketReq>()->setSocketId(assocId);
    if (delay == 0)
        send(pkt, "socketOut");
    else
        scheduleAfter(delay, pkt);
}

void SctpServer::generateAndSend()
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
    if (queueSize > 0 && numRequestsToSend > 0 && count < queueSize * 2)
        sctpSendReq->setLast(false);
    else
        sctpSendReq->setLast(true);
    sctpSendReq->setPrMethod(par("prMethod"));
    sctpSendReq->setPrValue(par("prValue"));
    lastStream = (lastStream + 1) % outboundStreams;
    sctpSendReq->setSid(lastStream);
    sctpSendReq->setSocketId(assocId);
    applicationPacket->setKind(ordered ? SCTP_C_SEND_ORDERED : SCTP_C_SEND_UNORDERED);
    applicationPacket->addTag<SocketReq>()->setSocketId(assocId);
    applicationPacket->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
    bytesSent += numBytes;
    packetsSent++;
    sendOrSchedule(applicationPacket);
}

Message *SctpServer::makeReceiveRequest(cMessage *msg)
{
    Message *message = check_and_cast<Message *>(msg);
    auto& intags = getTags(message);
    SctpCommandReq *ind = intags.findTag<SctpCommandReq>();
    Request *cmsg = new Request("ReceiveRequest", SCTP_C_RECEIVE);
    auto cmd = cmsg->addTag<SctpSendReq>();
    cmd->setSocketId(ind->getSocketId());
    cmd->setSid(ind->getSid());
    cmd->setNumMsgs(ind->getNumMsgs());
    return cmsg;
}

Message *SctpServer::makeDefaultReceive()
{
    Request *cmsg = new Request("DefaultReceive", SCTP_C_RECEIVE);
    SctpCommandReq *cmd = cmsg->addTag<SctpCommandReq>();
    cmd->setSocketId(assocId);
    cmd->setSid(0);
    cmd->setNumMsgs(1);
    return cmsg;
}

Message *SctpServer::makeAbortNotification(SctpCommandReq *msg)
{
    Request *cmsg = new Request("SCTP_C_ABORT", SCTP_C_ABORT);
    SctpSendReq *cmd = cmsg->addTag<SctpSendReq>();
    assocId = msg->getSocketId();
    cmd->setSocketId(assocId);
    cmd->setSid(msg->getSid());
    cmd->setNumMsgs(msg->getNumMsgs());
    return cmsg;
}

void SctpServer::handleMessage(cMessage *msg)
{
    // TODO: there is another memory leak somewhere...
    int id = 0;
    EV_INFO << "SctpServer::handleMessage\n";
    if (msg->isSelfMessage())
        handleTimer(msg);
    else {
        switch (msg->getKind()) {
            case SCTP_I_PEER_CLOSED:
            case SCTP_I_ABORT: {
                Message *message = check_and_cast<Message *>(msg);
                assocId = message->getTag<SocketInd>()->getSocketId();
                SctpCommandReq *command = message->findTag<SctpCommandReq>();
                serverAssocStatMap[assocId].peerClosed = true;
                if (par("numPacketsToReceivePerClient").intValue() == 0) {
                    if (serverAssocStatMap[assocId].abortSent == false) {
                        sendOrSchedule(makeAbortNotification(command->dup()));
                        serverAssocStatMap[assocId].abortSent = true;
                    }
                }
                else {
                    if (serverAssocStatMap[assocId].rcvdPackets == static_cast<unsigned long int>(par("numPacketsToReceivePerClient"))
                        && serverAssocStatMap[assocId].abortSent == false)
                    {
                        sendOrSchedule(makeAbortNotification(command->dup()));
                        serverAssocStatMap[assocId].abortSent = true;
                    }
                }
                if (delayTimer->isScheduled())
                    cancelEvent(delayTimer);
                if (delayFirstReadTimer->isScheduled())
                    cancelEvent(delayFirstReadTimer);
                delete msg;
                break;
            }
            case SCTP_I_AVAILABLE: {
                EV_INFO << "SCTP_I_AVAILABLE arrived at server\n";
                Message *message = check_and_cast<Message *>(msg);
                int newSockId = message->getTag<SctpAvailableReq>()->getNewSocketId();
                EV_INFO << "new socket id = " << newSockId << endl;
                Request *cmsg = new Request("SCTP_C_ACCEPT_SOCKET_ID", SCTP_C_ACCEPT_SOCKET_ID);
                cmsg->addTag<SctpAvailableReq>()->setSocketId(newSockId);
                cmsg->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
                cmsg->addTag<SocketReq>()->setSocketId(newSockId);
                EV_INFO << "Sending accept socket id request ..." << endl;
                delete msg;
                send(cmsg, "socketOut");
                break;
            }
            case SCTP_I_ESTABLISHED: {
                count = 0;
                Message *message = check_and_cast<Message *>(msg);
                SctpConnectReq *connectInfo = message->getTag<SctpConnectReq>();
                numSessions++;
                assocId = connectInfo->getSocketId();
                inboundStreams = connectInfo->getInboundStreams();
                outboundStreams = connectInfo->getOutboundStreams();
                serverAssocStatMap[assocId].rcvdPackets = par("numPacketsToReceivePerClient");
                serverAssocStatMap[assocId].sentPackets = par("numPacketsToSendPerClient");
                serverAssocStatMap[assocId].rcvdBytes = 0;
                serverAssocStatMap[assocId].start = 0;
                serverAssocStatMap[assocId].stop = 0;
                serverAssocStatMap[assocId].lifeTime = 0;
                serverAssocStatMap[assocId].abortSent = false;
                serverAssocStatMap[assocId].peerClosed = false;
                char text[50];
                sprintf(text, "App: Received Bytes of assoc %d", assocId);
                bytesPerAssoc[assocId] = new cOutVector(text);
                sprintf(text, "App: EndToEndDelay of assoc %d", assocId);
                endToEndDelay[assocId] = new cOutVector(text);

                delete msg;
                if (par("numPacketsToSendPerClient").intValue() > 0) {
                    auto i = serverAssocStatMap.find(assocId);
                    numRequestsToSend = i->second.sentPackets;
                    if (par("thinkTime").doubleValue() > 0) {
                        generateAndSend();
                        timeoutMsg->setKind(SCTP_C_SEND);
                        scheduleAfter(par("thinkTime"), timeoutMsg);
                        numRequestsToSend--;
                        i->second.sentPackets = numRequestsToSend;
                    }
                    else {
                        if (queueSize == 0) {
                            while (numRequestsToSend > 0) {
                                generateAndSend();
                                numRequestsToSend--;
                                i->second.sentPackets = numRequestsToSend;
                            }
                        }
                        else if (queueSize > 0) {
                            while (numRequestsToSend > 0 && count++ < queueSize * 2) {
                                generateAndSend();
                                numRequestsToSend--;
                                i->second.sentPackets = numRequestsToSend;
                            }

                            Request *cmsg = new Request("SCTP_C_QUEUE_MSGS_LIMIT", SCTP_C_QUEUE_MSGS_LIMIT);
                            SctpInfoReq *qinfo = cmsg->addTag<SctpInfoReq>();
                            qinfo->setText(queueSize);
                            qinfo->setSocketId(id);
                            sendOrSchedule(cmsg);
                        }
                        auto j = serverAssocStatMap.find(assocId);
                        if (j->second.rcvdPackets == 0 && par("waitToClose").doubleValue() > 0) {
                            char as[5];
                            sprintf(as, "%d", assocId);
                            cMessage *abortMsg = new cMessage(as, SCTP_I_ABORT);
                            scheduleAfter(par("waitToClose"), abortMsg);
                        }
                        else {
                            EV_INFO << "no more packets to send, call shutdown for assoc " << assocId << "\n";
                            Request *cmsg = new Request("ShutdownRequest", SCTP_C_SHUTDOWN);
                            SctpCommandReq *cmd = cmsg->addTag<SctpCommandReq>();
                            cmd->setSocketId(assocId);
                            sendOrSchedule(cmsg);
                        }
                    }
                }
                break;
            }

            case SCTP_I_DATA_NOTIFICATION: {
                notificationsReceived++;
                Message *cmsg;
                if (schedule == false) {
                    if (delayFirstRead > 0 && !delayFirstReadTimer->isScheduled()) {
                        cmsg = makeReceiveRequest(msg);
                        scheduleAfter(delayFirstRead, cmsg);
                        scheduleAfter(delayFirstRead, delayFirstReadTimer);
                    }
                    else if (readInt && firstData) {
                        firstData = false;
                        cmsg = makeReceiveRequest(msg);
                        scheduleAfter(par("readingInterval"), delayTimer);
                        sendOrSchedule(cmsg);
                    }
                    else if (delayFirstRead == 0 && readInt == false) {
                        cmsg = makeReceiveRequest(msg);
                        sendOrSchedule(cmsg);
                    }
                }
                else {
                    EV_INFO << simTime() << " makeReceiveRequest\n";
                    cmsg = makeReceiveRequest(msg);
                    sendOrSchedule(cmsg);
                }
                delete msg;
                break;
            }

            case SCTP_I_DATA: {
                notificationsReceived--;
                packetsRcvd++;
                EV_INFO << simTime() << " server: data arrived. " << packetsRcvd << " Packets received now\n";
                Packet *message = check_and_cast<Packet *>(msg);
                auto& tags = getTags(message);
                SctpRcvReq *ind = tags.findTag<SctpRcvReq>();
                id = ind->getSocketId();
                auto j = serverAssocStatMap.find(id);
                auto k = bytesPerAssoc.find(id);
                if (j->second.rcvdBytes == 0)
                    j->second.start = simTime();

                j->second.rcvdBytes += PK(msg)->getByteLength();
                k->second->record(j->second.rcvdBytes);

                if (!echo) {
                    if (par("numPacketsToReceivePerClient").intValue() > 0) {
                        j->second.rcvdPackets--;

                        auto m = endToEndDelay.find(id);
                        for (auto& region : message->peekData()->getAllTags<CreationTimeTag>())
                            m->second->record(simTime() - region.getTag()->getCreationTime());

                        EV_INFO << "server: Data received. Left packets to receive=" << j->second.rcvdPackets << "\n";

                        if (j->second.rcvdPackets == 0) {
                            if (serverAssocStatMap[assocId].peerClosed == true && serverAssocStatMap[assocId].abortSent == false) {
                                sendOrSchedule(makeAbortNotification(ind));
                                serverAssocStatMap[assocId].abortSent = true;
                                j->second.stop = simTime();
                                j->second.lifeTime = j->second.stop - j->second.start;
                                break;
                            }
                            else {
                                Request *cmsg = new Request("SCTP_C_NO_OUTSTANDING", SCTP_C_NO_OUTSTANDING);
                                SctpCommandReq *qinfo = cmsg->addTag<SctpCommandReq>();
                                qinfo->setSocketId(id);
                                sendOrSchedule(cmsg);
                                j->second.stop = simTime();
                                j->second.lifeTime = j->second.stop - j->second.start;
                            }
                        }
                    }
                }
                else {
                    auto m = endToEndDelay.find(id);
                    const auto& smsg = message->peekData();

                    for (auto& region : smsg->getAllTags<CreationTimeTag>())
                        m->second->record(simTime() - region.getTag()->getCreationTime());

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
                delete msg;
                break;
            }

            case SCTP_I_SHUTDOWN_RECEIVED: {
                Message *message = check_and_cast<Message *>(msg);
                id = message->getTag<SocketInd>()->getSocketId();
                EV_INFO << "server: SCTP_I_SHUTDOWN_RECEIVED for assoc " << id << "\n";
                auto i = serverAssocStatMap.find(id);
                if (i->second.sentPackets == 0 || par("numPacketsToSendPerClient").intValue() == 0) {
                    Request *cmsg = new Request("SCTP_C_NO_OUTSTANDING", SCTP_C_NO_OUTSTANDING);
                    SctpCommandReq *qinfo = cmsg->addTag<SctpCommandReq>();
                    qinfo->setSocketId(id);
                    sendOrSchedule(cmsg);
                    i->second.stop = simTime();
                    i->second.lifeTime = i->second.stop - i->second.start;
                }
                shutdownReceived = true;
                delete msg;
                break;
            }

            case SCTP_I_SEND_STREAMS_RESETTED:
            case SCTP_I_RCV_STREAMS_RESETTED: {
                EV_INFO << "Streams have been resetted\n";
                delete msg;
                break;
            }

            case SCTP_I_CLOSED: {
                Message *message = check_and_cast<Message *>(msg);
                id = message->getTag<SocketInd>()->getSocketId();
                EV_INFO << "server: SCTP_I_CLOSED for assoc "  << id << endl;
                ServerAssocStatMap::iterator i = serverAssocStatMap.find(id);
                i->second.stop = simTime();
                i->second.lifeTime = i->second.stop - i->second.start;
                if (delayTimer->isScheduled())
                    cancelEvent(delayTimer);
                if (finishEndsSimulation)
                    endSimulation();
                delete msg;
                break;
            }

            default:
                delete msg;
                break;
        }
    }
}

void SctpServer::handleTimer(cMessage *msg)
{
    if (msg == delayTimer) {
        if (delayFirstRead == 0) {
            sendOrSchedule(makeDefaultReceive());
            scheduleAfter(par("readingInterval"), delayTimer);
        }
        return;
    }
    else if (msg == delayFirstReadTimer) {
        delayFirstRead = 0;
        if (readInt && !delayTimer->isScheduled()) {
            simtime_t tempInterval = par("readingInterval");
            scheduleAfter(tempInterval, delayTimer);
            scheduleAfter(tempInterval, makeDefaultReceive());
        }
        return;
    }

    switch (msg->getKind()) {
        case SCTP_C_SEND:
            if (numRequestsToSend > 0) {
                generateAndSend();
                if (par("thinkTime").doubleValue() > 0)
                    scheduleAfter(par("thinkTime"), timeoutMsg);
                numRequestsToSend--;
            }
            break;

        case SCTP_I_ABORT: {
            Request *cmsg = new Request("SCTP_C_CLOSE", SCTP_C_CLOSE);
            SctpCommandReq *cmd = cmsg->addTag<SctpCommandReq>();
            int id = atoi(msg->getName());
            cmd->setSocketId(id);
            sendOrSchedule(cmsg);
        }
        break;

        case SCTP_C_RECEIVE:
            EV_INFO << simTime() << " SctpServer:SCTP_C_RECEIVE\n";
            if (readInt || delayFirstRead > 0)
                schedule = false;
            else
                schedule = true;
            sendOrSchedule(msg);
            break;

        default:
            EV_INFO << "MsgKind =" << msg->getKind() << " unknown\n";
            break;
    }
}

void SctpServer::finish()
{
    EV_INFO << getFullPath() << ": opened " << numSessions << " sessions\n";
    EV_INFO << getFullPath() << ": sent " << bytesSent << " bytes in " << packetsSent << " packets\n";
    for (auto & elem : serverAssocStatMap) {
        EV_DETAIL << getFullPath() << " Assoc: " << elem.first << "\n";
        EV_DETAIL << "\tstart time: " << elem.second.start << "\n";
        EV_DETAIL << "\tstop time: " << elem.second.stop << "\n";
        EV_DETAIL << "\tlife time: " << elem.second.lifeTime << "\n";
        EV_DETAIL << "\treceived bytes:" << elem.second.rcvdBytes << "\n";
        EV_DETAIL << "\tthroughput: " << (elem.second.rcvdBytes / elem.second.lifeTime.dbl()) * 8 << " bit/sec\n";
        recordScalar("bytes rcvd", elem.second.rcvdBytes);
        recordScalar("throughput", (elem.second.rcvdBytes / elem.second.lifeTime.dbl()) * 8);
    }
    EV_INFO << getFullPath() << "Over all " << packetsRcvd << " packets received\n ";
    EV_INFO << getFullPath() << "Over all " << notificationsReceived << " notifications received\n ";
    EV_INFO << "Server finished\n";
}

SctpServer::~SctpServer()
{
    for (auto & elem : bytesPerAssoc)
        delete elem.second;

    for (auto & elem : endToEndDelay)
        delete elem.second;

    bytesPerAssoc.clear();
    endToEndDelay.clear();
    serverAssocStatMap.clear();
    delete socket;
    cancelAndDelete(timeoutMsg);
    cancelAndDelete(delayTimer);
    cancelAndDelete(delayFirstReadTimer);
}

SctpServer::SctpServer()
{
    timeoutMsg = nullptr;
    socket = nullptr;
    delayFirstReadTimer = nullptr;
    delayTimer = nullptr;
    numSessions = 0;
    packetsSent = 0;
    packetsRcvd = 0;
    bytesSent = 0;
    notificationsReceived = 0;
    inboundStreams = 17;
    outboundStreams = 1;
    queueSize = 0;
    delay = 0;
    delayFirstRead = 0;
    finishEndsSimulation = true;
    echo = false;
    ordered = true;
    lastStream = 0;
    assocId = 0;
    readInt = false;
    schedule = false;
    firstData = true;
    shutdownReceived = false;
    abortSent = false;
    count = 0;
    numRequestsToSend = 0;
}

} // namespace inet

