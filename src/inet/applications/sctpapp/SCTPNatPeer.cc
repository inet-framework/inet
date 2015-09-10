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

#include "inet/applications/sctpapp/SCTPNatPeer.h"
#include "inet/transportlayer/contract/sctp/SCTPSocket.h"
#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"
#include "inet/transportlayer/sctp/SCTPMessage_m.h"
#include <stdlib.h>
#include <stdio.h>
#include "inet/transportlayer/sctp/SCTPAssociation.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

using namespace sctp;

#define MSGKIND_CONNECT    0
#define MSGKIND_SEND       1
#define MSGKIND_ABORT      2
#define MSGKIND_PRIMARY    3
#define MSGKIND_RESET      4
#define MSGKIND_STOP       5

Define_Module(SCTPNatPeer);

SCTPNatPeer::SCTPNatPeer()
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

SCTPNatPeer::~SCTPNatPeer()
{
    cancelAndDelete(timeMsg);
    cancelAndDelete(timeoutMsg);
}

void SCTPNatPeer::initialize()
{
    WATCH(numSessions);
    WATCH(packetsSent);
    WATCH(packetsRcvd);
    WATCH(bytesSent);
    //WATCH(rcvdBytesPerAssoc);
    WATCH(numRequestsToSend);
    // parameters
    const char *addressesString = par("localAddress");
    AddressVector addresses = L3AddressResolver().resolve(cStringTokenizer(addressesString).asVector());
    int32 port = par("localPort");
    echo = par("echo").boolValue();
    delay = par("echoDelay");
    outboundStreams = par("outboundStreams");
    inboundStreams = par("inboundStreams");
    ordered = (bool)par("ordered");
    queueSize = par("queueSize");
    timeoutMsg = new cMessage("SrvAppTimer");
    if (addresses.size() == 0) {
        clientSocket.bind(port);
    }
    else {
        clientSocket.bindx(addresses, port);
    }
    clientSocket.setCallbackObject(this);
    clientSocket.setOutputGate(gate("sctpOut"));
    rendezvous = (bool)par("rendezvous");
    if ((simtime_t)par("startTime") > 0) {
        cMessage *msg = new cMessage("ConnectTimer");
        msg->setKind(MSGKIND_CONNECT);
        scheduleAt((simtime_t)par("startTime"), msg);
    }
}

void SCTPNatPeer::sendOrSchedule(cMessage *msg)
{
    if (delay == 0) {
        send(msg, "sctpOut");
    }
    else {
        scheduleAt(simTime() + delay, msg);
    }
}

void SCTPNatPeer::generateAndSend()
{
    uint32 numBytes;
    EV << "SCTPNatPeer:generateAndSend \n";

    cPacket *cmsg = new cPacket("SCTP_C_SEND");
    SCTPSimpleMessage *msg = new SCTPSimpleMessage("Server");
    numBytes = (int64)(long)par("requestLength");
    msg->setDataArraySize(numBytes);
    for (uint32 i = 0; i < numBytes; i++) {
        msg->setData(i, 's');
    }
    msg->setDataLen(numBytes);
    msg->setEncaps(false);
    msg->setBitLength(numBytes * 8);
    cmsg->encapsulate(msg);
    SCTPSendInfo *cmd = new SCTPSendInfo();
    cmd->setAssocId(serverAssocId);
    if (ordered)
        cmd->setSendUnordered(COMPLETE_MESG_ORDERED);
    else
        cmd->setSendUnordered(COMPLETE_MESG_UNORDERED);
    lastStream = (lastStream + 1) % outboundStreams;
    cmd->setSid(lastStream);
    cmd->setPrValue((double)par("prValue"));
    cmd->setPrMethod((int32)par("prMethod"));
    cmd->setLast(true);
    cmsg->setKind(SCTP_C_SEND);
    cmsg->setControlInfo(cmd);
    packetsSent++;
    bytesSent += msg->getBitLength() / 8;
    sendOrSchedule(cmsg);
}

void SCTPNatPeer::connectx(AddressVector connectAddressList, int32 connectPort)
{
    uint32 outStreams = par("outboundStreams");
    clientSocket.setOutboundStreams(outStreams);
    uint32 inStreams = par("inboundStreams");
    clientSocket.setInboundStreams(inStreams);

    EV << "issuing OPEN command\n";
    EV << "Assoc " << clientSocket.getConnectionId() << "::connect to  port " << connectPort << "\n";
    bool streamReset = par("streamReset");
    clientSocket.connectx(connectAddressList, connectPort, streamReset, (int32)par("prMethod"), (uint32)par("numRequestsPerSession"));
    numSessions++;

    if (!streamReset)
        streamReset = false;
    else if (streamReset == true) {
        cMessage *cmsg = new cMessage("StreamReset");
        cmsg->setKind(MSGKIND_RESET);
        EV << "StreamReset Timer scheduled at " << simTime() << "\n";
        scheduleAt(simTime() + (double)par("streamRequestTime"), cmsg);
    }
    uint32 streamNum = 0;
    cStringTokenizer tokenizer(par("streamPriorities").stringValue());
    while (tokenizer.hasMoreTokens()) {
        const char *token = tokenizer.nextToken();
        clientSocket.setStreamPriority(streamNum, (uint32)atoi(token));

        streamNum++;
    }
}

void SCTPNatPeer::connect(L3Address connectAddress, int32 connectPort)
{
    clientSocket.setOutboundStreams(outboundStreams);
    clientSocket.setInboundStreams(inboundStreams);

    EV << "issuing OPEN command\n";
    EV << "Assoc " << clientSocket.getConnectionId() << "::connect to address " << connectAddress << ", port " << connectPort << "\n";
    bool streamReset = par("streamReset");
    clientSocket.connect(connectAddress, connectPort, streamReset, (int32)par("prMethod"), (uint32)par("numRequestsPerSession"));
    numSessions++;

    if (!streamReset)
        streamReset = false;
    else if (streamReset == true) {
        cMessage *cmsg = new cMessage("StreamReset");
        cmsg->setKind(MSGKIND_RESET);
        EV << "StreamReset Timer scheduled at " << simTime() << "\n";
        scheduleAt(simTime() + (double)par("streamRequestTime"), cmsg);
    }
    uint32 streamNum = 0;
    cStringTokenizer tokenizer(par("streamPriorities").stringValue());
    while (tokenizer.hasMoreTokens()) {
        const char *token = tokenizer.nextToken();
        clientSocket.setStreamPriority(streamNum, (uint32)atoi(token));

        streamNum++;
    }
}

void SCTPNatPeer::handleMessage(cMessage *msg)
{
    int32 id;

    if (msg->isSelfMessage()) {
        handleTimer(msg);
    }
    else {
        EV << "SCTPNatPeer::handleMessage kind=" << SCTPAssociation::indicationName(msg->getKind()) << " (" << msg->getKind() << ")\n";
        switch (msg->getKind()) {
            case SCTP_I_ADDRESS_ADDED:
                if (rendezvous)
                    clientSocket.processMessage(PK(msg));
                else
                    delete msg;
                break;

            case SCTP_I_PEER_CLOSED:
            case SCTP_I_ABORT: {
                if (rendezvous)
                    clientSocket.processMessage(PK(msg));
                else {
                    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->getControlInfo()->dup());
                    cMessage *cmsg = new cMessage("Notification");
                    SCTPSendInfo *cmd = new SCTPSendInfo();
                    id = ind->getAssocId();
                    cmd->setAssocId(id);
                    cmd->setSid(ind->getSid());
                    cmd->setNumMsgs(ind->getNumMsgs());
                    cmsg->setControlInfo(cmd);
                    delete ind;
                    delete msg;
                    cmsg->setKind(SCTP_C_ABORT);
                    sendOrSchedule(cmsg);
                }

                break;
            }

            case SCTP_I_ESTABLISHED: {
                if (clientSocket.getState() == SCTPSocket::CONNECTING)
                    clientSocket.processMessage(PK(msg));
                else {
                    int32 count = 0;
                    SCTPConnectInfo *connectInfo = check_and_cast<SCTPConnectInfo *>(msg->removeControlInfo());
                    numSessions++;
                    serverAssocId = connectInfo->getAssocId();
                    id = serverAssocId;
                    outboundStreams = connectInfo->getOutboundStreams();
                    inboundStreams = connectInfo->getInboundStreams();
                    rcvdPacketsPerAssoc[serverAssocId] = (int64)(long)par("numPacketsToReceivePerClient");
                    sentPacketsPerAssoc[serverAssocId] = (int64)(long)par("numPacketsToSendPerClient");
                    char text[128];
                    sprintf(text, "App: Received Bytes of assoc %d", serverAssocId);
                    bytesPerAssoc[serverAssocId] = new cOutVector(text);
                    rcvdBytesPerAssoc[serverAssocId] = 0;
                    sprintf(text, "App: EndToEndDelay of assoc %d", serverAssocId);
                    endToEndDelay[serverAssocId] = new cOutVector(text);
                    sprintf(text, "Hist: EndToEndDelay of assoc %d", serverAssocId);
                    histEndToEndDelay[serverAssocId] = new cDoubleHistogram(text);
                    delete connectInfo;
                    delete msg;

                    if ((int64)(long)par("numPacketsToSendPerClient") > 0) {
                        auto i = sentPacketsPerAssoc.find(serverAssocId);
                        numRequestsToSend = i->second;
                        if ((simtime_t)par("thinkTime") > 0) {
                            generateAndSend();
                            timeoutMsg->setKind(SCTP_C_SEND);
                            scheduleAt(simTime() + (simtime_t)par("thinkTime"), timeoutMsg);
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

                                cMessage *cmsg = new cMessage("SCTP_C_QUEUE_MSGS_LIMIT");
                                SCTPInfo *qinfo = new SCTPInfo();
                                qinfo->setText(queueSize);
                                cmsg->setKind(SCTP_C_QUEUE_MSGS_LIMIT);
                                qinfo->setAssocId(id);
                                cmsg->setControlInfo(qinfo);
                                sendOrSchedule(cmsg);
                            }

                            EV << "!!!!!!!!!!!!!!!All data sent from Peer !!!!!!!!!!\n";

                            auto j = rcvdPacketsPerAssoc.find(serverAssocId);
                            if (j->second == 0 && (simtime_t)par("waitToClose") > 0) {
                                char as[5];
                                sprintf(as, "%d", serverAssocId);
                                cMessage *abortMsg = new cMessage(as);
                                abortMsg->setKind(SCTP_I_ABORT);
                                scheduleAt(simTime() + (simtime_t)par("waitToClose"), abortMsg);
                            }
                            else {
                                EV << "no more packets to send, call shutdown for assoc " << serverAssocId << "\n";
                                cMessage *cmsg = new cMessage("SCTP_C_SHUTDOWN");
                                //SCTPInfo* qinfo = new SCTPInfo();
                                SCTPCommand *cmd = new SCTPCommand();
                                cmsg->setKind(SCTP_C_SHUTDOWN);
                                cmd->setAssocId(serverAssocId);
                                //qinfo->setAssocId(id);
                                //cmsg->setControlInfo(qinfo);
                                cmsg->setControlInfo(cmd);
                                sendOrSchedule(cmsg);
                            }
                        }
                    }
                }
                break;
            }

            case SCTP_I_DATA_NOTIFICATION: {
                notifications++;
                SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
                cMessage *cmsg = new cMessage("SCTP_C_RECEIVE");
                SCTPSendInfo *cmd = new SCTPSendInfo();
                id = ind->getAssocId();
                cmd->setAssocId(id);
                cmd->setSid(ind->getSid());
                cmd->setNumMsgs(ind->getNumMsgs());
                cmsg->setKind(SCTP_C_RECEIVE);
                cmsg->setControlInfo(cmd);
                delete ind;
                delete msg;
                if (!cmsg->isScheduled() && schedule == false) {
                    scheduleAt(simTime() + (simtime_t)par("delayFirstRead"), cmsg);
                }
                else if (schedule == true)
                    sendOrSchedule(cmsg);
                break;
            }

            case SCTP_I_DATA: {
                SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->getControlInfo());
                id = ind->getAssocId();
                if (rendezvous) {
                    SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>(msg);
                    NatMessage *nat = check_and_cast<NatMessage *>(smsg->decapsulate());
                    peerAddressList.clear();
                    if (nat->getMulti()) {
                        peerAddressList.push_back(nat->getPeer2Addresses(0));
                        EV << "address 0: " << nat->getPeer2Addresses(0) << endl;
                        peerAddressList.push_back(nat->getPeer2Addresses(1));
                        EV << "address 1: " << nat->getPeer2Addresses(1) << endl;
                    }
                    else {
                        peerAddress = nat->getPeer2Addresses(0);
                    }
                    peerPort = nat->getPortPeer2();
                    delete nat;
                    delete smsg;
                }
                else {
                    auto j = rcvdBytesPerAssoc.find(id);
                    if (j == rcvdBytesPerAssoc.end() && (clientSocket.getState() == SCTPSocket::CONNECTED))
                        clientSocket.processMessage(PK(msg));
                    else if (j != rcvdBytesPerAssoc.end()) {
                        j->second += PK(msg)->getBitLength() / 8;
                        auto k = bytesPerAssoc.find(id);
                        k->second->record(j->second);
                        packetsRcvd++;
                        if (!echo) {
                            if ((int64)(long)par("numPacketsToReceivePerClient") > 0) {
                                auto i = rcvdPacketsPerAssoc.find(id);
                                i->second--;
                                SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>(msg);
                                auto j = endToEndDelay.find(id);
                                j->second->record(simTime() - smsg->getCreationTime());
                                auto k = histEndToEndDelay.find(id);
                                k->second->collect(simTime() - smsg->getCreationTime());

                                if (i->second == 0) {
                                    cMessage *cmsg = new cMessage("SCTP_C_NO_OUTSTANDING");
                                    SCTPInfo *qinfo = new SCTPInfo();
                                    cmsg->setKind(SCTP_C_NO_OUTSTANDING);
                                    qinfo->setAssocId(id);
                                    cmsg->setControlInfo(qinfo);
                                    sendOrSchedule(cmsg);
                                }
                            }
                            delete msg;
                        }
                        else {
                            SCTPSendInfo *cmd = new SCTPSendInfo();
                            cmd->setAssocId(id);

                            SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>(msg->dup());
                            auto j = endToEndDelay.find(id);
                            j->second->record(simTime() - smsg->getCreationTime());
                            auto k = histEndToEndDelay.find(id);
                            k->second->collect(simTime() - smsg->getCreationTime());
                            cPacket *cmsg = new cPacket("SCTP_C_SEND");
                            bytesSent += smsg->getBitLength() / 8;
                            cmd->setSendUnordered(cmd->getSendUnordered());
                            lastStream = (lastStream + 1) % outboundStreams;
                            cmd->setPrValue(0);
                            cmd->setSid(lastStream);
                            cmd->setLast(true);
                            cmsg->encapsulate(smsg);
                            cmsg->setKind(SCTP_C_SEND);
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
                SCTPCommand *command = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
                id = command->getAssocId();
                EV << "peer: SCTP_I_SHUTDOWN_RECEIVED for assoc " << id << "\n";
                auto i = rcvdPacketsPerAssoc.find(id);
                if (i == rcvdPacketsPerAssoc.end() && (clientSocket.getState() == SCTPSocket::CONNECTED))
                    clientSocket.processMessage(PK(msg));
                else if (i != rcvdPacketsPerAssoc.end()) {
                    if (i->second == 0) {
                        cMessage *cmsg = new cMessage("SCTP_C_NO_OUTSTANDING");
                        SCTPInfo *qinfo = new SCTPInfo();
                        cmsg->setKind(SCTP_C_NO_OUTSTANDING);
                        qinfo->setAssocId(id);
                        cmsg->setControlInfo(qinfo);
                        sendOrSchedule(cmsg);
                    }

                    shutdownReceived = true;
                    delete msg;
                } else {
                    delete msg;
                }
                delete command;
                //delete msg;
            }

            case SCTP_I_SEND_STREAMS_RESETTED:
            case SCTP_I_RCV_STREAMS_RESETTED: {
                EV << "Streams have been resetted\n";
                break;
            }

            case SCTP_I_CLOSED:
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

void SCTPNatPeer::handleTimer(cMessage *msg)
{
    cMessage *cmsg;
    SCTPCommand *cmd;
    int32 id;

    EV << "SCTPNatPeer::handleTimer\n";

    SCTPConnectInfo *connectInfo = dynamic_cast<SCTPConnectInfo *>(msg->getControlInfo());
    switch (msg->getKind()) {
        case MSGKIND_CONNECT:
            EV << "starting session call connect\n";
            connect(L3AddressResolver().resolve(par("connectAddress"), 1), par("connectPort"));
            delete msg;
            break;

        case SCTP_C_SEND:

            EV << "SCTPNatPeer:MSGKIND_SEND\n";

            if (numRequestsToSend > 0) {
                generateAndSend();
                if ((simtime_t)par("thinkTime") > 0)
                    scheduleAt(simTime() + (simtime_t)par("thinkTime"), timeoutMsg);
                numRequestsToSend--;
            }
            break;

        case SCTP_I_ABORT:

            EV << "SCTPNatPeer:MsgKIND_ABORT for assoc " << atoi(msg->getName()) << "\n";

            cmsg = new cMessage("CLOSE", SCTP_C_CLOSE);
            cmd = new SCTPCommand();
            id = atoi(msg->getName());
            cmd->setAssocId(id);
            cmsg->setControlInfo(cmd);
            sendOrSchedule(cmsg);
            break;

        case SCTP_C_RECEIVE:

            EV << "SCTPNatPeer:SCTP_C_RECEIVE\n";
            schedule = true;
            sendOrSchedule(msg);
            break;

        default:

            EV << "MsgKind =" << msg->getKind() << " unknown\n";

            break;
    }
    delete connectInfo;
}

void SCTPNatPeer::socketDataNotificationArrived(int32 connId, void *ptr, cPacket *msg)
{
    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
    cMessage *cmsg = new cMessage("SCTP_C_RECEIVE");
    SCTPSendInfo *cmd = new SCTPSendInfo();
    cmd->setAssocId(ind->getAssocId());
    cmd->setSid(ind->getSid());
    cmd->setNumMsgs(ind->getNumMsgs());
    cmsg->setKind(SCTP_C_RECEIVE);
    cmsg->setControlInfo(cmd);
    delete ind;
    clientSocket.sendNotification(cmsg);
}

void SCTPNatPeer::socketPeerClosed(int32, void *)
{
    // close the connection (if not already closed)
    if (clientSocket.getState() == SCTPSocket::PEER_CLOSED) {
        EV << "remote SCTP closed, closing here as well\n";
        setStatusString("closed");
        clientSocket.close();
        if (rendezvous) {
            const char *addressesString = par("localAddress");
            AddressVector addresses = L3AddressResolver().resolve(cStringTokenizer(addressesString).asVector());
            int32 port = par("localPort");
            rendezvousSocket.setOutputGate(gate("sctpOut"));
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
            rendezvousSocket.listen(true, (bool)par("streamReset"), par("numPacketsToSendPerClient"));
            if ((bool)par("multi"))
                connectx(peerAddressList, peerPort);
            else
                connect(peerAddress, peerPort);
            rendezvous = false;
        }
    }
}

void SCTPNatPeer::socketClosed(int32, void *)
{
    // *redefine* to start another session etc.

    EV << "connection closed\n";
    setStatusString("closed");
    clientSocket.close();
    if (rendezvous) {
        const char *addressesString = par("localAddress");
        AddressVector addresses = L3AddressResolver().resolve(cStringTokenizer(addressesString).asVector());
        int32 port = par("localPort");
        rendezvousSocket.setOutputGate(gate("sctpOut"));
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
        rendezvousSocket.listen(true, (bool)par("streamReset"), par("numPacketsToSendPerClient"));
        if ((bool)par("multi"))
            connectx(peerAddressList, peerPort);
        else
            connect(peerAddress, peerPort);
        rendezvous = false;
    }
}

void SCTPNatPeer::socketFailure(int32, void *, int32 code)
{
    // subclasses may override this function, and add code try to reconnect after a delay.
    EV << "connection broken\n";
    setStatusString("broken");

    //numBroken++;

    // reconnect after a delay
    timeMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime() + (simtime_t)par("reconnectInterval"), timeMsg);
}

void SCTPNatPeer::socketStatusArrived(int32 assocId, void *yourPtr, SCTPStatusInfo *status)
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
    delete status;
}

void SCTPNatPeer::setStatusString(const char *s)
{
    if (hasGUI())
        getDisplayString().setTagArg("t", 0, s);
}

void SCTPNatPeer::sendRequest(bool last)
{
    EV << "sending request, " << numRequestsToSend - 1 << " more to go\n";
    uint32 i;
    int64 numBytes = (int64)(long)par("requestLength");
    if (numBytes < 1)
        numBytes = 1;

    EV << "SCTPNatPeer: sending " << numBytes << " data bytes\n";

    cPacket *cmsg = new cPacket("SCTP_C_SEND");
    SCTPSimpleMessage *msg = new SCTPSimpleMessage("data");

    msg->setDataArraySize(numBytes);
    for (i = 0; i < numBytes; i++) {
        msg->setData(i, 'a');
    }
    msg->setDataLen(numBytes);
    msg->setEncaps(false);
    msg->setBitLength(numBytes * 8);
    msg->setCreationTime(simTime());
    cmsg->encapsulate(msg);
    if (ordered)
        cmsg->setKind(SCTP_C_SEND_ORDERED);
    else
        cmsg->setKind(SCTP_C_SEND_UNORDERED);
    SCTPSendInfo* sendCommand = new SCTPSendInfo;
    sendCommand->setLast(true);
    cmsg->setControlInfo(sendCommand);
    // send SCTPMessage with SCTPSimpleMessage enclosed
    clientSocket.sendMsg(cmsg);
    bytesSent += numBytes;
}

void SCTPNatPeer::socketEstablished(int32, void *, unsigned long int buffer)
{
    int32 count = 0;
    // *redefine* to perform or schedule first sending
    EV << "SCTPNatPeer: socketEstablished\n";
    setStatusString("connected");
    if (rendezvous) {
        NatMessage *msg = new NatMessage("Rendezvous");
        msg->setKind(SCTP_C_NAT_INFO);
        msg->setMulti((bool)par("multi"));
        msg->setPeer1(par("ownName"));
        msg->setPeer1AddressesArraySize(1);
        msg->setPeer1Addresses(0, L3Address());
        msg->setPortPeer1(par("localPort"));
        msg->setPeer2(par("peerName"));
        msg->setPeer2AddressesArraySize(1);
        msg->setPeer2Addresses(0, L3Address());
        msg->setPortPeer2(0);
        cPacket *cmsg = new cPacket(msg->getName());
        SCTPSimpleMessage *smsg = new SCTPSimpleMessage("nat_data");
        smsg->setEncaps(true);
        smsg->encapsulate(msg);
        smsg->setCreationTime(simTime());
        smsg->setByteLength(16);
        smsg->setDataLen(16);
        cmsg->encapsulate(smsg);
        clientSocket.sendMsg(cmsg);

        if ((bool)par("multi")) {
            cMessage *cmesg = new cMessage("SCTP_C_SEND_ASCONF");
            SCTPCommand *cmd = new SCTPCommand();
            cmd->setAssocId(clientSocket.getConnectionId());
            cmesg->setControlInfo(cmd);
            cmesg->setKind(SCTP_C_SEND_ASCONF);
            clientSocket.sendNotification(cmesg);
        }
    }
    else {
        EV << " determine number of requests in this session\n";
        numRequestsToSend = (int64)(long)par("numRequestsPerSession");
        numPacketsToReceive = (int64)(long)par("numPacketsToReceive");
        if (numRequestsToSend < 1)
            numRequestsToSend = 0;
        // perform first request (next one will be sent when reply arrives)
        if (numRequestsToSend > 0) {
            if ((simtime_t)par("thinkTime") > 0) {
                if (sendAllowed) {
                    sendRequest();
                    numRequestsToSend--;
                }
                timeMsg->setKind(MSGKIND_SEND);
                scheduleAt(simTime() + (simtime_t)par("thinkTime"), timeMsg);
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

                if (numPacketsToReceive == 0 && (simtime_t)par("waitToClose") > 0) {
                    timeMsg->setKind(MSGKIND_ABORT);
                    scheduleAt(simTime() + (simtime_t)par("waitToClose"), timeMsg);
                }
                if (numRequestsToSend == 0 && (simtime_t)par("waitToClose") == 0) {
                    EV << "socketEstablished:no more packets to send, call shutdown\n";
                    clientSocket.shutdown();
                }
            }
        }
    }
}

void SCTPNatPeer::sendQueueRequest()
{
    cMessage *cmsg = new cMessage("SCTP_C_QUEUE_MSGS_LIMIT");
    SCTPInfo *qinfo = new SCTPInfo();
    qinfo->setText(queueSize);
    cmsg->setKind(SCTP_C_QUEUE_MSGS_LIMIT);
    qinfo->setAssocId(clientSocket.getConnectionId());
    cmsg->setControlInfo(qinfo);
    clientSocket.sendRequest(cmsg);
}

void SCTPNatPeer::sendRequestArrived()
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

void SCTPNatPeer::socketDataArrived(int32, void *, cPacket *msg, bool)
{
    // *redefine* to perform or schedule next sending
    packetsRcvd++;

    EV << "Client received packet Nr " << packetsRcvd << " from SCTP\n";
    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->getControlInfo());
    bytesRcvd += msg->getBitLength() / 8;

    if (echo) {
        SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>(msg);
        cPacket *cmsg = new cPacket("SCTP_C_SEND");
        echoedBytesSent += smsg->getBitLength() / 8;
        cmsg->encapsulate(smsg);
        if (ind->getSendUnordered())
            cmsg->setKind(SCTP_C_SEND_UNORDERED);
        else
            cmsg->setKind(SCTP_C_SEND_ORDERED);
        packetsSent++;
        clientSocket.sendMsg(cmsg);
    }
    if ((int64)(long)par("numPacketsToReceive") > 0) {
        numPacketsToReceive--;
        delete msg;
        if (numPacketsToReceive == 0) {
            EV << "Peer: all packets received\n";
        }
    }
}

void SCTPNatPeer::shutdownReceivedArrived(int32 connId)
{
    if (numRequestsToSend == 0 || rendezvous) {
        cMessage *cmsg = new cMessage("SCTP_C_NO_OUTSTANDING");
        SCTPInfo *qinfo = new SCTPInfo();
        cmsg->setKind(SCTP_C_NO_OUTSTANDING);
        qinfo->setAssocId(connId);
        cmsg->setControlInfo(qinfo);
        clientSocket.sendNotification(cmsg);
    }
}

void SCTPNatPeer::msgAbandonedArrived(int32 assocId)
{
    chunksAbandoned++;
}

void SCTPNatPeer::sendqueueFullArrived(int32 assocId)
{
    sendAllowed = false;
}

void SCTPNatPeer::addressAddedArrived(int32 assocId, L3Address localAddr, L3Address remoteAddr)
{
    EV << getFullPath() << ": addressAddedArrived for remoteAddr " << remoteAddr << "\n";
    localAddressList.push_back(localAddr);
    clientSocket.addAddress(localAddr);
    if (rendezvous) {
        NatMessage *msg = new NatMessage("Rendezvous");
        msg->setKind(SCTP_C_NAT_INFO);
        msg->setMulti((bool)par("multi"));
        msg->setPeer1(par("ownName"));
        msg->setPeer1AddressesArraySize(2);
        msg->setPeer1Addresses(0, L3Address());
        msg->setPeer1Addresses(1, L3Address());
        msg->setPortPeer1(par("localPort"));
        msg->setPeer2(par("peerName"));
        msg->setPeer2AddressesArraySize(2);
        msg->setPeer2Addresses(0, L3Address());
        msg->setPeer2Addresses(1, L3Address());
        msg->setPortPeer2(0);
        cPacket* cmsg = new cPacket(msg->getName());
        SCTPSimpleMessage *smsg = new SCTPSimpleMessage("nat_data");
        smsg->setEncaps(true);
        smsg->encapsulate(msg);
        smsg->setCreationTime(simTime());
        smsg->setByteLength(16);
        smsg->setDataLen(16);
        cmsg->encapsulate(smsg);

        SCTPSendInfo* sendCommand = new SCTPSendInfo;
        sendCommand->setLast(true);
        sendCommand->setAssocId(assocId);
        cmsg->setControlInfo(sendCommand);

        clientSocket.sendMsg(cmsg);
    }
}

void SCTPNatPeer::finish()
{
    EV << getFullPath() << ": opened " << numSessions << " sessions\n";
    EV << getFullPath() << ": sent " << bytesSent << " bytes in " << packetsSent << " packets\n";
    for (auto & elem : rcvdBytesPerAssoc) {
        EV << getFullPath() << ": received " << elem.second << " bytes in assoc " << elem.first << "\n";
    }
    EV << getFullPath() << "Over all " << packetsRcvd << " packets received\n ";
    EV << getFullPath() << "Over all " << notifications << " notifications received\n ";
    for (auto j = bytesPerAssoc.begin(); j != bytesPerAssoc.end(); j++) {
        delete j->second;
    }
    bytesPerAssoc.clear();
    for (auto k = endToEndDelay.begin(); k != endToEndDelay.end(); k++) {
        delete k->second;
    }
    endToEndDelay.clear();
    for (auto l = histEndToEndDelay.begin(); l != histEndToEndDelay.end(); l++) {
        delete l->second;
    }
    histEndToEndDelay.clear();
    rcvdPacketsPerAssoc.clear();
    sentPacketsPerAssoc.clear();
    rcvdBytesPerAssoc.clear();
}

} // namespace inet

