//
// Copyright 2008-2012 Irene Ruengeler
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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/applications/sctpapp/SctpNatPeer.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {

using namespace sctp;

#define MSGKIND_CONNECT    0
#define MSGKIND_SEND       1
#define MSGKIND_ABORT      2
#define MSGKIND_PRIMARY    3
#define MSGKIND_RESET      4
#define MSGKIND_STOP       5

Define_Module(SctpNatPeer);

SctpNatPeer::SctpNatPeer()
{
    timeMsg = nullptr;
    timeoutMsg = nullptr;
    numSessions = 0;
    packetsSent = 0;
    packetsRcvd = 0;
    bytesSent = 0;
    notifications = 0;
    serverAssocId = 0;
    delay = 0;
    echo = false;
    schedule = false;
    shutdownReceived = false;
    sendAllowed = true;
    numRequestsToSend = 0;
    ordered = true;
    queueSize = 0;
    outboundStreams = 1;
    inboundStreams = 1;
    bytesRcvd = 0;
    echoedBytesSent = 0;
    lastStream = 0;
    chunksAbandoned = 0;
    numPacketsToReceive = 1;
    rendezvous = false;
    peerPort = 0;
}

SctpNatPeer::~SctpNatPeer()
{
    cancelAndDelete(timeMsg);
    cancelAndDelete(timeoutMsg);
}

void SctpNatPeer::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    EV_DEBUG << "initialize SCTP NAT Peer stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        WATCH(numSessions);
        WATCH(packetsSent);
        WATCH(packetsRcvd);
        WATCH(bytesSent);
        WATCH(numRequestsToSend);
        timeoutMsg = new cMessage("SrvAppTimer");
        queueSize = par("queueSize");
   } else if (stage == INITSTAGE_APPLICATION_LAYER) {
        // parameters
        const char *addressesString = par("localAddress");
        AddressVector addresses = L3AddressResolver().resolve(cStringTokenizer(addressesString).asVector());
        int32 port = par("localPort");
        echo = par("echo");
        delay = par("echoDelay");
        outboundStreams = par("outboundStreams");
        inboundStreams = par("inboundStreams");
        ordered = par("ordered");
        clientSocket.setOutputGate(gate("socketOut"));
        clientSocket.setCallback(this);
        if (addresses.size() == 0) {
            clientSocket.bind(port);
        }
        else {
            clientSocket.bindx(addresses, port);
        }

        rendezvous = par("rendezvous");
        if (par("startTime").doubleValue() > 0.0) {
            cMessage *msg = new cMessage("ConnectTimer", MSGKIND_CONNECT);
            scheduleAt(par("startTime"), msg);
        }
    }
}

void SctpNatPeer::sendOrSchedule(cMessage *msg)
{
    if (delay == 0) {
        send(msg, "socketOut");
    }
    else {
        scheduleAt(simTime() + delay, msg);
    }
}

void SctpNatPeer::generateAndSend()
{
    auto applicationPacket = new Packet("ApplicationPacket");
    auto applicationData = makeShared<BytesChunk>();
    int numBytes = par("requestLength");
    EV_INFO << "Send " << numBytes << " bytes of data, bytesSent = " << bytesSent << endl;
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
    clientSocket.send(applicationPacket);
}

void SctpNatPeer::connectx(AddressVector connectAddressList, int32 connectPort)
{
    uint32 outStreams = par("outboundStreams");
    clientSocket.setOutboundStreams(outStreams);
    uint32 inStreams = par("inboundStreams");
    clientSocket.setInboundStreams(inStreams);

    EV << "issuing OPEN command\n";
    EV << "Assoc " << clientSocket.getSocketId() << "::connect to  port " << connectPort << "\n";
    bool streamReset = par("streamReset");
    clientSocket.connectx(connectAddressList, connectPort, streamReset, static_cast<uint32>(par("prMethod")), static_cast<uint32>(par("numRequestsPerSession")));
    numSessions++;

    if (!streamReset)
        streamReset = false;
    else if (streamReset == true) {
        cMessage *cmsg = new cMessage("StreamReset", MSGKIND_RESET);
        EV << "StreamReset Timer scheduled at " << simTime() << "\n";
        scheduleAt(simTime() + par("streamRequestTime").doubleValue(), cmsg);
    }
    uint32 streamNum = 0;
    cStringTokenizer tokenizer(par("streamPriorities"));
    while (tokenizer.hasMoreTokens()) {
        const char *token = tokenizer.nextToken();
        clientSocket.setStreamPriority(streamNum, static_cast<uint32>(atoi(token)));

        streamNum++;
    }
}

void SctpNatPeer::connect(L3Address connectAddress, int32 connectPort)
{
    clientSocket.setOutboundStreams(outboundStreams);
    clientSocket.setInboundStreams(inboundStreams);

    EV << "issuing OPEN command\n";
    EV << "Assoc " << clientSocket.getSocketId() << "::connect to address " << connectAddress << ", port " << connectPort << "\n";
    bool streamReset = par("streamReset");
    clientSocket.connect(connectAddress, connectPort, streamReset, static_cast<int32>(par("prMethod")), static_cast<uint32>(par("numRequestsPerSession")));
    numSessions++;

    if (!streamReset)
        streamReset = false;
    else if (streamReset == true) {
        cMessage *cmsg = new cMessage("StreamReset", MSGKIND_RESET);
        EV << "StreamReset Timer scheduled at " << simTime() << "\n";
        scheduleAt(simTime() + par("streamRequestTime").doubleValue(), cmsg);
    }
    uint32 streamNum = 0;
    cStringTokenizer tokenizer(par("streamPriorities"));
    while (tokenizer.hasMoreTokens()) {
        const char *token = tokenizer.nextToken();
        clientSocket.setStreamPriority(streamNum, static_cast<uint32>(atoi(token)));

        streamNum++;
    }
}

void SctpNatPeer::handleMessage(cMessage *msg)
{
    int32 id;

    if (msg->isSelfMessage()) {
        handleTimer(msg);
    }
    else {
        EV << "SctpNatPeer::handleMessage kind=" << SctpAssociation::indicationName(msg->getKind()) << " (" << msg->getKind() << ")\n";
        switch (msg->getKind()) {
            case SCTP_I_ADDRESS_ADDED:
                if (rendezvous)
                    clientSocket.processMessage(msg);
               // else
                delete msg;
                break;

            case SCTP_I_PEER_CLOSED:
            case SCTP_I_ABORT: {
                if (rendezvous)
                    clientSocket.processMessage(msg);
                else {
                    Message *message = check_and_cast<Message *>(msg);
                    auto& msgtags = getTags(message);
                    SctpCommandReq *command = msgtags.findTag<SctpCommandReq>();
                    Request *cmsg = new Request("SCTP_C_ABORT", SCTP_C_ABORT);
                    SctpSendReq *cmd = cmsg->addTag<SctpSendReq>();
                    int assocId = command->getSocketId();
                    cmsg->addTag<SocketReq>()->setSocketId(assocId);
                    cmsg->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
                    cmd->setSocketId(assocId);
                    cmd->setSid(command->getSid());
                    cmd->setNumMsgs(command->getNumMsgs());
                    delete msg;
                    sendOrSchedule(cmsg);
                }
                break;
            }

            case SCTP_I_ESTABLISHED: {
                if (clientSocket.getState() == SctpSocket::CONNECTING) {
                    clientSocket.processMessage(msg);
                } else {
                    int32 count = 0;
                    Message *message = check_and_cast<Message *>(msg);
                    auto& tags = getTags(message);
                    SctpConnectReq *connectInfo = tags.findTag<SctpConnectReq>();
                    numSessions++;
                    serverAssocId = connectInfo->getSocketId();
                    id = serverAssocId;
                    outboundStreams = connectInfo->getOutboundStreams();
                    inboundStreams = connectInfo->getInboundStreams();
                    rcvdPacketsPerAssoc[serverAssocId] = static_cast<int64>(par("numPacketsToReceivePerClient"));
                    sentPacketsPerAssoc[serverAssocId] = static_cast<int64>(par("numPacketsToSendPerClient"));
                    char text[128];
                    sprintf(text, "App: Received Bytes of assoc %d", serverAssocId);
                    bytesPerAssoc[serverAssocId] = new cOutVector(text);
                    rcvdBytesPerAssoc[serverAssocId] = 0;
                    sprintf(text, "App: EndToEndDelay of assoc %d", serverAssocId);
                    endToEndDelay[serverAssocId] = new cOutVector(text);
                    sprintf(text, "Hist: EndToEndDelay of assoc %d", serverAssocId);
                    histEndToEndDelay[serverAssocId] = new cHistogram(text);

                    delete msg;

                    if (static_cast<int64>(par("numPacketsToSendPerClient")) > 0) {
                        auto i = sentPacketsPerAssoc.find(serverAssocId);
                        numRequestsToSend = i->second;
                        if (par("thinkTime").doubleValue() > 0.0) {
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
                                while (numRequestsToSend > 0 && count++ < queueSize * 2) {
                                    generateAndSend();
                                    numRequestsToSend--;
                                    i->second = numRequestsToSend;
                                }

                                Request *cmsg = new Request("SCTP_C_QUEUE_MSGS_LIMIT", SCTP_C_QUEUE_MSGS_LIMIT);
                                SctpInfoReq *qinfo = cmsg->addTag<SctpInfoReq>();
                                qinfo->setText(queueSize);
                                qinfo->setSocketId(id);
                                sendOrSchedule(cmsg);
                            }

                            EV << "!!!!!!!!!!!!!!!All data sent from Peer !!!!!!!!!!\n";

                            auto j = rcvdPacketsPerAssoc.find(serverAssocId);
                            if (j->second == 0 && par("waitToClose").doubleValue() > 0.0) {
                                char as[5];
                                sprintf(as, "%d", serverAssocId);
                                cMessage *abortMsg = new cMessage(as, SCTP_I_ABORT);
                                scheduleAt(simTime() + par("waitToClose"), abortMsg);
                            }
                            else {
                                EV << "no more packets to send, call shutdown for assoc " << serverAssocId << "\n";
                                Request *cmsg = new Request("ShutdownRequest", SCTP_C_SHUTDOWN);
                                SctpCommandReq *cmd = cmsg->addTag<SctpCommandReq>();
                                cmd->setSocketId(serverAssocId);
                                sendOrSchedule(cmsg);
                            }
                        }
                    }
                }
                break;
            }

            case SCTP_I_DATA_NOTIFICATION: {
                EV_DETAIL << "NatPeer: SCTP_I_DATA_NOTIFICATION arrived\n";
                notifications++;
                Message *message = check_and_cast<Message *>(msg);
                auto& intags = getTags(message);
                SctpCommandReq *ind = intags.findTag<SctpCommandReq>();
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
                SctpRcvReq *ind = message->findTag<SctpRcvReq>();
                id = ind->getSocketId();
                if (rendezvous) {
                    const auto& smsg = message->peekDataAsBytes();
                    int bufferlen = B(smsg->getChunkLength()).get();
                    uint8_t buffer[bufferlen];
                    std::vector<uint8_t> vec = smsg->getBytes();
                    for (int i = 0; i < bufferlen; i++) {
                        buffer[i] = vec[i];
                    }
                    struct nat_message *nat = (struct nat_message *) buffer;
                    peerAddressList.clear();
                    if (nat->multi) {
                        peerAddressList.push_back(Ipv4Address(nat->peer2Addresses[0]));
                        EV << "address 0: " << Ipv4Address(nat->peer2Addresses[0]).str() << endl;
                        peerAddressList.push_back(Ipv4Address(nat->peer2Addresses[1]));
                        EV << "address 1: " << Ipv4Address(nat->peer2Addresses[1]).str()  << endl;
                    }
                    else {
                        peerAddress = Ipv4Address(nat->peer2Addresses[0]);
                    }
                    peerPort = nat->portPeer2;
                    delete msg;
                }
                else {
                    auto j = rcvdBytesPerAssoc.find(id);
                    if (j == rcvdBytesPerAssoc.end() && (clientSocket.getState() == SctpSocket::CONNECTED))
                        clientSocket.processMessage(PK(msg));
                    else if (j != rcvdBytesPerAssoc.end()) {
                        j->second += PK(msg)->getBitLength() / 8;
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
                                    SctpCommandReq *qinfo = cmsg->addTag<SctpCommandReq>();
                                    qinfo->setSocketId(id);
                                    sendOrSchedule(cmsg);
                                }
                            }
                            delete msg;
                        }
                        else {
                            SctpSimpleMessage *smsg = check_and_cast<SctpSimpleMessage *>(msg->dup());
                            auto j = endToEndDelay.find(id);
                            j->second->record(simTime() - smsg->getCreationTime());
                            auto k = histEndToEndDelay.find(id);
                            k->second->collect(simTime() - smsg->getCreationTime());
                            Packet *cmsg = new Packet("SCTP_C_SEND", SCTP_C_SEND);
                            bytesSent += smsg->getByteLength();
                            auto cmd = cmsg->addTag<SctpSendReq>();
                            cmd->setSendUnordered(cmd->getSendUnordered());
                            lastStream = (lastStream + 1) % outboundStreams;
                            cmd->setSocketId(id);
                            cmd->setPrValue(0);
                            cmd->setSid(lastStream);
                            cmd->setLast(true);
                            cmsg->encapsulate(smsg);
                            cmsg->setControlInfo(cmd);
                            packetsSent++;
                            delete msg;
                            sendOrSchedule(cmsg);
                        }
                    } else {
                        delete msg;
                    }
                }
                break;
            }

            case SCTP_I_SHUTDOWN_RECEIVED: {
                Message *message = check_and_cast<Message *>(msg);
                id = message->getTag<SocketInd>()->getSocketId();
                EV << "peer: SCTP_I_SHUTDOWN_RECEIVED for assoc " << id << "\n";
                auto i = rcvdPacketsPerAssoc.find(id);
                if (i == rcvdPacketsPerAssoc.end() && (clientSocket.getState() == SctpSocket::CONNECTED)) {
                    clientSocket.processMessage(msg);
                } else if (i != rcvdPacketsPerAssoc.end()) {
                    if (i->second == 0) {
                        Request *cmsg = new Request("SCTP_C_NO_OUTSTANDING", SCTP_C_NO_OUTSTANDING);
                        SctpCommandReq *qinfo = cmsg->addTag<SctpCommandReq>();
                        qinfo->setSocketId(id);
                        sendOrSchedule(cmsg);
                    }

                    shutdownReceived = true;
                    delete msg;
                } else {
                    delete msg;
                }
                delete msg;
            }

            case SCTP_I_SEND_STREAMS_RESETTED:
            case SCTP_I_RCV_STREAMS_RESETTED: {
                EV << "Streams have been resetted\n";
                break;
            }

            case SCTP_I_CLOSED:
            case SCTP_I_SENDSOCKETOPTIONS:
                delete msg;
                break;
        }
    }

    if (hasGUI()) {
        char buf[32];
        auto l = rcvdBytesPerAssoc.find(id);
        sprintf(buf, "rcvd: %lld bytes\nsent: %lld bytes", (long long int)l->second, (long long int)bytesSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void SctpNatPeer::handleTimer(cMessage *msg)
{
    int32 id;

    EV << "SctpNatPeer::handleTimer\n";

    switch (msg->getKind()) {
        case MSGKIND_CONNECT:
            EV << "starting session call connect\n";
            connect(L3AddressResolver().resolve(par("connectAddress"), 1), par("connectPort"));
            delete msg;
            break;

        case SCTP_C_SEND:

            EV << "SctpNatPeer:MSGKIND_SEND\n";

            if (numRequestsToSend > 0) {
                generateAndSend();
                if (par("thinkTime").doubleValue() > 0.0)
                    scheduleAt(simTime() + par("thinkTime"), timeoutMsg);
                numRequestsToSend--;
            }
            break;

        case SCTP_I_ABORT: {
                EV << "SctpNatPeer:MsgKIND_ABORT for assoc " << atoi(msg->getName()) << "\n";

                Request *cmsg = new Request("SCTP_C_CLOSE", SCTP_C_CLOSE);
                SctpCommandReq *cmd = cmsg->addTag<SctpCommandReq>();
                id = atoi(msg->getName());
                cmd->setSocketId(id);
                cmsg->setControlInfo(cmd);
                sendOrSchedule(cmsg);
            }
            break;

        case SCTP_C_RECEIVE:

            EV << "SctpNatPeer:SCTP_C_RECEIVE\n";
            schedule = true;
            sendOrSchedule(msg);
            break;

        default:

            EV << "MsgKind =" << msg->getKind() << " unknown\n";

            break;
    }
}

void SctpNatPeer::socketDataNotificationArrived(SctpSocket *socket, Message *msg)
{
    Message *message = check_and_cast<Message *>(msg);
    auto& intags = getTags(message);
    SctpCommandReq *ind = intags.findTag<SctpCommandReq>();
    Request *cmesg = new Request("SCTP_C_RECEIVE", SCTP_C_RECEIVE);
    SctpSendReq *cmd = cmesg->addTag<SctpSendReq>();
    cmd->setSocketId(ind->getSocketId());
    cmd->setSid(ind->getSid());
    cmd->setNumMsgs(ind->getNumMsgs());
    clientSocket.sendNotification(cmesg);
}

void SctpNatPeer::socketPeerClosed(SctpSocket *socket)
{
    // close the connection (if not already closed)
    if (clientSocket.getState() == SctpSocket::PEER_CLOSED) {
        EV << "remote Sctp closed, closing here as well\n";
        setStatusString("closed");
        clientSocket.close();
        if (rendezvous) {
            const char *addressesString = par("localAddress");
            AddressVector addresses = L3AddressResolver().resolve(cStringTokenizer(addressesString).asVector());
            int32 port = par("localPort");
            rendezvousSocket.setOutputGate(gate("socketOut"));
            rendezvousSocket.setOutboundStreams(outboundStreams);
            rendezvousSocket.setInboundStreams(inboundStreams);
            if (addresses.size() == 0) {
                rendezvousSocket.bind(port);
                clientSocket.bind(port);
            }
            else {
                clientSocket.bindx(addresses, port);
                rendezvousSocket.bindx(addresses, port);
            }
            rendezvousSocket.listen(true, par("streamReset").boolValue(), par("numPacketsToSendPerClient"));
            if (par("multi").boolValue())
                connectx(peerAddressList, peerPort);
            else
                connect(peerAddress, peerPort);
            rendezvous = false;
        }
    }
}

void SctpNatPeer::socketClosed(SctpSocket *socket)
{
    // *redefine* to start another session etc.

    EV << "connection closed\n";
    setStatusString("closed");
    clientSocket.close();
    if (rendezvous) {
        const char *addressesString = par("localAddress");
        AddressVector addresses = L3AddressResolver().resolve(cStringTokenizer(addressesString).asVector());
        int32 port = par("localPort");
        rendezvousSocket.setOutputGate(gate("socketOut"));
        rendezvousSocket.setOutboundStreams(outboundStreams);
        rendezvousSocket.setInboundStreams(inboundStreams);
        if (addresses.size() == 0) {
            rendezvousSocket.bind(port);
            clientSocket.bind(port);
        }
        else {
            clientSocket.bindx(addresses, port);
            rendezvousSocket.bindx(addresses, port);
        }
        rendezvousSocket.listen(true, par("streamReset").boolValue(), par("numPacketsToSendPerClient"));
        if (par("multi").boolValue())
            connectx(peerAddressList, peerPort);
        else
            connect(peerAddress, peerPort);
        rendezvous = false;
    }
}

void SctpNatPeer::socketFailure(SctpSocket *socket, int32 code)
{
    // subclasses may override this function, and add code try to reconnect after a delay.
    EV << "connection broken\n";
    setStatusString("broken");

    //numBroken++;

    // reconnect after a delay
    timeMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime() + par("reconnectInterval"), timeMsg);
}

void SctpNatPeer::socketStatusArrived(SctpSocket *socket, SctpStatusReq *status)
{
    struct pathStatus ps;
    auto i = sctpPathStatus.find(status->getPathId());
    if (i != sctpPathStatus.end()) {
        ps = i->second;
        ps.active = status->getActive();
    }
    else {
        ps.active = status->getActive();
        //ps.pid = status->pathId();  FIXME
        ps.primaryPath = false;
        sctpPathStatus[ps.pid] = ps;
    }
}

void SctpNatPeer::setStatusString(const char *s)
{
    if (hasGUI())
        getDisplayString().setTagArg("t", 0, s);
}

void SctpNatPeer::sendRequest(bool last)
{
    EV << "sending request, " << numRequestsToSend - 1 << " more to go\n";
    int64 numBytes = par("requestLength");
    if (numBytes < 1)
        numBytes = 1;

    EV << "SctpNatPeer: sending " << numBytes << " data bytes\n";

    auto cmsg = new Packet("ApplicationPacket");
    cmsg->setKind(ordered ? SCTP_C_SEND_ORDERED : SCTP_C_SEND_UNORDERED);
    auto msg = makeShared<BytesChunk>();
    std::vector<uint8_t> vec;
    vec.resize(numBytes);
    for (int i = 0; i < numBytes; i++)
        vec[i] = (bytesSent + i) & 0xFF;
    msg->setBytes(vec);
    msg->addTag<CreationTimeTag>()->setCreationTime(simTime());
    cmsg->insertAtBack(msg);
    auto sendCommand = cmsg->addTag<SctpSendReq>();
    sendCommand->setLast(true);
    // send SctpMessage with SctpSimpleMessage enclosed
    clientSocket.send(cmsg);
    bytesSent += numBytes;
}

void SctpNatPeer::socketEstablished(SctpSocket *socket, unsigned long int buffer)
{
    int32 count = 0;
    // *redefine* to perform or schedule first sending
    EV << "SctpNatPeer: socketEstablished\n";
    setStatusString("connected");
    if (rendezvous) {
        uint8_t buffer[100];
        int buflen = 14;
        struct nat_message *nat = (struct nat_message *)(buffer);

        nat->peer1 = par("ownName");
        nat->peer2 = par("peerName");
        nat->portPeer1 = par("localPort");
        nat->portPeer2 = 0;
        nat->peer1Addresses[0] = L3Address().toIpv4().getInt();
        nat->peer2Addresses[0] = L3Address().toIpv4().getInt();
        nat->numAddrPeer1 = 1;
        nat->numAddrPeer2 = 1;
        buflen = ADD_PADDING(buflen + 4 * (nat->numAddrPeer1 +  nat->numAddrPeer2));
        bool mul = par("multi");
        if (mul == true) {
            nat->multi = 1;
        } else {
            nat->multi = 0;
        }

        auto applicationData = makeShared<BytesChunk>(buffer, buflen);
        applicationData->addTag<CreationTimeTag>()->setCreationTime(simTime());
        auto applicationPacket = new Packet("ApplicationPacket", SCTP_C_SEND_ORDERED);
        applicationPacket->insertAtBack(applicationData);
        auto sctpSendReq = applicationPacket->addTag<SctpSendReq>();
        sctpSendReq->setLast(true);
        sctpSendReq->setPrMethod(0);
        sctpSendReq->setPrValue(0);
        sctpSendReq->setSid(0);
        clientSocket.send(applicationPacket);

        if (par("multi").boolValue()) {
            Request *cmesg = new Request("SCTP_C_SEND_ASCONF", SCTP_C_SEND_ASCONF);
            SctpCommandReq *cmd = cmesg->addTag<SctpCommandReq>();
            cmd->setSocketId(clientSocket.getSocketId());
            clientSocket.sendNotification(cmesg);
        }
    }
    else {
        EV << " determine number of requests in this session\n";
        numRequestsToSend = par("numRequestsPerSession");
        numPacketsToReceive = par("numPacketsToReceive");
        if (numRequestsToSend < 1)
            numRequestsToSend = 0;
        // perform first request (next one will be sent when reply arrives)
        if (numRequestsToSend > 0) {
            if (par("thinkTime").doubleValue() > 0.0) {
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
                        if (count == queueSize * 2)
                            sendRequest();
                        else
                            sendRequest(false);
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

                if (numPacketsToReceive == 0 && par("waitToClose").doubleValue() > 0.0) {
                    timeMsg->setKind(MSGKIND_ABORT);
                    scheduleAt(simTime() + par("waitToClose"), timeMsg);
                }
                if (numRequestsToSend == 0 && par("waitToClose").doubleValue() == 0.0) {
                    EV << "socketEstablished:no more packets to send, call shutdown\n";
                    clientSocket.shutdown();
                }
            }
        }
    }
}

void SctpNatPeer::sendQueueRequest()
{
    Request *cmsg = new Request("SCTP_C_QUEUE_MSGS_LIMIT", SCTP_C_QUEUE_MSGS_LIMIT);
    SctpInfoReq *qinfo = cmsg->addTag<SctpInfoReq>();
    qinfo->setText(queueSize);
    qinfo->setSocketId(clientSocket.getSocketId());
    clientSocket.sendRequest(cmsg);
}

void SctpNatPeer::sendRequestArrived(SctpSocket *socket)
{
    int32 count = 0;

    EV << "sendRequestArrived numRequestsToSend=" << numRequestsToSend << "\n";
    while (numRequestsToSend > 0 && count++ < queueSize && sendAllowed) {
        numRequestsToSend--;
        if (count == queueSize || numRequestsToSend == 0)
            sendRequest();
        else
            sendRequest(false);

        if (numRequestsToSend == 0) {
            EV << "no more packets to send, call shutdown\n";
            clientSocket.shutdown();
        }
    }
}

void SctpNatPeer::socketDataArrived(SctpSocket *socket, Packet *msg, bool)
{
    // *redefine* to perform or schedule next sending
    packetsRcvd++;

    EV << "Client received packet Nr " << packetsRcvd << " from SCTP\n";

    auto& tags = getTags(msg);
    SctpRcvReq *ind = tags.findTag<SctpRcvReq>();

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
        delete msg;
        if (numPacketsToReceive == 0) {
            EV << "Peer: all packets received\n";
        }
    }
}

void SctpNatPeer::shutdownReceivedArrived(SctpSocket *socket)
{
    if (numRequestsToSend == 0 || rendezvous) {
        Message *cmsg = new Message("SCTP_C_NO_OUTSTANDING", SCTP_C_NO_OUTSTANDING);
        SctpCommandReq *qinfo = cmsg->addTag<SctpCommandReq>();
        qinfo->setSocketId(socket->getSocketId());
        clientSocket.sendNotification(cmsg);
    }
}

void SctpNatPeer::msgAbandonedArrived(SctpSocket *socket)
{
    chunksAbandoned++;
}

void SctpNatPeer::sendqueueFullArrived(SctpSocket *socket)
{
    sendAllowed = false;
}

void SctpNatPeer::addressAddedArrived(SctpSocket *socket, L3Address localAddr, L3Address remoteAddr)
{
    EV << getFullPath() << ": addressAddedArrived for remoteAddr " << remoteAddr << "\n";
    localAddressList.push_back(localAddr);
    clientSocket.addAddress(localAddr);
    if (rendezvous) {
        uint8_t buffer[100];
        int buflen = 16;
        struct nat_message *nat = (struct nat_message *)(buffer);
        nat->peer1 = par("ownName");
        nat->peer2 = par("peerName");
        nat->portPeer1 = par("localPort");
        nat->portPeer2 = 0;
        nat->numAddrPeer1 = 2;
        nat->numAddrPeer2 = 2;
        bool mul = par("multi");
        if (mul == true) {
            nat->multi = 1;
        } else {
            nat->multi = 0;
        }
        nat->peer1Addresses[0] = L3Address().toIpv4().getInt();
        nat->peer2Addresses[0] = L3Address().toIpv4().getInt();
        nat->peer1Addresses[1] = L3Address().toIpv4().getInt();
        nat->peer2Addresses[1] = L3Address().toIpv4().getInt();
        buflen = ADD_PADDING(buflen + 4 * (nat->numAddrPeer1 + nat->numAddrPeer2));

        auto applicationData = makeShared<BytesChunk>(buffer, buflen);
        applicationData->addTag<CreationTimeTag>()->setCreationTime(simTime());
        auto applicationPacket = new Packet("ApplicationPacket", SCTP_C_SEND_ORDERED);
        applicationPacket->insertAtBack(applicationData);
        auto sctpSendReq = applicationPacket->addTag<SctpSendReq>();
        sctpSendReq->setLast(true);
        sctpSendReq->setPrMethod(0);
        sctpSendReq->setPrValue(0);
        sctpSendReq->setSid(0);
        clientSocket.send(applicationPacket);
    }
}

void SctpNatPeer::finish()
{
    EV << getFullPath() << ": opened " << numSessions << " sessions\n";
    EV << getFullPath() << ": sent " << bytesSent << " bytes in " << packetsSent << " packets\n";
    for (auto & elem : rcvdBytesPerAssoc) {
        EV << getFullPath() << ": received " << elem.second << " bytes in assoc " << elem.first << "\n";
    }
    EV << getFullPath() << "Over all " << packetsRcvd << " packets received\n ";
    EV << getFullPath() << "Over all " << notifications << " notifications received\n ";
    for (auto & elem : bytesPerAssoc) {
        delete elem.second;
    }
    bytesPerAssoc.clear();
    for (auto & elem : endToEndDelay) {
        delete elem.second;
    }
    endToEndDelay.clear();
    for (auto & elem : histEndToEndDelay) {
        delete elem.second;
    }
    histEndToEndDelay.clear();
    rcvdPacketsPerAssoc.clear();
    sentPacketsPerAssoc.clear();
    rcvdBytesPerAssoc.clear();
}

} // namespace inet

