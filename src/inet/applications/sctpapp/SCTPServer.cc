//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
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
        numSessions = packetsSent = packetsRcvd = bytesSent = notificationsReceived = 0;
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
        lastStream = 0;
        //abort = NULL;
        //abortSent = false;
        timeoutMsg = new cMessage("SrvAppTimer");
        delayTimer = new cMessage("delayTimer");
        delayTimer->setContextPointer(this);
        delayFirstReadTimer = new cMessage("delayFirstReadTimer");
        firstData = true;

        echo = par("echo");
        delay = par("echoDelay");
        delayFirstRead = par("delayFirstRead");
        cPar *delT = &par("readingInterval");
        if (delT->isNumeric() && (double)*delT == 0)
            readInt = false;
        else
            readInt = true;
        schedule = false;
        shutdownReceived = false;
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

void SCTPServer::sendOrSchedule(cPacket *msg)
{
    if (delay == 0)
        send(msg, "sctpOut");
    else
        scheduleAt(simulation.getSimTime() + delay, msg);
}

void SCTPServer::generateAndSend()
{
    cPacket *cmsg = new cPacket("CMSG");
    SCTPSimpleMessage *msg = new SCTPSimpleMessage("Server");
    int numBytes = par("requestLength");
    msg->setDataArraySize(numBytes);

    for (int i = 0; i < numBytes; i++)
        msg->setData(i, 's');

    msg->setDataLen(numBytes);
    msg->setEncaps(false);
    msg->setBitLength(numBytes * 8);
    cmsg->encapsulate(msg);
    SCTPSendCommand *cmd = new SCTPSendCommand("Send1");
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

cPacket *SCTPServer::makeReceiveRequest(cPacket *msg)
{
    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
    cPacket *cmsg = new cPacket("ReceiveRequest");
    SCTPSendCommand *cmd = new SCTPSendCommand("Send2");
    cmd->setAssocId(ind->getAssocId());
    cmd->setSid(ind->getSid());
    cmd->setNumMsgs(ind->getNumMsgs());
    cmsg->setKind(SCTP_C_RECEIVE);
    cmsg->setControlInfo(cmd);
    delete ind;
    return cmsg;
}

cPacket *SCTPServer::makeDefaultReceive()
{
    cPacket *cmsg = new cPacket("DefaultReceive");
    SCTPSendCommand *cmd = new SCTPSendCommand("Send3");
    cmd->setAssocId(assocId);
    cmd->setSid(0);
    cmd->setNumMsgs(1);
    cmsg->setKind(SCTP_C_RECEIVE);
    cmsg->setControlInfo(cmd);
    return cmsg;
}

cPacket *SCTPServer::makeAbortNotification(SCTPCommand *msg)
{
    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg);
    cPacket *cmsg = new cPacket("AbortNotification");
    SCTPSendCommand *cmd = new SCTPSendCommand("Send4");
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
    cPacket *cmsg;
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
                    ServerAssocStatMap::iterator i = serverAssocStatMap.find(assocId);
                    numRequestsToSend = i->second.sentPackets;
                    if (par("thinkTime").doubleValue() > 0) {
                        generateAndSend();
                        timeoutMsg->setKind(SCTP_C_SEND);
                        scheduleAt(simulation.getSimTime() + par("thinkTime"), timeoutMsg);
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

                            cPacket *cmsg = new cPacket("Queue");
                            SCTPInfo *qinfo = new SCTPInfo("Info1");
                            qinfo->setText(queueSize);
                            cmsg->setKind(SCTP_C_QUEUE_MSGS_LIMIT);
                            qinfo->setAssocId(id);
                            cmsg->setControlInfo(qinfo);
                            sendOrSchedule(cmsg);
                        }
                        ServerAssocStatMap::iterator j = serverAssocStatMap.find(assocId);
                        if (j->second.rcvdPackets == 0 && par("waitToClose").doubleValue() > 0) {
                            char as[5];
                            sprintf(as, "%d", assocId);
                            cPacket *abortMsg = new cPacket(as);
                            abortMsg->setKind(SCTP_I_ABORT);
                            scheduleAt(simulation.getSimTime() + par("waitToClose"), abortMsg);
                        }
                        else {
                            EV_INFO << "no more packets to send, call shutdown for assoc " << assocId << "\n";
                            cPacket *cmsg = new cPacket("ShutdownRequest");
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
                        cmsg = makeReceiveRequest(PK(msg));
                        scheduleAt(simulation.getSimTime() + delayFirstRead, cmsg);
                        scheduleAt(simulation.getSimTime() + delayFirstRead, delayFirstReadTimer);
                    }
                    else if (readInt && firstData) {
                        firstData = false;
                        cmsg = makeReceiveRequest(PK(msg));
                        scheduleAt(simulation.getSimTime() + par("readingInterval"), delayTimer);
                        sendOrSchedule(cmsg);
                    }
                    else if (delayFirstRead == 0 && readInt == false) {
                        cmsg = makeReceiveRequest(PK(msg));
                        sendOrSchedule(cmsg);
                    }
                }
                else {
                    EV_INFO << simulation.getSimTime() << " makeReceiveRequest\n";
                    cmsg = makeReceiveRequest(PK(msg));
                    sendOrSchedule(cmsg);
                }
                delete msg;
                break;
            }

            case SCTP_I_DATA: {
                notificationsReceived--;
                packetsRcvd++;
                EV_INFO << simulation.getSimTime() << " server: data arrived. " << packetsRcvd << " Packets received now\n";
                SCTPRcvCommand *ind = check_and_cast<SCTPRcvCommand *>(msg->removeControlInfo());
                id = ind->getAssocId();
                ServerAssocStatMap::iterator j = serverAssocStatMap.find(id);
                BytesPerAssoc::iterator k = bytesPerAssoc.find(id);
                if (j->second.rcvdBytes == 0)
                    j->second.start = simulation.getSimTime();

                j->second.rcvdBytes += PK(msg)->getByteLength();
                k->second->record(j->second.rcvdBytes);

                if (!echo) {
                    if (par("numPacketsToReceivePerClient").longValue() > 0) {
                        j->second.rcvdPackets--;
                        SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>(msg);
                        EndToEndDelay::iterator m = endToEndDelay.find(id);
                        m->second->record(simulation.getSimTime() - smsg->getCreationTime());
                        EV_INFO << "server: Data received. Left packets to receive=" << j->second.rcvdPackets << "\n";

                        if (j->second.rcvdPackets == 0) {
                            if (serverAssocStatMap[assocId].peerClosed == true && serverAssocStatMap[assocId].abortSent == false) {
                                sendOrSchedule(makeAbortNotification(ind));
                                serverAssocStatMap[assocId].abortSent = true;
                                j->second.stop = simulation.getSimTime();
                                j->second.lifeTime = j->second.stop - j->second.start;
                                break;
                            }
                            else {
                                cPacket *cmsg = new cPacket("Request");
                                SCTPInfo *qinfo = new SCTPInfo("Info2");
                                cmsg->setKind(SCTP_C_NO_OUTSTANDING);
                                qinfo->setAssocId(id);
                                cmsg->setControlInfo(qinfo);
                                sendOrSchedule(cmsg);
                                j->second.stop = simulation.getSimTime();
                                j->second.lifeTime = j->second.stop - j->second.start;
                            }
                        }
                    }
                    delete msg;
                }
                else {
                    SCTPSendCommand *cmd = new SCTPSendCommand("Send6");
                    cmd->setAssocId(id);
                    SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>(msg->dup());
                    EndToEndDelay::iterator n = endToEndDelay.find(id);
                    n->second->record(simulation.getSimTime() - smsg->getCreationTime());
                    cPacket *cmsg = new cPacket("SVData");
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
                delete ind;
                break;
            }

            case SCTP_I_SHUTDOWN_RECEIVED: {
                SCTPCommand *command = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
                id = command->getAssocId();
                EV_INFO << "server: SCTP_I_SHUTDOWN_RECEIVED for assoc " << id << "\n";
                ServerAssocStatMap::iterator i = serverAssocStatMap.find(id);
                if (i->second.sentPackets == 0 || par("numPacketsToSendPerClient").longValue() == 0) {
                    cPacket *cmsg = new cPacket("Request");
                    SCTPInfo *qinfo = new SCTPInfo("Info3");
                    cmsg->setKind(SCTP_C_NO_OUTSTANDING);
                    qinfo->setAssocId(id);
                    cmsg->setControlInfo(qinfo);
                    sendOrSchedule(cmsg);
                    i->second.stop = simulation.getSimTime();
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

            case SCTP_I_CLOSED:
                if (delayTimer->isScheduled())
                    cancelEvent(delayTimer);
                if (finishEndsSimulation)
                    endSimulation();
                delete msg;
                break;

            default:
                delete msg;
                break;
        }
    }
}

void SCTPServer::handleTimer(cMessage *msg)
{
    if (msg == delayTimer) {
        EV_INFO << simulation.getSimTime() << " delayTimer expired\n";
        sendOrSchedule(makeDefaultReceive());
        scheduleAt(simulation.getSimTime() + par("readingInterval"), delayTimer);
        return;
    }
    else if (msg == delayFirstReadTimer) {
        delayFirstRead = 0;

        if (readInt && !delayTimer->isScheduled()) {
            double tempInterval = par("readingInterval");
            scheduleAt(simulation.getSimTime() + (simtime_t)tempInterval, delayTimer);
            scheduleAt(simulation.getSimTime() + (simtime_t)tempInterval, makeDefaultReceive());
        }
        return;
    }

    switch (msg->getKind()) {
        case SCTP_C_SEND:
            if (numRequestsToSend > 0) {
                generateAndSend();
                if (par("thinkTime").doubleValue() > 0)
                    scheduleAt(simulation.getSimTime() + par("thinkTime"), timeoutMsg);
                numRequestsToSend--;
            }
            break;

        case SCTP_I_ABORT: {
            cPacket *cmsg = new cPacket("CLOSE", SCTP_C_CLOSE);
            SCTPCommand *cmd = new SCTPCommand("Send6");
            int id = atoi(msg->getName());
            cmd->setAssocId(id);
            cmsg->setControlInfo(cmd);
            sendOrSchedule(cmsg);
        }
        break;

        case SCTP_C_RECEIVE:
            EV_INFO << simulation.getSimTime() << " SCTPServer:SCTP_C_RECEIVE\n";
            if (readInt || delayFirstRead > 0)
                schedule = false;
            else
                schedule = true;
            sendOrSchedule(PK(msg));
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
    for (ServerAssocStatMap::iterator l = serverAssocStatMap.begin(); l != serverAssocStatMap.end(); ++l) {
        EV_DETAIL << getFullPath() << " Assoc: " << l->first << "\n";
        EV_DETAIL << "\tstart time: " << l->second.start << "\n";
        EV_DETAIL << "\tstop time: " << l->second.stop << "\n";
        EV_DETAIL << "\tlife time: " << l->second.lifeTime << "\n";
        EV_DETAIL << "\treceived bytes:" << l->second.rcvdBytes << "\n";
        EV_DETAIL << "\tthroughput: " << (l->second.rcvdBytes / l->second.lifeTime.dbl()) * 8 << " bit/sec\n";
        recordScalar("bytes rcvd", l->second.rcvdBytes);
        recordScalar("throughput", (l->second.rcvdBytes / l->second.lifeTime.dbl()) * 8);
    }
    EV_INFO << getFullPath() << "Over all " << packetsRcvd << " packets received\n ";
    EV_INFO << getFullPath() << "Over all " << notificationsReceived << " notifications received\n ";
    EV_INFO << "Server finished\n";
}

SCTPServer::~SCTPServer()
{
    for (BytesPerAssoc::iterator i = bytesPerAssoc.begin(); i != bytesPerAssoc.end(); ++i)
        delete i->second;

    for (EndToEndDelay::iterator i = endToEndDelay.begin(); i != endToEndDelay.end(); ++i)
        delete i->second;

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
    timeoutMsg = NULL;
    socket = NULL;
    delayFirstReadTimer = NULL;
    delayTimer = NULL;
}

} // namespace inet

