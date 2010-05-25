//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
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


#include "SCTPServer.h"
#include "SCTPSocket.h"
#include "SCTPCommand_m.h"
#include "SCTPMessage_m.h"
#include <stdlib.h>
#include <stdio.h>
#include "SCTPAssociation.h"

#define MSGKIND_CONNECT  0
#define MSGKIND_SEND         1
#define MSGKIND_      2

Define_Module(SCTPServer);

void SCTPServer::initialize()
{
    char * token;
    cPar *delT;
    AddressVector addresses;
    socket = NULL;
    sctpEV3<<"initialize SCTP Server\n";
    numSessions = packetsSent = packetsRcvd = bytesSent = notifications = 0;
    WATCH(numSessions);
    WATCH(packetsSent);
    WATCH(packetsRcvd);
    WATCH(bytesSent);
    WATCH(numRequestsToSend);
    // parameters
    finishEndsSimulation = (bool)par("finishEndsSimulation");
    const char* address = par("address");
    token = strtok((char*)address,",");
    while (token != NULL)
    {
        addresses.push_back(IPvXAddress(token));
        token = strtok(NULL, ",");
    }
    int32 port = par("port");
    echoFactor = par("echoFactor");
    delay = par("echoDelay");
    delayFirstRead = par("delayFirstRead");
    delT = &par("readingInterval");
    if (delT->isNumeric() && (double)*delT==0)
        readInt=false;
    else
        readInt=true;
    int32 messagesToPush = par("messagesToPush");
    inboundStreams = par("inboundStreams");
    outboundStreams = par("outboundStreams");
    ordered = (bool)par("ordered");
    queueSize = par("queueSize");
    lastStream = 0;
    //abort = NULL;
    //abortSent = false;
    timeoutMsg = new cMessage("SrvAppTimer");
    delayTimer = new cMessage("delayTimer");
    delayTimer->setContextPointer(this);
    delayFirstReadTimer = new cMessage("delayFirstReadTimer");
    firstData = true;
    SCTPSocket *socket = new SCTPSocket();
    socket->setOutputGate(gate("sctpOut"));
    socket->setInboundStreams(inboundStreams);
    socket->setOutboundStreams(outboundStreams);
    if (strcmp(address,"")==0)
        socket->bind(port);
    else
    {
        socket->bindx(addresses, port);
    }
    socket->listen(true, par("numPacketsToSendPerClient"), messagesToPush);
    sctpEV3<<"SCTPServer::initialized listen port="<<port<<"\n";
    schedule = false;
    shutdownReceived = false;
}

void SCTPServer::sendOrSchedule(cPacket *msg)
{
    if (delay==0)
    {
        send(msg, "sctpOut");
    }
    else
    {
        scheduleAt(simulation.getSimTime()+delay, msg);
    }
}

void SCTPServer::generateAndSend()
{
uint32 numBytes;

    cPacket* cmsg = new cPacket("CMSG");
    SCTPSimpleMessage* msg = new SCTPSimpleMessage("Server");
    numBytes = (uint32)par("requestLength");
    msg->setDataArraySize(numBytes);
    for (uint32 i=0; i<numBytes; i++)
    {
        msg->setData(i, 's');
    }
    msg->setDataLen(numBytes);
    msg->setBitLength(numBytes * 8);
    cmsg->encapsulate(msg);
    SCTPSendCommand *cmd = new SCTPSendCommand("Send1");
    cmd->setAssocId(assocId);
    if (ordered)
        cmd->setSendUnordered(COMPLETE_MESG_ORDERED);
    else
        cmd->setSendUnordered(COMPLETE_MESG_UNORDERED);
    lastStream=(lastStream+1)%outboundStreams;
    cmd->setSid(lastStream);
    if (queueSize>0 && numRequestsToSend > 0 && count < queueSize*2)
        cmd->setLast(false);
    else
        cmd->setLast(true);
    cmsg->setKind(SCTP_C_SEND);
    cmsg->setControlInfo(cmd);
    packetsSent++;
    bytesSent+=msg->getBitLength()/8;
    sendOrSchedule(cmsg);
}

cPacket* SCTPServer::makeReceiveRequest(cPacket* msg)
{
    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
    cPacket* cmsg = new cPacket("ReceiveRequest");
    SCTPSendCommand *cmd = new SCTPSendCommand("Send2");
    cmd->setAssocId(ind->getAssocId());
    cmd->setSid(ind->getSid());
    cmd->setNumMsgs(ind->getNumMsgs());
    cmsg->setKind(SCTP_C_RECEIVE);
    cmsg->setControlInfo(cmd);
    delete ind;
    return cmsg;
}

cPacket* SCTPServer::makeDefaultReceive()
{
    cPacket* cmsg = new cPacket("DefaultReceive");
    SCTPSendCommand *cmd = new SCTPSendCommand("Send3");
    cmd->setAssocId(assocId);
    cmd->setSid(0);
    cmd->setNumMsgs(1);
    cmsg->setKind(SCTP_C_RECEIVE);
    cmsg->setControlInfo(cmd);
    return cmsg;
}

cPacket* SCTPServer::makeAbortNotification(SCTPCommand* msg)
{
    SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg);
    cPacket* cmsg = new cPacket("AbortNotification");
    SCTPSendCommand *cmd = new SCTPSendCommand("Send4");
    assocId = ind->getAssocId();
    cmd->setAssocId(assocId);
    cmd->setSid(ind->getSid());
    cmd->setNumMsgs(ind->getNumMsgs());
    cmsg->setControlInfo(cmd);
    delete ind;
    //delete msg;
    cmsg->setKind(SCTP_C_ABORT);
    return cmsg;
}

void SCTPServer::handleMessage(cMessage *msg)
{
    int32 id;
    cPacket* cmsg;

    if (msg->isSelfMessage())
    {

        handleTimer(msg);
    }
    else
    {
    switch (msg->getKind())
    {
        case SCTP_I_PEER_CLOSED:
        case SCTP_I_ABORT:
        {
            SCTPCommand *command = dynamic_cast<SCTPCommand *>(msg->removeControlInfo());
            assocId = command->getAssocId();
            serverAssocStatMap[assocId].peerClosed = true;
            if ((long) par("numPacketsToReceivePerClient")==0)
            {
                if (serverAssocStatMap[assocId].abortSent==false)
                {
                    sendOrSchedule(makeAbortNotification(command->dup()));
                    serverAssocStatMap[assocId].abortSent = true;
                }
            }
            else
            {
                if (serverAssocStatMap[assocId].rcvdPackets==(unsigned long) par("numPacketsToReceivePerClient") &&
                    serverAssocStatMap[assocId].abortSent==false)
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
        case SCTP_I_ESTABLISHED:
        {
            count=0;
            SCTPConnectInfo *connectInfo = dynamic_cast<SCTPConnectInfo *>(msg->removeControlInfo());
            numSessions++;
            assocId = connectInfo->getAssocId();
            inboundStreams = connectInfo->getInboundStreams();
            outboundStreams = connectInfo->getOutboundStreams();
            serverAssocStatMap[assocId].rcvdPackets= (long) par("numPacketsToReceivePerClient");
            serverAssocStatMap[assocId].sentPackets= (long) par("numPacketsToSendPerClient");
            serverAssocStatMap[assocId].rcvdBytes=0;
            serverAssocStatMap[assocId].start=0;
            serverAssocStatMap[assocId].stop=0;
            serverAssocStatMap[assocId].lifeTime=0;
            serverAssocStatMap[assocId].abortSent=false;
            serverAssocStatMap[assocId].peerClosed = false;
            char text[30];
            sprintf(text, "App: Received Bytes of assoc %d",assocId);
            bytesPerAssoc[assocId] = new cOutVector(text);
            sprintf(text, "App: EndToEndDelay of assoc %d",assocId);
            endToEndDelay[assocId] = new cOutVector(text);

            delete connectInfo;
            delete msg;
            if ((long) par("numPacketsToSendPerClient") > 0)
            {
                ServerAssocStatMap::iterator i = serverAssocStatMap.find(assocId);
                numRequestsToSend = i->second.sentPackets;
                if ((simtime_t)par("thinkTime") > 0)
                {
                    generateAndSend();
                    timeoutMsg->setKind(SCTP_C_SEND);
                    scheduleAt(simulation.getSimTime()+(simtime_t)par("thinkTime"), timeoutMsg);
                    numRequestsToSend--;
                    i->second.sentPackets = numRequestsToSend;
                }
                else
                {
                    if (queueSize==0)
                    {
                        while (numRequestsToSend > 0)
                        {
                            generateAndSend();
                            numRequestsToSend--;
                            i->second.sentPackets = numRequestsToSend;
                        }
                    }
                    else if (queueSize>0)
                    {
                        while (numRequestsToSend > 0 && count++ < queueSize*2)
                        {
                            generateAndSend();
                            numRequestsToSend--;
                            i->second.sentPackets = numRequestsToSend;
                        }

                        cPacket* cmsg = new cPacket("Queue");
                        SCTPInfo* qinfo = new SCTPInfo("Info1");
                        qinfo->setText(queueSize);
                        cmsg->setKind(SCTP_C_QUEUE_MSGS_LIMIT);
                        qinfo->setAssocId(id);
                        cmsg->setControlInfo(qinfo);
                        sendOrSchedule(cmsg);
                    }
                    ServerAssocStatMap::iterator j=serverAssocStatMap.find(assocId);
                    if (j->second.rcvdPackets == 0 && (simtime_t)par("waitToClose")>0)
                    {
                        char as[5];
                        sprintf(as, "%d",assocId);
                        cPacket* abortMsg = new cPacket(as);
                        abortMsg->setKind(SCTP_I_ABORT);
                        scheduleAt(simulation.getSimTime()+(simtime_t)par("waitToClose"), abortMsg);
                    }
                    else
                    {
                        sctpEV3<<"no more packets to send, call shutdown for assoc "<<assocId<<"\n";
                        cPacket* cmsg = new cPacket("ShutdownRequest");
                        SCTPCommand* cmd = new SCTPCommand("Send5");
                        cmsg->setKind(SCTP_C_SHUTDOWN);
                        cmd->setAssocId(assocId);
                        cmsg->setControlInfo(cmd);
                        sendOrSchedule(cmsg);
                    }
                }
            }
            break;
        }
        case SCTP_I_DATA_NOTIFICATION:
        {
            notifications++;


            if (schedule==false)
            {
                if (delayFirstRead>0 && !delayFirstReadTimer->isScheduled())
                {

                    cmsg=makeReceiveRequest(PK(msg));
                    scheduleAt(simulation.getSimTime()+delayFirstRead, cmsg);
                    scheduleAt(simulation.getSimTime()+delayFirstRead, delayFirstReadTimer);
                }
                else if (readInt && firstData)
                {
                    firstData=false;
                    cmsg=makeReceiveRequest(PK(msg));
                    scheduleAt(simulation.getSimTime()+(simtime_t)par("readingInterval"), delayTimer);
                    sendOrSchedule(cmsg);
                }
                else if (delayFirstRead==0 && readInt==false)
                {
                    cmsg=makeReceiveRequest(PK(msg));
                    sendOrSchedule(cmsg);
                }

            }
            else
            {
                sctpEV3<<simulation.getSimTime()<<" makeReceiveRequest\n";
                cmsg=makeReceiveRequest(PK(msg));
                sendOrSchedule(cmsg);
            }
            delete msg;
            break;
        }
        case SCTP_I_DATA:
        {
            notifications--;
            packetsRcvd++;
            sctpEV3<<simulation.getSimTime()<<" server: data arrived. "<<packetsRcvd<<" Packets received now\n";
            SCTPRcvCommand *ind = check_and_cast<SCTPRcvCommand *>(msg->removeControlInfo());
            id = ind->getAssocId();
            ServerAssocStatMap::iterator j=serverAssocStatMap.find(id);
            BytesPerAssoc::iterator k=bytesPerAssoc.find(id);
            if (j->second.rcvdBytes == 0)
                j->second.start = simulation.getSimTime();

            j->second.rcvdBytes+= PK(msg)->getByteLength();
            k->second->record(j->second.rcvdBytes);

            if (echoFactor==0)
            {
                if ((uint32)par("numPacketsToReceivePerClient")>0)
                {
                    j->second.rcvdPackets--;
                    SCTPSimpleMessage *smsg=check_and_cast<SCTPSimpleMessage*>(msg);
                    EndToEndDelay::iterator m=endToEndDelay.find(id);
                    m->second->record(simulation.getSimTime()-smsg->getCreationTime());
                    sctpEV3<<"server: Data received. Left packets to receive="<<j->second.rcvdPackets<<"\n";

                    if (j->second.rcvdPackets == 0)
                    {
                        if (serverAssocStatMap[assocId].peerClosed==true && serverAssocStatMap[assocId].abortSent==false)
                        {
                            sendOrSchedule(makeAbortNotification(ind));
                            serverAssocStatMap[assocId].abortSent = true;
                            j->second.stop = simulation.getSimTime();
                            j->second.lifeTime = j->second.stop - j->second.start;
                            break;
                        }
                        else
                        {
                            cPacket* cmsg = new cPacket("Request");
                            SCTPInfo* qinfo = new SCTPInfo("Info2");
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
            else
            {
                SCTPSendCommand *cmd = new SCTPSendCommand("Send6");
                cmd->setAssocId(id);
                SCTPSimpleMessage *smsg=check_and_cast<SCTPSimpleMessage*>(msg->dup());
                EndToEndDelay::iterator n=endToEndDelay.find(id);
                n->second->record(simulation.getSimTime()-smsg->getCreationTime());
                cPacket* cmsg = new cPacket("SVData");
                bytesSent+=smsg->getBitLength()/8;
                cmd->setSendUnordered(cmd->getSendUnordered());
                lastStream=(lastStream+1)%outboundStreams;
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
        case SCTP_I_SHUTDOWN_RECEIVED:
        {
            SCTPCommand *command = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
            id = command->getAssocId();
            sctpEV3<<"server: SCTP_I_SHUTDOWN_RECEIVED for assoc "<<id<<"\n";
            ServerAssocStatMap::iterator i=serverAssocStatMap.find(id);
            if (i->second.sentPackets == 0 || (long) par("numPacketsToSendPerClient")==0)
            {
                cPacket* cmsg = new cPacket("Request");
                SCTPInfo* qinfo = new SCTPInfo("Info3");
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
        case SCTP_I_CLOSED:
            if (delayTimer->isScheduled())
                cancelEvent(delayTimer);
            if (finishEndsSimulation) {
                endSimulation();
            }
            delete msg;
        break;
        default: delete msg;
    }
    }
}

void SCTPServer::handleTimer(cMessage *msg)
{
    cPacket* cmsg;
    SCTPCommand* cmd;
    int32 id;
    double tempInterval;

    if (msg==delayTimer)
    {
        ServerAssocStatMap::iterator i=serverAssocStatMap.find(assocId);
        sctpEV3<<simulation.getSimTime()<<" delayTimer expired\n";
        sendOrSchedule(makeDefaultReceive());
        scheduleAt(simulation.getSimTime()+(double)par("readingInterval"), delayTimer);
        return;
    }
    else if (msg==delayFirstReadTimer)
    {
        delayFirstRead = 0;

        if (readInt && !delayTimer->isScheduled())
        {
            tempInterval = (double)par("readingInterval");
            scheduleAt(simulation.getSimTime()+(simtime_t)tempInterval, delayTimer);
            scheduleAt(simulation.getSimTime()+(simtime_t)tempInterval, makeDefaultReceive());
        }
        return;
    }

    switch (msg->getKind())
    {
    case SCTP_C_SEND:
        if (numRequestsToSend>0)
        {
            generateAndSend();
            if ((simtime_t)par("thinkTime") > 0)
                scheduleAt(simulation.getSimTime()+(simtime_t)par("thinkTime"), timeoutMsg);
            numRequestsToSend--;
        }
        break;
    case SCTP_I_ABORT:

        cmsg = new cPacket("CLOSE", SCTP_C_CLOSE);
        cmd = new SCTPCommand("Send6");
        id = atoi(msg->getName());
              cmd->setAssocId(id);
        cmsg->setControlInfo(cmd);
        sendOrSchedule(cmsg);
        break;
    case SCTP_C_RECEIVE:
        sctpEV3<<simulation.getSimTime()<<" SCTPServer:SCTP_C_RECEIVE\n";
        if (readInt || delayFirstRead > 0)
            schedule = false;
        else
            schedule = true;
        sendOrSchedule(PK(msg));
        break;
    default:

        sctpEV3<<"MsgKind ="<<msg->getKind()<<" unknown\n";

        break;
    }
}

void SCTPServer::finish()
{
    delete timeoutMsg;
    if (delayTimer->isScheduled())
        cancelEvent(delayTimer);
    delete delayTimer;
    delete delayFirstReadTimer;

        ev << getFullPath() << ": opened " << numSessions << " sessions\n";
    ev << getFullPath() << ": sent " << bytesSent << " bytes in " << packetsSent << " packets\n";
    for (ServerAssocStatMap::iterator l=serverAssocStatMap.begin(); l!=serverAssocStatMap.end(); l++)
    {
        ev << getFullPath() << " Assoc: "<<l->first<<"\n";
        ev << "\tstart time: "<<l->second.start <<"\n";
        ev << "\tstop time: "<<l->second.stop <<"\n";
        ev << "\tlife time: "<<l->second.lifeTime <<"\n";
        ev << "\treceived bytes:" << l->second.rcvdBytes << "\n";
        ev << "\tthroughput: "<<(l->second.rcvdBytes / l->second.lifeTime.dbl())*8 <<" bit/sec\n";
        recordScalar("bytes rcvd", l->second.rcvdBytes);
        recordScalar("throughput", (l->second.rcvdBytes / l->second.lifeTime.dbl())*8);

    }
    ev << getFullPath() << "Over all " << packetsRcvd << " packets received\n ";
    ev << getFullPath() << "Over all " << notifications << " notifications received\n ";

    BytesPerAssoc::iterator j;
    while ((j = bytesPerAssoc.begin())!= bytesPerAssoc.end())
    {
        delete j->second;
        bytesPerAssoc.erase(j);
    }
    EndToEndDelay::iterator k;
    while ((k = endToEndDelay.begin())!= endToEndDelay.end())
    {
        delete k->second;
        endToEndDelay.erase(k);
    }
    serverAssocStatMap.clear();
    sctpEV3<<"Server finished\n";
}

SCTPServer::~SCTPServer()
{
    delete socket;
}
