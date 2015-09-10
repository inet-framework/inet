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

#include "inet/applications/sctpapp/SCTPPeer.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/sctp/SCTPAssociation.h"
#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"
#include "inet/transportlayer/sctp/SCTPMessage_m.h"
#include "inet/transportlayer/contract/sctp/SCTPSocket.h"

#include <stdlib.h>
#include <stdio.h>

namespace inet {

#define MSGKIND_CONNECT    0
#define MSGKIND_SEND       1
#define MSGKIND_ABORT      2
#define MSGKIND_PRIMARY    3
#define MSGKIND_RESET      4
#define MSGKIND_STOP       5

Define_Module(SCTPPeer);

simsignal_t SCTPPeer::sentPkSignal = registerSignal("sentPk");
simsignal_t SCTPPeer::echoedPkSignal = registerSignal("echoedPk");
simsignal_t SCTPPeer::rcvdPkSignal = registerSignal("rcvdPk");

SCTPPeer::SCTPPeer()
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

SCTPPeer::~SCTPPeer()
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

void SCTPPeer::initialize(int stage)
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
        ordered = par("ordered").boolValue();
        queueSize = par("queueSize");
        timeoutMsg = new cMessage("SrvAppTimer");
        listeningSocket.setOutputGate(gate("sctpOut"));
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
        listeningSocket.listen(true, par("streamReset").boolValue(), par("numPacketsToSendPerClient").longValue());
        EV_DEBUG << "SCTPPeer::initialized listen port=" << port << "\n";
        clientSocket.setCallbackObject(this);
        clientSocket.setOutputGate(gate("sctpOut"));

        if ((simtime_t)par("startTime") > SIMTIME_ZERO) {    //FIXME is invalid the startTime == 0 ????
            connectTimer = new cMessage("ConnectTimer");
            connectTimer->setKind(MSGKIND_CONNECT);
            scheduleAt(par("startTime"), connectTimer);
        }

        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void SCTPPeer::sendOrSchedule(cMessage *msg)
{
    if (delay == 0) {
        send(msg, "sctpOut");
    }
    else {
        scheduleAt(simTime() + delay, msg);
    }
}

void SCTPPeer::generateAndSend(SCTPConnectInfo *connectInfo)
{
    cPacket *cmsg = new cPacket("SCTP_C_SEND");
    SCTPSimpleMessage *msg = new SCTPSimpleMessage("Server");
    int numBytes = par("requestLength");
    msg->setDataArraySize(numBytes);

    for (int i = 0; i < numBytes; i++)
        msg->setData(i, 's');

    msg->setDataLen(numBytes);
    msg->setByteLength(numBytes);
    msg->setEncaps(false);
    cmsg->encapsulate(msg);
    SCTPSendInfo *cmd = new SCTPSendInfo();
    cmd->setAssocId(serverAssocId);

    if (ordered)
        cmd->setSendUnordered(COMPLETE_MESG_ORDERED);
    else
        cmd->setSendUnordered(COMPLETE_MESG_UNORDERED);

    lastStream = (lastStream + 1) % outboundStreams;
    cmd->setSid(lastStream);
    cmd->setPrValue(par("prValue"));
    cmd->setPrMethod(par("prMethod"));
    cmd->setLast(true);
    cmsg->setKind(SCTP_C_SEND);
    cmsg->setControlInfo(cmd);
    packetsSent++;
    bytesSent += msg->getByteLength();
    emit(sentPkSignal, msg);
    sendOrSchedule(cmsg);
}

void SCTPPeer::connect()
{
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");
    int outStreams = par("outboundStreams");
    clientSocket.setOutboundStreams(outStreams);

    EV_INFO << "issuing OPEN command\n";
    EV_INFO << "Assoc " << clientSocket.getConnectionId() << "::connect to address " << connectAddress << ", port " << connectPort << "\n";
    numSessions++;
    bool streamReset = par("streamReset");
    L3Address destination;
    L3AddressResolver().tryResolve(connectAddress, destination);
    if (destination.isUnspecified())
        EV << "cannot resolve destination address: " << connectAddress << endl;
    else {
        clientSocket.connect(destination, connectPort, streamReset, (int)par("prMethod"), (unsigned int)par("numRequestsPerSession"));
    }

    if (streamReset) {
        cMessage *cmsg = new cMessage("StreamReset");
        cmsg->setKind(MSGKIND_RESET);
        EV_INFO << "StreamReset Timer scheduled at " << simTime() << "\n";
        scheduleAt(simTime() + par("streamRequestTime"), cmsg);
    }

    unsigned int streamNum = 0;
    cStringTokenizer tokenizer(par("streamPriorities").stringValue());
    while (tokenizer.hasMoreTokens()) {
        const char *token = tokenizer.nextToken();
        clientSocket.setStreamPriority(streamNum, (unsigned int)atoi(token));

        streamNum++;
    }
}

void SCTPPeer::handleMessage(cMessage *msg)
{
    int id = -1;

    if (msg->isSelfMessage())
        handleTimer(msg);

    switch (msg->getKind()) {
        case SCTP_I_PEER_CLOSED:
        case SCTP_I_ABORT: {
            SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->getControlInfo()->dup());
            cMessage *cmsg = new cMessage("SCTP_C_ABORT");
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
            break;
        }

        case SCTP_I_ESTABLISHED: {
            if (clientSocket.getState() == SCTPSocket::CONNECTING)
                clientSocket.processMessage(PK(msg));
            else {
                SCTPConnectInfo *connectInfo = check_and_cast<SCTPConnectInfo *>(msg->removeControlInfo());
                numSessions++;
                serverAssocId = connectInfo->getAssocId();
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
                histEndToEndDelay[serverAssocId] = new cDoubleHistogram(text);

                //delete connectInfo;
                delete msg;

                if (par("numPacketsToSendPerClient").longValue() > 0) {
                    auto i = sentPacketsPerAssoc.find(serverAssocId);
                    numRequestsToSend = i->second;
                    if (par("thinkTime").doubleValue() > 0) {
                        generateAndSend(connectInfo);
                        timeoutMsg->setKind(SCTP_C_SEND);
                        scheduleAt(simTime() + par("thinkTime"), timeoutMsg);
                        numRequestsToSend--;
                        i->second = numRequestsToSend;
                    }
                    else {
                        if (queueSize == 0) {
                            while (numRequestsToSend > 0) {
                                generateAndSend(connectInfo);
                                numRequestsToSend--;
                                i->second = numRequestsToSend;
                            }
                        }
                        else if (queueSize > 0) {
                            int count = 0;
                            while (numRequestsToSend > 0 && count++ < queueSize * 2) {
                                generateAndSend(connectInfo);
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

                        EV_INFO << "!!!!!!!!!!!!!!!All data sent from Server !!!!!!!!!!\n";

                        auto j = rcvdPacketsPerAssoc.find(serverAssocId);
                        if (j->second == 0 && par("waitToClose").doubleValue() > 0) {
                            char as[5];
                            sprintf(as, "%d", serverAssocId);
                            cMessage *abortMsg = new cMessage(as);
                            abortMsg->setKind(SCTP_I_ABORT);
                            scheduleAt(simTime() + par("waitToClose"), abortMsg);
                        }
                        else {
                            EV_INFO << "no more packets to send, call shutdown for assoc " << serverAssocId << "\n";
                            cMessage *cmsg = new cMessage("SCTP_C_SHUTDOWN");
                            SCTPCommand *cmd = new SCTPCommand();
                            cmsg->setKind(SCTP_C_SHUTDOWN);
                            cmd->setAssocId(serverAssocId);
                            cmsg->setControlInfo(cmd);
                            sendOrSchedule(cmsg);
                        }
                    }
                }
            }
            break;
        }

        case SCTP_I_DATA_NOTIFICATION: {
            notificationsReceived++;
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
                scheduleAt(simTime() + par("delayFirstRead"), cmsg);
            }
            else if (schedule == true)
                sendOrSchedule(cmsg);
            break;
        }

        case SCTP_I_DATA: {
            SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->getControlInfo());
            id = ind->getAssocId();
            auto j = rcvdBytesPerAssoc.find(id);
            if (j == rcvdBytesPerAssoc.end() && (clientSocket.getState() == SCTPSocket::CONNECTED))
                clientSocket.processMessage(PK(msg));
            else if (j != rcvdBytesPerAssoc.end()) {
                j->second += PK(msg)->getByteLength();
                auto k = bytesPerAssoc.find(id);
                k->second->record(j->second);
                packetsRcvd++;

                if (!echo) {
                    if (par("numPacketsToReceivePerClient").longValue() > 0) {
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

                    SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>(msg);
                    auto j = endToEndDelay.find(id);
                    j->second->record(simTime() - smsg->getCreationTime());
                    auto k = histEndToEndDelay.find(id);
                    k->second->collect(simTime() - smsg->getCreationTime());
                    cPacket *cmsg = new cPacket("SCTP_C_SEND");
                    bytesSent += smsg->getByteLength();
                    cmd->setPrValue(0);
                    emit(sentPkSignal, smsg);
                    cmd->setSendUnordered(cmd->getSendUnordered());
                    lastStream = (lastStream + 1) % outboundStreams;
                    cmd->setSid(lastStream);
                    cmd->setLast(true);
                    cmsg->encapsulate(smsg);
                    cmsg->setKind(SCTP_C_SEND);
                    cmsg->setControlInfo(cmd);
                    packetsSent++;
                    sendOrSchedule(cmsg);
                }
            } else {
                delete msg;
            }
            break;
        }

        case SCTP_I_SHUTDOWN_RECEIVED: {
            SCTPCommand *command = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
            id = command->getAssocId();
            EV_INFO << "server: SCTP_I_SHUTDOWN_RECEIVED for assoc " << id << "\n";
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

                delete command;
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

void SCTPPeer::handleTimer(cMessage *msg)
{
    EV_TRACE << "SCTPPeer::handleTimer\n";

    SCTPConnectInfo *connectInfo = dynamic_cast<SCTPConnectInfo *>(msg->getControlInfo());

    switch (msg->getKind()) {
        case MSGKIND_CONNECT:
            EV_INFO << "starting session call connect\n";
            connect();
            break;

        case SCTP_C_SEND:
            if (numRequestsToSend > 0) {
                generateAndSend(connectInfo);
                if (par("thinkTime").doubleValue() > 0)
                    scheduleAt(simTime() + par("thinkTime"), timeoutMsg);
                numRequestsToSend--;
            }
            break;

        case SCTP_I_ABORT: {
            cMessage *cmsg = new cMessage("CLOSE", SCTP_C_CLOSE);
            SCTPCommand *cmd = new SCTPCommand();
            int id = atoi(msg->getName());
            cmd->setAssocId(id);
            cmsg->setControlInfo(cmd);
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

void SCTPPeer::socketDataNotificationArrived(int connId, void *ptr, cPacket *msg)
{
    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
    cMessage *cmsg = new cMessage("CMSG");
    SCTPSendInfo *cmd = new SCTPSendInfo();
    cmd->setAssocId(ind->getAssocId());
    cmd->setSid(ind->getSid());
    cmd->setNumMsgs(ind->getNumMsgs());
    cmsg->setKind(SCTP_C_RECEIVE);
    cmsg->setControlInfo(cmd);
    delete ind;
    clientSocket.sendNotification(cmsg);
}

void SCTPPeer::socketPeerClosed(int, void *)
{
    // close the connection (if not already closed)
    if (clientSocket.getState() == SCTPSocket::PEER_CLOSED) {
        EV_INFO << "remote SCTP closed, closing here as well\n";
        setStatusString("closing");
        clientSocket.close();
    }
}

void SCTPPeer::socketClosed(int, void *)
{
    // *redefine* to start another session etc.
    EV_INFO << "connection closed\n";
    setStatusString("closed");
}

void SCTPPeer::socketFailure(int, void *, int code)
{
    // subclasses may override this function, and add code try to reconnect after a delay.
    EV_WARN << "connection broken\n";
    setStatusString("broken");
    // reconnect after a delay
    timeMsg->setKind(MSGKIND_CONNECT);
    scheduleAt(simTime() + par("reconnectInterval"), timeMsg);
}

void SCTPPeer::socketStatusArrived(int assocId, void *yourPtr, SCTPStatusInfo *status)
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
    delete status;
}

void SCTPPeer::setStatusString(const char *s)
{
    if (hasGUI())
        getDisplayString().setTagArg("t", 0, s);
}

void SCTPPeer::sendRequest(bool last)
{
    EV_INFO << "sending request, " << numRequestsToSend - 1 << " more to go\n";
    long numBytes = par("requestLength");

    if (numBytes < 1)
        numBytes = 1;

    EV_INFO << "SCTPClient: sending " << numBytes << " data bytes\n";

    cPacket *cmsg = new cPacket("SCTP_C_SEND");
    SCTPSimpleMessage *msg = new SCTPSimpleMessage("data");

    msg->setDataArraySize(numBytes);

    for (int i = 0; i < numBytes; i++)
        msg->setData(i, 'a');

    msg->setDataLen(numBytes);
    msg->setEncaps(false);
    msg->setBitLength(numBytes * 8);
    msg->setCreationTime(simTime());
    cmsg->encapsulate(msg);
    cmsg->setKind(ordered ? SCTP_C_SEND_ORDERED : SCTP_C_SEND_UNORDERED);

    SCTPSendInfo* sendCommand = new SCTPSendInfo;
    sendCommand->setLast(last);
    cmsg->setControlInfo(sendCommand);
    
    // send SCTPMessage with SCTPSimpleMessage enclosed
    emit(sentPkSignal, msg);
    clientSocket.sendMsg(cmsg);
    bytesSent += numBytes;
}

void SCTPPeer::socketEstablished(int, void *)
{
    int count = 0;
    // *redefine* to perform or schedule first sending
    EV_INFO << "SCTPClient: connected\n";
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

void SCTPPeer::sendQueueRequest()
{
    cMessage *cmsg = new cMessage("SCTP_C_QUEUE_MSGS_LIMIT");
    SCTPInfo *qinfo = new SCTPInfo();
    qinfo->setText(queueSize);
    cmsg->setKind(SCTP_C_QUEUE_MSGS_LIMIT);
    qinfo->setAssocId(clientSocket.getConnectionId());
    cmsg->setControlInfo(qinfo);
    clientSocket.sendRequest(cmsg);
}

void SCTPPeer::sendRequestArrived()
{
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

void SCTPPeer::socketDataArrived(int, void *, cPacket *msg, bool)
{
    // *redefine* to perform or schedule next sending
    packetsRcvd++;

    EV_INFO << "Client received packet Nr " << packetsRcvd << " from SCTP\n";

    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->getControlInfo());

    emit(rcvdPkSignal, msg);
    bytesRcvd += msg->getByteLength();

    if (echo) {
        SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>(msg);
        cPacket *cmsg = new cPacket("SCTP_C_SEND");
        echoedBytesSent += smsg->getByteLength();
        emit(echoedPkSignal, smsg);
        cmsg->encapsulate(smsg);
        cmsg->setKind(ind->getSendUnordered() ? SCTP_C_SEND_UNORDERED : SCTP_C_SEND_ORDERED);
        packetsSent++;
        clientSocket.sendMsg(cmsg);
    }

    if (par("numPacketsToReceive").longValue() > 0) {
        numPacketsToReceive--;
        if (numPacketsToReceive == 0) {
            setStatusString("closing");
            clientSocket.close();
        }
    }
}

void SCTPPeer::shutdownReceivedArrived(int connId)
{
    if (numRequestsToSend == 0) {
        cMessage *cmsg = new cMessage("SCTP_C_NO_OUTSTANDING");
        SCTPInfo *qinfo = new SCTPInfo();
        cmsg->setKind(SCTP_C_NO_OUTSTANDING);
        qinfo->setAssocId(connId);
        cmsg->setControlInfo(qinfo);
        clientSocket.sendNotification(cmsg);
    }
}

void SCTPPeer::msgAbandonedArrived(int assocId)
{
    chunksAbandoned++;
}

void SCTPPeer::sendqueueFullArrived(int assocId)
{
    sendAllowed = false;
}

void SCTPPeer::finish()
{
    EV_INFO << getFullPath() << ": opened " << numSessions << " sessions\n";
    EV_INFO << getFullPath() << ": sent " << bytesSent << " bytes in " << packetsSent << " packets\n";

    for (auto & elem : rcvdBytesPerAssoc)
        EV_DETAIL << getFullPath() << ": received " << elem.second << " bytes in assoc " << elem.first << "\n";

    EV_INFO << getFullPath() << "Over all " << packetsRcvd << " packets received\n ";
    EV_INFO << getFullPath() << "Over all " << notificationsReceived << " notifications received\n ";
}

} // namespace inet

