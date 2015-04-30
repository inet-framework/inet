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

#include "inet/applications/sctpapp/SCTPServer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/sctp/SCTPAssociation.h"
#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"
#include "inet/transportlayer/sctp/SCTPMessage_m.h"
#include "inet/transportlayer/contract/sctp/SCTPSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

Define_Module(SCTPServer);

void SCTPServer::initialize(int stage)
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
        finishEndsSimulation = par("finishEndsSimulation").boolValue();

        inboundStreams = par("inboundStreams");
        outboundStreams = par("outboundStreams");
        ordered = par("ordered").boolValue();
        queueSize = par("queueSize");
        timeoutMsg = new cMessage("SrvAppTimer");
        delayTimer = new cMessage("delayTimer");
        delayTimer->setContextPointer(this);
        delayFirstReadTimer = new cMessage("delayFirstReadTimer");

        echo = par("echo");
        delay = par("echoDelay");
        delayFirstRead = par("delayFirstRead");
        cPar *delT = &par("readingInterval");
        if (delT->isNumeric() && (double)*delT == 0)
            readInt = false;
        else
            readInt = true;
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        const char *addressesString = par("localAddress");
        AddressVector addresses = L3AddressResolver().resolve(cStringTokenizer(addressesString).asVector());
        int port = par("localPort");
        int messagesToPush = par("messagesToPush");

        socket = new SCTPSocket();
        socket->setOutputGate(gate("sctpOut"));
        socket->setInboundStreams(inboundStreams);
        socket->setOutboundStreams(outboundStreams);

        if (addresses.size() == 0)
            socket->bind(port);
        else
            socket->bindx(addresses, port);

        socket->listen(true, par("streamReset").boolValue(), par("numPacketsToSendPerClient").longValue(), messagesToPush);
        EV_INFO << "SCTPServer::initialized listen port=" << port << "\n";
        cStringTokenizer tokenizer(par("streamPriorities").stringValue());
        for (unsigned int streamNum = 0; tokenizer.hasMoreTokens(); streamNum++) {
            const char *token = tokenizer.nextToken();
            socket->setStreamPriority(streamNum, (unsigned int)atoi(token));
        }

        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void SCTPServer::sendOrSchedule(cMessage *msg)
{
    if (delay == 0)
        send(msg, "sctpOut");
    else
        scheduleAt(simTime() + delay, msg);
}

void SCTPServer::generateAndSend()
{
    cPacket *cmsg = new cPacket("SCTP_C_SEND");
    SCTPSimpleMessage *msg = new SCTPSimpleMessage("Server");
    int numBytes = par("requestLength");
    msg->setDataArraySize(numBytes);

    for (int i = 0; i < numBytes; i++)
        msg->setData(i, 's');

    msg->setDataLen(numBytes);
    msg->setEncaps(false);
    msg->setBitLength(numBytes * 8);
    cmsg->encapsulate(msg);
    SCTPSendInfo *cmd = new SCTPSendInfo("Send1");
    cmd->setAssocId(assocId);
    cmd->setSendUnordered(ordered ? COMPLETE_MESG_ORDERED : COMPLETE_MESG_UNORDERED);
    lastStream = (lastStream + 1) % outboundStreams;
    cmd->setSid(lastStream);
    cmd->setPrValue(par("prValue"));
    cmd->setPrMethod(par("prMethod"));

    if (queueSize > 0 && numRequestsToSend > 0 && count < queueSize * 2)
        cmd->setLast(false);
    else
        cmd->setLast(true);

    cmsg->setKind(SCTP_C_SEND);
    cmsg->setControlInfo(cmd);
    packetsSent++;
    bytesSent += msg->getBitLength() / 8;
    sendOrSchedule(cmsg);
}

cMessage *SCTPServer::makeReceiveRequest(cMessage *msg)
{
    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
    cMessage *cmsg = new cMessage("ReceiveRequest");
    SCTPSendInfo *cmd = new SCTPSendInfo("Send2");
    cmd->setAssocId(ind->getAssocId());
    cmd->setSid(ind->getSid());
    cmd->setNumMsgs(ind->getNumMsgs());
    cmsg->setKind(SCTP_C_RECEIVE);
    cmsg->setControlInfo(cmd);
    delete ind;
    return cmsg;
}

cMessage *SCTPServer::makeDefaultReceive()
{
    cMessage *cmsg = new cMessage("DefaultReceive");
    SCTPSendInfo *cmd = new SCTPSendInfo("Send3");
    cmd->setAssocId(assocId);
    cmd->setSid(0);
    cmd->setNumMsgs(1);
    cmsg->setKind(SCTP_C_RECEIVE);
    cmsg->setControlInfo(cmd);
    return cmsg;
}

cMessage *SCTPServer::makeAbortNotification(SCTPCommand *msg)
{
    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg);
    cMessage *cmsg = new cMessage("SCTP_C_ABORT");
    SCTPSendInfo *cmd = new SCTPSendInfo("Send4");
    assocId = ind->getAssocId();
    cmd->setAssocId(assocId);
    cmd->setSid(ind->getSid());
    cmd->setNumMsgs(ind->getNumMsgs());
    cmsg->setControlInfo(cmd);
    delete ind;
    cmsg->setKind(SCTP_C_ABORT);
    return cmsg;
}

void SCTPServer::handleMessage(cMessage *msg)
{
    // TODO: there is another memory leak somewhere...
    int id = 0;
    cMessage *cmsg;
    if (msg->isSelfMessage())
        handleTimer(msg);
    else {
        switch (msg->getKind()) {
            case SCTP_I_PEER_CLOSED:
            case SCTP_I_ABORT: {
                SCTPCommand *command = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
                assocId = command->getAssocId();
                serverAssocStatMap[assocId].peerClosed = true;
                if (par("numPacketsToReceivePerClient").longValue() == 0) {
                    if (serverAssocStatMap[assocId].abortSent == false) {
                        sendOrSchedule(makeAbortNotification(command->dup()));
                        serverAssocStatMap[assocId].abortSent = true;
                    }
                }
                else {
                    if (serverAssocStatMap[assocId].rcvdPackets == (unsigned int)par("numPacketsToReceivePerClient")
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
                delete command;
                delete msg;
                break;
            }

            case SCTP_I_ESTABLISHED: {
                count = 0;
                SCTPConnectInfo *connectInfo = check_and_cast<SCTPConnectInfo *>(msg->removeControlInfo());
                numSessions++;
                assocId = connectInfo->getAssocId();
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

                delete connectInfo;
                delete msg;
                if (par("numPacketsToSendPerClient").longValue() > 0) {
                    auto i = serverAssocStatMap.find(assocId);
                    numRequestsToSend = i->second.sentPackets;
                    if (par("thinkTime").doubleValue() > 0) {
                        generateAndSend();
                        timeoutMsg->setKind(SCTP_C_SEND);
                        scheduleAt(simTime() + par("thinkTime"), timeoutMsg);
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

                            cMessage *cmsg = new cMessage("SCTP_C_QUEUE_MSGS_LIMIT");
                            SCTPInfo *qinfo = new SCTPInfo("Info1");
                            qinfo->setText(queueSize);
                            cmsg->setKind(SCTP_C_QUEUE_MSGS_LIMIT);
                            qinfo->setAssocId(id);
                            cmsg->setControlInfo(qinfo);
                            sendOrSchedule(cmsg);
                        }
                        auto j = serverAssocStatMap.find(assocId);
                        if (j->second.rcvdPackets == 0 && par("waitToClose").doubleValue() > 0) {
                            char as[5];
                            sprintf(as, "%d", assocId);
                            cMessage *abortMsg = new cMessage(as);
                            abortMsg->setKind(SCTP_I_ABORT);
                            scheduleAt(simTime() + par("waitToClose"), abortMsg);
                        }
                        else {
                            EV_INFO << "no more packets to send, call shutdown for assoc " << assocId << "\n";
                            cMessage *cmsg = new cMessage("ShutdownRequest");
                            SCTPCommand *cmd = new SCTPCommand("Send5");
                            cmsg->setKind(SCTP_C_SHUTDOWN);
                            cmd->setAssocId(assocId);
                            cmsg->setControlInfo(cmd);
                            sendOrSchedule(cmsg);
                        }
                    }
                }
                break;
            }

            case SCTP_I_DATA_NOTIFICATION: {
                notificationsReceived++;

                if (schedule == false) {
                    if (delayFirstRead > 0 && !delayFirstReadTimer->isScheduled()) {
                        cmsg = makeReceiveRequest(msg);
                        scheduleAt(simTime() + delayFirstRead, cmsg);
                        scheduleAt(simTime() + delayFirstRead, delayFirstReadTimer);
                    }
                    else if (readInt && firstData) {
                        firstData = false;
                        cmsg = makeReceiveRequest(msg);
                        scheduleAt(simTime() + par("readingInterval"), delayTimer);
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
                SCTPRcvInfo *ind = check_and_cast<SCTPRcvInfo *>(msg->removeControlInfo());
                id = ind->getAssocId();
                auto j = serverAssocStatMap.find(id);
                auto k = bytesPerAssoc.find(id);
                if (j->second.rcvdBytes == 0)
                    j->second.start = simTime();

                j->second.rcvdBytes += PK(msg)->getByteLength();
                k->second->record(j->second.rcvdBytes);

                if (!echo) {
                    if (par("numPacketsToReceivePerClient").longValue() > 0) {
                        j->second.rcvdPackets--;
                        SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>(msg);
                        auto m = endToEndDelay.find(id);
                        m->second->record(simTime() - smsg->getCreationTime());
                        EV_INFO << "server: Data received. Left packets to receive=" << j->second.rcvdPackets << "\n";

                        if (j->second.rcvdPackets == 0) {
                            if (serverAssocStatMap[assocId].peerClosed == true && serverAssocStatMap[assocId].abortSent == false) {
                                sendOrSchedule(makeAbortNotification(ind));
                                serverAssocStatMap[assocId].abortSent = true;
                                j->second.stop = simTime();
                                j->second.lifeTime = j->second.stop - j->second.start;
                                delete msg;
                                break;
                            }
                            else {
                                cMessage *cmsg = new cMessage("SCTP_C_NO_OUTSTANDING");
                                SCTPInfo *qinfo = new SCTPInfo("Info2");
                                cmsg->setKind(SCTP_C_NO_OUTSTANDING);
                                qinfo->setAssocId(id);
                                cmsg->setControlInfo(qinfo);
                                sendOrSchedule(cmsg);
                                j->second.stop = simTime();
                                j->second.lifeTime = j->second.stop - j->second.start;
                            }
                        }
                    }
                    delete msg;
                }
                else {
                    SCTPSendInfo *cmd = new SCTPSendInfo("SCTP_C_SEND");
                    cmd->setAssocId(id);
                    SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>(msg);
                    auto n = endToEndDelay.find(id);
                    n->second->record(simTime() - smsg->getCreationTime());
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
                    sendOrSchedule(cmsg);
                }
                delete ind;
                break;
            }

            case SCTP_I_SHUTDOWN_RECEIVED: {
                SCTPCommand *command = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
                id = command->getAssocId();
                EV_INFO << "server: SCTP_I_SHUTDOWN_RECEIVED for assoc " << id << "\n";
                auto i = serverAssocStatMap.find(id);
                if (i->second.sentPackets == 0 || par("numPacketsToSendPerClient").longValue() == 0) {
                    cMessage *cmsg = new cMessage("SCTP_C_NO_OUTSTANDING");
                    SCTPInfo *qinfo = new SCTPInfo("Info3");
                    cmsg->setKind(SCTP_C_NO_OUTSTANDING);
                    qinfo->setAssocId(id);
                    cmsg->setControlInfo(qinfo);
                    sendOrSchedule(cmsg);
                    i->second.stop = simTime();
                    i->second.lifeTime = i->second.stop - i->second.start;
                }
                delete command;
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
                SCTPCommand *command = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
                id = command->getAssocId();
                EV_INFO << "server: SCTP_I_CLOSED for assoc "  << id << endl;
                ServerAssocStatMap::iterator i = serverAssocStatMap.find(id);
                i->second.stop = simTime();
                i->second.lifeTime = i->second.stop - i->second.start;
                if (delayTimer->isScheduled())
                    cancelEvent(delayTimer);
                if (finishEndsSimulation)
                    endSimulation();
                delete command;
                delete msg;
                break;
            }

            default:
                delete msg;
                break;
        }
    }
}

void SCTPServer::handleTimer(cMessage *msg)
{
    if (msg == delayTimer) {
        EV_INFO << simTime() << " delayTimer expired\n";
        sendOrSchedule(makeDefaultReceive());
        scheduleAt(simTime() + par("readingInterval"), delayTimer);
        return;
    }
    else if (msg == delayFirstReadTimer) {
        delayFirstRead = 0;

        if (readInt && !delayTimer->isScheduled()) {
            double tempInterval = par("readingInterval");
            scheduleAt(simTime() + (simtime_t)tempInterval, delayTimer);
            scheduleAt(simTime() + (simtime_t)tempInterval, makeDefaultReceive());
        }
        return;
    }

    switch (msg->getKind()) {
        case SCTP_C_SEND:
            if (numRequestsToSend > 0) {
                generateAndSend();
                if (par("thinkTime").doubleValue() > 0)
                    scheduleAt(simTime() + par("thinkTime"), timeoutMsg);
                numRequestsToSend--;
            }
            break;

        case SCTP_I_ABORT: {
            cMessage *cmsg = new cMessage("SCTP_C_CLOSE", SCTP_C_CLOSE);
            SCTPCommand *cmd = new SCTPCommand("Send6");
            int id = atoi(msg->getName());
            cmd->setAssocId(id);
            cmsg->setControlInfo(cmd);
            sendOrSchedule(cmsg);
        }
        break;

        case SCTP_C_RECEIVE:
            EV_INFO << simTime() << " SCTPServer:SCTP_C_RECEIVE\n";
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

void SCTPServer::finish()
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

SCTPServer::~SCTPServer()
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

SCTPServer::SCTPServer()
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

