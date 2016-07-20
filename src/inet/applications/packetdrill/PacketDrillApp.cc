//
// Copyright (C) 2015 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <stdlib.h>
#include <stdio.h>

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "PacketDrillUtils.h"
#include "PacketDrillApp.h"
#include "PacketDrillInfo_m.h"
#include "inet/transportlayer/udp/UDPPacket_m.h"
#include "inet/networklayer/ipv4/IPv4Datagram_m.h"
#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"
#include "inet/transportlayer/sctp/SCTPAssociation.h"

Define_Module(PacketDrillApp);

namespace inet {

using namespace tcp;

#define MSGKIND_START  0
#define MSGKIND_EVENT  1

PacketDrillApp::PacketDrillApp()
{
    localPort = 1000;
    remotePort = 2000;
    protocol = 0;
    idInbound = 0;
    idOutbound = 0;
    relSequenceIn = 0;
    relSequenceOut = 0;
    peerTS = 0;
    peerWindow = 0;
    localAddress = L3Address("127.0.0.1");
    remoteAddress = L3Address("127.0.0.1");
}

void PacketDrillApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // parameters
        msgArrived = false;
        recvFromSet = false;
        listenSet = false;
        acceptSet = false;
        establishedPending = false;
        socketOptionsArrived = false;
        abortSent = false;
        receivedPackets = new cPacketQueue("receiveQueue");
        outboundPackets = new cPacketQueue("outboundPackets");
        expectedMessageSize = 0;
        eventCounter = 0;
        numEvents = 0;
        localVTag = 0;
        eventTimer = new cMessage("event timer");
        eventTimer->setKind(MSGKIND_EVENT);
        simStartTime = simTime();
        simRelTime = simTime();
    } else if (stage == INITSTAGE_APPLICATION_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
        pd = new PacketDrill(this);
        config = new PacketDrillConfig();
        script = new PacketDrillScript((const char *)(par("scriptFile")));
        localAddress = L3Address(par("localAddress"));
        remoteAddress = L3Address(par("remoteAddress"));
        localPort = par("localPort");
        remotePort = par("remotePort");

        cMessage* timeMsg = new cMessage("PacketDrillAppTimer");
        timeMsg->setKind(MSGKIND_START);
        scheduleAt((simtime_t)par("startTime"), timeMsg);
    }
}


void PacketDrillApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleTimer(msg);
    } else {
        if (msg->getArrivalGate()->isName("tunIn")) {
            if (outboundPackets->getLength() == 0) {
                cEvent *nextMsg = getSimulation()->getScheduler()->guessNextEvent();
                if (nextMsg) {
                    if ((simTime() + par("latency")) < nextMsg->getArrivalTime()) {
                        delete (PacketDrillInfo *)msg->getContextPointer();
                        delete msg;
                        throw cTerminationException("Packetdrill error: Packet arrived at the wrong time");
                    } else {
                        PacketDrillInfo *info = new PacketDrillInfo();
                        info->setLiveTime(getSimulation()->getSimTime());
                        msg->setContextPointer(info);
                        receivedPackets->insert(PK(msg));
                    }
                }
            } else {
                IPv4Datagram *datagram = check_and_cast<IPv4Datagram *>(outboundPackets->pop());
                IPv4Datagram *live = check_and_cast<IPv4Datagram*>(msg);
                PacketDrillInfo *info = (PacketDrillInfo *)datagram->getContextPointer();
                if (verifyTime((enum eventTime_t) info->getTimeType(), info->getScriptTime(),
                        info->getScriptTimeEnd(), info->getOffset(), getSimulation()->getSimTime(), "outbound packet")
                        == STATUS_ERR) {
                    delete info;
                    delete msg;
                    throw cTerminationException("Packetdrill error: Packet arrived at the wrong time");
                }
                if (!compareDatagram(datagram, live)) {
                    delete (PacketDrillInfo *)msg->getContextPointer();
                    delete msg;
                    throw cTerminationException("Packetdrill error: Datagrams are not the same");
                }
                delete info;
                if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
                    eventCounter++;
                    scheduleEvent();
                }
                delete (PacketDrillInfo *)msg->getContextPointer();
                delete msg;
                delete datagram;
            }
        } else if (msg->getArrivalGate()->isName("udpIn")) {
            PacketDrillEvent *event = (PacketDrillEvent *)(script->getEventList()->get(eventCounter));
            if (verifyTime((enum eventTime_t) event->getTimeType(), event->getEventTime(), event->getEventTimeEnd(),
                    event->getEventOffset(), getSimulation()->getSimTime(), "inbound packet") == STATUS_ERR) {
                delete msg;
                throw cTerminationException("Packetdrill error: Packet arrived at the wrong time");
            }
            if (recvFromSet) {
                recvFromSet = false;
                msgArrived = false;
                if (!(PK(msg)->getByteLength() == expectedMessageSize)) {
                    delete msg;
                    throw cTerminationException("Packetdrill error: Received data has unexpected size");
                }
                if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
                    eventCounter++;
                    scheduleEvent();
                }
                delete msg;
            } else {
                PacketDrillInfo* info = new PacketDrillInfo();
                info->setLiveTime(getSimulation()->getSimTime());
                msg->setContextPointer(info);
                receivedPackets->insert(PK(msg));
                msgArrived = true;
                if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
                    eventCounter++;
                    scheduleEvent();
                }
            }
        } else if (msg->getArrivalGate()->isName("tcpIn")) {
            switch (msg->getKind()) {
                case TCP_I_ESTABLISHED:
                    if (listenSet) {
                        if (acceptSet) {
                            tcpSocket.setState(TCPSocket::CONNECTED);
                            tcpConnId = check_and_cast<TCPCommand *>(msg->getControlInfo())->getSocketId();
                            listenSet = false;
                            acceptSet = false;
                        } else {
                            tcpConnId = check_and_cast<TCPCommand *>(msg->getControlInfo())->getSocketId();
                            establishedPending = true;
                        }
                    } else {
                        tcpSocket.setState(TCPSocket::CONNECTED);
                        tcpConnId = check_and_cast<TCPCommand *>(msg->getControlInfo())->getSocketId();
                    }
                    delete msg;
                    break;
                case TCP_I_CLOSED:
                    delete msg;
                    break;
                case TCP_I_DATA_NOTIFICATION:
                    if (recvFromSet)
                    {
                        cMessage *msg = new cMessage("data request");
                        msg->setKind(TCP_C_READ);
                        TCPCommand *cmd = new TCPCommand();
                        cmd->setSocketId(tcpConnId);
                        msg->setControlInfo(cmd);
                        send(msg, "tcpOut");
                        recvFromSet = false;
                        // send a receive request to TCP
                    }
                    msgArrived = true;
                    delete msg;
                    break;
                default: EV_INFO << "Message kind not supported (yet)";
            }
        } else if (msg->getArrivalGate()->isName("sctpIn")) {
            switch (msg->getKind()) {
                case SCTP_I_SENDSOCKETOPTIONS: {
                    sctpSocket.setUserOptions((void*) (msg->getContextPointer()));
                    socketOptionsArrived = true;
                    if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
                        eventCounter++;
                        scheduleEvent();
                    }
                    delete msg;
                    return;
                }
                case SCTP_I_ESTABLISHED: {
                    if (listenSet) {
                        if (acceptSet) {
                            sctpSocket.setState(SCTPSocket::CONNECTED);
                            sctpAssocId = check_and_cast<SCTPCommand *>(msg->getControlInfo())->getSocketId();
                            listenSet = false;
                            acceptSet = false;
                        } else {
                            sctpAssocId = check_and_cast<SCTPCommand *>(msg->getControlInfo())->getSocketId();
                            establishedPending = true;
                        }
                    } else {
                        sctpSocket.setState(SCTPSocket::CONNECTED);
                        SCTPConnectInfo *connectInfo = check_and_cast<SCTPConnectInfo *>(msg->removeControlInfo());
                        sctpAssocId = connectInfo->getSocketId();
                        sctpSocket.setInboundStreams(connectInfo->getInboundStreams());
                        sctpSocket.setOutboundStreams(connectInfo->getOutboundStreams());
                        delete connectInfo;
                    }
                    break;
                }
                case SCTP_I_DATA_NOTIFICATION: {
                    if (recvFromSet) {
                        cPacket* cmsg = new cPacket("ReceiveRequest", SCTP_C_RECEIVE);
                        SCTPSendInfo *cmd = new SCTPSendInfo("Send2");
                        cmd->setSocketId(sctpAssocId);
                        cmd->setSid(0);
                        cmsg->setControlInfo(cmd);
                        send(cmsg, "sctpOut");
                        recvFromSet = false;
                    }
                    if (sctpSocket.getState() == SCTPSocket::CLOSED) {
                        sctpSocket.abort();
                        abortSent = true;
                    }
                    if (!abortSent)
                        msgArrived = true;
                    break;
                }
                case SCTP_I_DATA: {
                    PacketDrillEvent *event = (PacketDrillEvent *) (script->getEventList()->get(eventCounter));
                    if (verifyTime((enum eventTime_t) event->getTimeType(), event->getEventTime(), event->getEventTimeEnd(),
                            event->getEventOffset(), getSimulation()->getSimTime(), "inbound packet") == STATUS_ERR)
                    {
                        delete msg;
                        throw cTerminationException("Packetdrill error: Packet arrived at the wrong time");
                    }
                    if (!(PK(msg)->getByteLength() == expectedMessageSize)) {
                        throw cTerminationException("Packetdrill error: Delivered message has wrong size");
                    }
                    msgArrived = false;
                    recvFromSet = false;
                    if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
                        eventCounter++;
                        scheduleEvent();
                    }
                    break;
                }
                case SCTP_I_CLOSED: {
                    break;
                }
                default: printf("Msg kind %d not implemented\n", msg->getKind());
            }
            delete msg;
            return;
        } else {
            delete msg;
            throw cRuntimeError("Unknown gate");
        }
    }
}

void PacketDrillApp::adjustTimes(PacketDrillEvent *event)
{
    simtime_t offset, offsetLastEvent;
    if (event->getTimeType() == ANY_TIME ||
        event->getTimeType() == RELATIVE_TIME ||
        event->getTimeType() == RELATIVE_RANGE_TIME) {
        offset = getSimulation()->getSimTime() - simStartTime;
        offsetLastEvent = ((PacketDrillEvent *)(script->getEventList()->get(eventCounter - 1)))->getEventTime() - simStartTime;
        offset = (offset.dbl() > offsetLastEvent.dbl()) ? offset : offsetLastEvent;
        event->setEventOffset(offset);
        event->setEventTime(event->getEventTime() + offset + simStartTime);
        if (event->getTimeType() == RELATIVE_RANGE_TIME) {
            event->setEventTimeEnd(event->getEventTimeEnd() + offset + simStartTime);
        }
    } else if (event->getTimeType() == ABSOLUTE_TIME) {
        event->setEventTime(event->getEventTime() + simStartTime);
    } else
        throw cRuntimeError("Unknown time type");
}

void PacketDrillApp::scheduleEvent()
{
    PacketDrillEvent *event = (PacketDrillEvent *)(script->getEventList()->get(eventCounter));
    event->setEventNumber(eventCounter);
    adjustTimes(event);
    eventTimer->setContextPointer(event);
    scheduleAt(event->getEventTime(), eventTimer);
}

void PacketDrillApp::runEvent(PacketDrillEvent* event)
{
    char str[128];
    if (event->getType() == PACKET_EVENT) {
        IPv4Datagram *ip = check_and_cast<IPv4Datagram *>(event->getPacket()->getInetPacket());
        if (event->getPacket()->getDirection() == DIRECTION_INBOUND) { // < injected packet, will go through the stack bottom up.
            if (protocol == IP_PROT_TCP) {
                TCPSegment* tcp = check_and_cast<TCPSegment*>(ip->decapsulate());
                tcp->setAckNo(tcp->getAckNo() + relSequenceOut);
                if (tcp->getHeaderOptionArraySize() > 0) {
                    for (unsigned int i = 0; i < tcp->getHeaderOptionArraySize(); i++) {
                        if (tcp->getHeaderOption(i)->getKind() == TCPOPT_TIMESTAMP) {
                            TCPOptionTimestamp *option = new TCPOptionTimestamp();
                            option->setEchoedTimestamp(peerTS);
                            tcp->setHeaderOption(i, option);
                        }
                    }
                }
                ip->encapsulate(tcp);
                snprintf(str, sizeof(str), "inbound %d", eventCounter);
                ip->setName(str);
            }
            if (protocol == IP_PROT_SCTP) {
                SCTPMessage* sctp = check_and_cast<SCTPMessage*>(ip->decapsulate());
                sctp->setTag(peerVTag);
                if (((SCTPChunk*) sctp->peekFirstChunk())->getChunkType() == INIT) {
                    SCTPInitChunk* init = check_and_cast<SCTPInitChunk*>(sctp->getChunks(0));
                    peerInStreams = init->getNoInStreams();
                    peerOutStreams = init->getNoOutStreams();
                    initPeerTsn = init->getInitTSN();
                    localVTag = init->getInitTag();
                    peerCumTsn = initPeerTsn - 1;
                }
                if (((SCTPChunk*) sctp->peekFirstChunk())->getChunkType() == INIT_ACK) {
                    SCTPInitAckChunk* initack = check_and_cast<SCTPInitAckChunk*>(sctp->getChunks(0));
                    localVTag = initack->getInitTag();
                    initPeerTsn = initack->getInitTSN();
                    peerCumTsn = initPeerTsn - 1;
                }
                if (((SCTPChunk*) sctp->peekFirstChunk())->getChunkType() == COOKIE_ECHO) {
                    SCTPCookieEchoChunk* cookieEcho = check_and_cast<SCTPCookieEchoChunk*>(sctp->getChunks(0));
                    int tempLength = cookieEcho->getByteLength();
                    cookieEcho->setStateCookie(peerCookie);
                    cookieEcho->setByteLength(SCTP_COOKIE_ACK_LENGTH + peerCookieLength);
                    sctp->setByteLength(sctp->getByteLength() - tempLength + cookieEcho->getByteLength());
                }
                if (((SCTPChunk*) sctp->peekFirstChunk())->getChunkType() == SACK)
                {
                    SCTPSackChunk* sack = check_and_cast<SCTPSackChunk*>(sctp->getChunks(0));
                    sack->setCumTsnAck(sack->getCumTsnAck() + initLocalTsn);
                    if (sack->getNumGaps() > 0) {
                        for (int i = 0; i < sack->getNumGaps(); i++) {
                            sack->setGapStart(i, sack->getGapStart(i) + initLocalTsn);
                            sack->setGapStop(i, sack->getGapStop(i) + initLocalTsn);
                        }
                    }
                    if (sack->getNumDupTsns() > 0)
                    {
                        for (int i = 0; i < sack->getNumDupTsns(); i++)
                        {
                            sack->setDupTsns(i, sack->getDupTsns(i) + initLocalTsn);
                        }
                    }
                    sctp->replaceChunk(sack, 0);
                }
                sctp->setName("inboundSctp");
                ip->encapsulate(sctp);
            }
            send(ip, "tunOut");
        } else if (event->getPacket()->getDirection() == DIRECTION_OUTBOUND) { // >
            if (receivedPackets->getLength() > 0) {
                IPv4Datagram *live = check_and_cast<IPv4Datagram *>(receivedPackets->pop());
                if (ip && live) {
                    PacketDrillInfo *liveInfo = (PacketDrillInfo *)live->getContextPointer();
                    if (verifyTime((enum eventTime_t) event->getTimeType(), event->getEventTime(),
                            event->getEventTimeEnd(), event->getEventOffset(), liveInfo->getLiveTime(),
                            "outbound packet") == STATUS_ERR) {
                        delete liveInfo;
                        delete live;
                        delete ip;
                        throw cTerminationException("Packetdrill error: Timing error");
                    }
                    delete liveInfo;
                    if (!compareDatagram(ip, live)) {
                        delete liveInfo;
                        throw cTerminationException("Packetdrill error: Datagrams are not the same");
                    }
                    if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
                        eventCounter++;
                        scheduleEvent();
                    }
                }
                delete live;
                delete ip;
            } else {
                if (protocol == IP_PROT_SCTP) {
                    SCTPMessage* sctp = check_and_cast<SCTPMessage*>(ip->getEncapsulatedPacket());
                    if (((SCTPChunk*) sctp->peekFirstChunk())->getChunkType() == INIT) {
                        SCTPInitChunk* init = check_and_cast<SCTPInitChunk*>(sctp->getChunks(0));
                        initLocalTsn = init->getInitTSN();
                        peerVTag = init->getInitTag();
                        localCumTsn = initLocalTsn - 1;
                        sctpSocket.setInboundStreams(init->getNoInStreams());
                        sctpSocket.setOutboundStreams(init->getNoOutStreams());
                    }
                    if (((SCTPChunk*) sctp->peekFirstChunk())->getChunkType() == INIT_ACK) {
                        SCTPInitAckChunk* initack = check_and_cast<SCTPInitAckChunk*>(sctp->getChunks(0));
                        initLocalTsn = initack->getInitTSN();
                        peerVTag = initack->getInitTag();
                        localCumTsn = initLocalTsn - 1;
                    }
                }
                PacketDrillInfo *info = new PacketDrillInfo("outbound");
                info->setScriptTime(event->getEventTime());
                info->setScriptTimeEnd(event->getEventTimeEnd());
                info->setOffset(event->getEventOffset());
                info->setTimeType(event->getTimeType());
                ip->setContextPointer(info);
                snprintf(str, sizeof(str), "outbound %d", eventCounter);
                ip->setName(str);
                outboundPackets->insert(ip);
            }
        } else
            throw cRuntimeError("Invalid direction");
    } else if (event->getType() == SYSCALL_EVENT) {
        EV_INFO << "syscallEvent: time_type = " << event->getTimeType() << " event time = " << event->getEventTime()
                << " end event time = " << event->getEventTimeEnd() << endl;
        runSystemCallEvent(event, event->getSyscall());
    }
}

void PacketDrillApp::handleTimer(cMessage *msg)
{
    switch (msg->getKind()) {
        case MSGKIND_START: {
            simStartTime = getSimulation()->getSimTime();
            simRelTime = simStartTime;
            if (script->parseScriptAndSetConfig(config, NULL)) {
                delete msg;
                throw cRuntimeError("Error parsing the script");
            }
            numEvents = script->getEventList()->getLength();
            scheduleEvent();
            delete msg;
            break;
        }

        case MSGKIND_EVENT: {
            PacketDrillEvent *event = (PacketDrillEvent *)msg->getContextPointer();
            runEvent(event);
            if ((socketOptionsArrived && !recvFromSet && outboundPackets->getLength() == 0) &&
                (!eventTimer->isScheduled() && eventCounter < numEvents - 1)) {
                eventCounter++;
                scheduleEvent();
            }
            break;
        }

        default:
            throw cRuntimeError("Unknown message kind");
    }
}

void PacketDrillApp::runSystemCallEvent(PacketDrillEvent* event, struct syscall_spec *syscall)
{
    char *error = NULL;
    const char *name = syscall->name;
    cQueue *args = new cQueue("systemCallArgs");
    int result = 0;

    // Evaluate script symbolic expressions to get live numeric args for system calls.

    if (pd->evaluateExpressionList(syscall->arguments, args, &error)) {
        free(error);
        return;
    }
    if (!strcmp(name, "socket")) {
        syscallSocket(syscall, args, &error);
    } else if (!strcmp(name, "bind")) {
        syscallBind(syscall, args, &error);
    } else if (!strcmp(name, "listen")) {
        syscallListen(syscall, args, &error);
    } else if (!strcmp(name, "write")) {
        syscallWrite(syscall, args, &error);
    } else if (!strcmp(name, "read")) {
        syscallRead((PacketDrillEvent*) event, syscall, args, &error);
    } else if (!strcmp(name, "sendto")) {
        syscallSendTo(syscall, args, &error);
    } else if (!strcmp(name, "recvfrom")) {
        syscallRecvFrom((PacketDrillEvent*)event, syscall, args, &error);
    } else if (!strcmp(name, "close")) {
        syscallClose(syscall, args, &error);
    } else if (!strcmp(name, "connect")) {
        syscallConnect(syscall, args, &error);
    } else if (!strcmp(name, "accept")) {
        syscallAccept(syscall, args, &error);
    } else if (!strcmp(name, "setsockopt")) {
        syscallSetsockopt(syscall, args, &error);
    } else {
        EV_INFO << "System call %s not known (yet)." << name;
    }

    delete(args);
    delete(syscall->arguments);

    if (result == STATUS_ERR) {
        EV_ERROR << event->getLineNumber() << ": runtime error in " << syscall->name << " call: " << error << endl;
        free(error);
    }
    return;
}

int PacketDrillApp::syscallSocket(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int type;
    PacketDrillExpression *exp;

    if (args->getLength() != 3) {
        return STATUS_ERR;
    }
    exp = (PacketDrillExpression *)args->get(0);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS)) {
        return STATUS_ERR;
    }
    exp = (PacketDrillExpression *)args->get(1);
    if (!exp || exp->getS32(&type, error)) {
        return STATUS_ERR;
    }
    exp = (PacketDrillExpression *)args->get(2);
    if (!exp || exp->getS32(&protocol, error)) {
        return STATUS_ERR;
    }

    switch (protocol) {
        case IP_PROT_UDP:
            udpSocket.setOutputGate(gate("udpOut"));
            udpSocket.bind(localPort);
            break;

        case IP_PROT_TCP:
            tcpSocket.readDataTransferModePar(*this);
            tcpSocket.setOutputGate(gate("tcpOut"));
            tcpSocket.bind(localPort);
            break;
        case IP_PROT_SCTP:
            sctpSocket.setOutputGate(gate("sctpOut"));
            if (sctpSocket.getOutboundStreams() == -1) {
                sctpSocket.setOutboundStreams((int) par("outboundStreams"));
            }
            if (sctpSocket.getInboundStreams() == -1) {
                sctpSocket.setInboundStreams((int) par("inboundStreams"));
            }
            sctpSocket.bind(localPort);
            break;
        default:
            throw cRuntimeError("Protocol type not supported for the socket system call");
    }

    return STATUS_OK;
}

int PacketDrillApp::syscallBind(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd;
    PacketDrillExpression *exp;

    if (args->getLength() != 3)
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(1);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(2);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;

    switch (protocol) {
        case IP_PROT_UDP:
            break;

        case IP_PROT_TCP:
            if (tcpSocket.getState() == TCPSocket::NOT_BOUND) {
                tcpSocket.bind(localAddress, localPort);
            }
            break;
        case IP_PROT_SCTP:
            if (sctpSocket.getState() == SCTPSocket::NOT_BOUND)
            {
                sctpSocket.bind(localPort);
            }
            break;
        default:
            throw cRuntimeError("Protocol type not supported for the bind system call");
    }
    return STATUS_OK;
}

int PacketDrillApp::syscallListen(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, backlog;
    PacketDrillExpression *exp;

    if (args->getLength() != 2)
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(1);
    if (!exp || exp->getS32(&backlog, error))
        return STATUS_ERR;

    switch (protocol) {
        case IP_PROT_UDP:
            break;

        case IP_PROT_TCP:
            listenSet = true;
            tcpSocket.listenOnce();
            break;
        case IP_PROT_SCTP: {
            sctpSocket.listen(0, true, 0, true);
            listenSet = true;
            break;
        }
        default:
            throw cRuntimeError("Protocol type not supported for the listen system call");
    }
    return STATUS_OK;
}

int PacketDrillApp::syscallAccept(struct syscall_spec *syscall, cQueue *args, char **error)
{
    if (!listenSet)
        return STATUS_ERR;

    if (establishedPending) {
        if (protocol == IP_PROT_TCP)
            tcpSocket.setState(TCPSocket::CONNECTED);
        else if (protocol == IP_PROT_SCTP)
            sctpSocket.setState(SCTPSocket::CONNECTED);
        establishedPending = false;
    } else {
        acceptSet = true;
    }

    return STATUS_OK;
}

int PacketDrillApp::syscallWrite(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, count;
    PacketDrillExpression* exp;

    if (args->getLength() != 3)
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(1);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(2);
    if (!exp || exp->getS32(&count, error))
        return STATUS_ERR;

    switch (protocol)
    {
        case IP_PROT_TCP: {
            cPacket *payload = new cPacket("Write");
            payload->setByteLength(syscall->result->getNum());
            tcpSocket.send(payload);
            break;
        }
        case IP_PROT_SCTP: {
            cPacket* cmsg = new cPacket("AppData");
            SCTPSimpleMessage* msg = new SCTPSimpleMessage("data");
            uint32 sendBytes = syscall->result->getNum();
            msg->setDataArraySize(sendBytes);
            for (uint32 i = 0; i < sendBytes; i++)
                msg->setData(i, 'a');

            msg->setDataLen(sendBytes);
            msg->setEncaps(false);
            msg->setByteLength(sendBytes);
            cmsg->encapsulate(msg);
            cmsg->setKind(SCTP_C_SEND_ORDERED);

            SCTPSendInfo* sendCommand = new SCTPSendInfo;
            sendCommand->setLast(true);
            cmsg->setControlInfo(sendCommand);

            sctpSocket.sendMsg(cmsg);
            break;
        }
        default: EV_INFO << "Protocol not supported for this socket call";
    }

    return STATUS_OK;
}


int PacketDrillApp::syscallConnect(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd;
    PacketDrillExpression *exp;

    if (args->getLength() != 3)
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(1);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(2);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;

    switch (protocol) {
        case IP_PROT_UDP:
            break;

        case IP_PROT_TCP:
            tcpSocket.connect(remoteAddress, remotePort);
            break;
        case IP_PROT_SCTP: {
            sctpSocket.connect(remoteAddress, remotePort, 0, true);
            break;
            }
        default:
            throw cRuntimeError("Protocol type not supported for the connect system call");
    }

    return STATUS_OK;
}


int PacketDrillApp::syscallSetsockopt(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, level, optname;
    PacketDrillExpression *exp;

    args->setName("syscallSetsockopt");
    assert(protocol == IP_PROT_SCTP);
    if (args->getLength() != 5)
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(1);
    if (!exp || exp->getS32(&level, error))
        return STATUS_ERR;
    if (level != IPPROTO_SCTP) {
        return STATUS_ERR;
    }
    exp = (PacketDrillExpression *) args->get(2);
    if (!exp || exp->getS32(&optname, error))
        return STATUS_ERR;

    exp = (PacketDrillExpression *) args->get(3);
    switch (exp->getType())
    {
        case EXPR_SCTP_INITMSG: {
            struct sctp_initmsg_expr *initmsg = exp->getInitmsg();
            sctpSocket.setOutboundStreams(initmsg->sinit_num_ostreams->getNum());
            sctpSocket.setInboundStreams(initmsg->sinit_max_instreams->getNum());
            sctpSocket.setMaxInitRetrans(initmsg->sinit_max_attempts->getNum());
            sctpSocket.setMaxInitRetransTimeout(initmsg->sinit_max_init_timeo->getNum());
            break;
        }
        case EXPR_SCTP_RTOINFO: {
            struct sctp_rtoinfo_expr *rtoinfo = exp->getRtoinfo();
            sctpSocket.setRtoInitial(rtoinfo->srto_initial->getNum() * 1.0 / 1000);
            sctpSocket.setRtoMax(rtoinfo->srto_max->getNum() * 1.0 / 1000);
            sctpSocket.setRtoMin(rtoinfo->srto_min->getNum() * 1.0 / 1000);
            break;
        }
        case EXPR_SCTP_SACKINFO: {
            struct sctp_sack_info_expr *sackinfo = exp->getSackinfo();
            sctpSocket.setSackPeriod(sackinfo->sack_delay->getNum() * 1.0 / 1000);
            sctpSocket.setSackFrequency(sackinfo->sack_freq->getNum());
            break;
        }
        case EXPR_SCTP_ASSOCVAL:
            switch (optname)
            {
                case SCTP_MAX_BURST: {
                    struct sctp_assoc_value_expr *burstvalue = exp->getAssocval();
                    sctpSocket.setMaxBurst(burstvalue->assoc_value->getNum());
                    break;
                }
                case SCTP_MAXSEG: {
                    struct sctp_assoc_value_expr *assocvalue = exp->getAssocval();
                    sctpSocket.setFragPoint(assocvalue->assoc_value->getNum());
                    break;
                }
            }
            break;
        case EXPR_LIST: {
            int value;

            if (!exp || exp->getType() != EXPR_LIST)
            {
                return STATUS_ERR;
            }
            if (exp->getList()->getLength() != 1)
            {
                printf("Expected [<integer>] but got multiple elements");
                return STATUS_ERR;
            }
            //PDExpression *exp2 = (PDExpression*)(exp->getList()->get(0));
            PacketDrillExpression *exp2 = (PacketDrillExpression*) (exp->getList()->pop());
            exp2->getS32(&value, error);
            sctpSocket.setNoDelay(value);
            delete exp2;
            break;
        }
        case EXPR_INTEGER:
            break;
        default:
            printf("Type %d not known\n", exp->getType());
    }
    return STATUS_OK;
}

int PacketDrillApp::syscallSendTo(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, count, flags;
    PacketDrillExpression *exp;

    if (args->getLength() != 6)
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(1);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(2);
    if (!exp || exp->getS32(&count, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(3);
    if (!exp || exp->getS32(&flags, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(4);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(5);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;

    cPacket *payload = new cPacket("SendTo");
    payload->setByteLength(count);

    switch (protocol) {
        case IP_PROT_UDP:
            udpSocket.sendTo(payload, remoteAddress, remotePort);
            break;

        default:
            throw cRuntimeError("Protocol type not supported for this system call");
    }

    return STATUS_OK;
}

int PacketDrillApp::syscallRead(PacketDrillEvent *event, struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, count;
    PacketDrillExpression* exp;

    if (args->getLength() != 3)
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(1);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *) args->get(2);
    if (!exp || exp->getS32(&count, error))
        return STATUS_ERR;

    if ((expectedMessageSize = syscall->result->getNum()) > 0) {
        if (msgArrived) {
            switch (protocol) {
                case IP_PROT_TCP: {
                    cMessage *msg = new cMessage("dataRequest");
                    msg->setKind(TCP_C_READ);
                    TCPCommand *tcpcmd = new TCPCommand();
                    tcpcmd->setSocketId(tcpConnId);
                    msg->setControlInfo(tcpcmd);
                    send(msg, "tcpOut");
                    break;
                }
                case IP_PROT_SCTP: {
                    cPacket* pkt = new cPacket("dataRequest", SCTP_C_RECEIVE);
                    SCTPSendInfo *sctpcmd = new SCTPSendInfo();
                    sctpcmd->setSocketId(sctpAssocId);
                    sctpcmd->setSid(0);
                    pkt->setControlInfo(sctpcmd);
                    send(pkt, "sctpOut");
                    break;
                }
                default: EV_INFO << "Protoocl not supported for this system call.";
            }
            msgArrived = false;
            expectedMessageSize = syscall->result->getNum();
            recvFromSet = true;
            // send a receive request to TCP
        }
        else
        {
            recvFromSet = true;
        }
    }
    else
    {
        if (msgArrived)
        {
            outboundPackets->pop();
            msgArrived = false;
        }
    }
    return STATUS_OK;
}

int PacketDrillApp::syscallRecvFrom(PacketDrillEvent *event, struct syscall_spec *syscall, cQueue *args, char **err)
{
    int script_fd, count, flags;
    PacketDrillExpression *exp;

    if (args->getLength() != 6)
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(0);
    if (!exp || exp->getS32(&script_fd, err))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(1);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(2);
    if (!exp || exp->getS32(&count, err))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(3);
    if (!exp || exp->getS32(&flags, err))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(4);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(5);
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;

    if (msgArrived) {
        cMessage *msg = (cMessage*)(receivedPackets->pop());
        msgArrived = false;
        recvFromSet = false;
        if (!(PK(msg)->getByteLength() == syscall->result->getNum())) {
            delete msg;
            throw cTerminationException("Packetdrill error: Wrong payload length");
        }
        PacketDrillInfo *info = (PacketDrillInfo *)msg->getContextPointer();
        if (verifyTime((enum eventTime_t) event->getTimeType(), event->getEventTime(), event->getEventTimeEnd(),
                event->getEventOffset(), info->getLiveTime(), "inbound packet") == STATUS_ERR) {
            delete info;
            delete msg;
            return false;
        }
        delete info;
        delete msg;
    } else {
        expectedMessageSize = syscall->result->getNum();
        recvFromSet = true;
    }
    return STATUS_OK;
}

int PacketDrillApp::syscallClose(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd;

    if (args->getLength() != 1)
        return STATUS_ERR;
    PacketDrillExpression *exp = (PacketDrillExpression *)args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;

    switch (protocol) {
        case IP_PROT_UDP: {
            EV_DETAIL << "close UDP socket\n";
            udpSocket.close();
            break;
        }

        case IP_PROT_TCP: {
            cMessage *msg = new cMessage("close");
            msg->setKind(TCP_C_CLOSE);
            TCPCommand *cmd = new TCPCommand();
            cmd->setSocketId(tcpConnId);
            msg->setControlInfo(cmd);
            send(msg, "tcpOut");
            break;
        }
        case IP_PROT_SCTP: {
            sctpSocket.close();
            break;
        }
        default:
            EV_INFO << "Protocol " << protocol << " is not supported for this system call\n";
    }

    return STATUS_OK;
}

void PacketDrillApp::finish()
{
    EV_INFO << "PacketDrillApp finished\n";
}

PacketDrillApp::~PacketDrillApp()
{
    delete eventTimer;
    delete script;
    delete pd;
    delete receivedPackets;
    delete outboundPackets;
    delete config;
}

// Verify that something happened at the expected time.

int PacketDrillApp::verifyTime(enum eventTime_t timeType, simtime_t scriptTime, simtime_t scriptTimeEnd, simtime_t offset,
        simtime_t liveTime, const char *description)
{
    simtime_t expectedTime = scriptTime;
    simtime_t expectedTimeEnd = scriptTimeEnd;
    simtime_t actualTime = liveTime;
    simtime_t tolerance = SimTime(config->getToleranceUsecs(), SIMTIME_US);

    if (timeType == ANY_TIME) {
        return STATUS_OK;
    }

    if (timeType == ABSOLUTE_RANGE_TIME || timeType == RELATIVE_RANGE_TIME) {
        if (actualTime < (expectedTime - tolerance) || actualTime > (expectedTimeEnd + tolerance)) {
            if (timeType == ABSOLUTE_RANGE_TIME) {
                EV_INFO << "timing error: expected " << description << " in time range " << scriptTime << " ~ "
                        << scriptTimeEnd << " sec, but happened at " << actualTime << " sec" << endl;
            } else if (timeType == RELATIVE_RANGE_TIME) {
                EV_INFO << "timing error: expected " << description << " in relative time range +"
                        << scriptTime - offset << " ~ " << scriptTimeEnd - offset << " sec, but happened at +"
                        << actualTime - offset << " sec" << endl;
            }
            return STATUS_ERR;
        } else {
            return STATUS_OK;
        }
    }

    if ((actualTime < (expectedTime - tolerance)) || (actualTime > (expectedTime + tolerance))) {
        EV_INFO << "timing error: expected " << description << " at " << scriptTime << " sec, but happened at "
                << actualTime << " sec" << endl;
        return STATUS_ERR;
    } else {
        return STATUS_OK;
    }
}

bool PacketDrillApp::compareDatagram(IPv4Datagram *storedDatagram, IPv4Datagram *liveDatagram)
{
    if (!(storedDatagram->getSrcAddress() == liveDatagram->getSrcAddress())) {
        return false;
    }
    if (!(storedDatagram->getDestAddress() == liveDatagram->getDestAddress())) {
        return false;
    }
    if (!(storedDatagram->getTransportProtocol() == liveDatagram->getTransportProtocol())) {
        return false;
    }
    if (!(storedDatagram->getTimeToLive() == liveDatagram->getTimeToLive())) {
        return false;
    }
    if (!(storedDatagram->getIdentification() == liveDatagram->getIdentification())){
        return false;
    }
    if (!(storedDatagram->getMoreFragments() == liveDatagram->getMoreFragments())) {
        return false;
    }
    if (!(storedDatagram->getDontFragment() == liveDatagram->getDontFragment())) {
        return false;
    }
    if (!(storedDatagram->getFragmentOffset() == liveDatagram->getFragmentOffset())) {
        return false;
    }
    if (!(storedDatagram->getTypeOfService() == liveDatagram->getTypeOfService())) {
        return false;
    }
    if (!(storedDatagram->getHeaderLength() == liveDatagram->getHeaderLength())) {
        return false;
    }
    switch (storedDatagram->getTransportProtocol()) {
        case IP_PROT_UDP: {
            UDPPacket *storedUdp = check_and_cast<UDPPacket *>(storedDatagram->getEncapsulatedPacket());
            UDPPacket *liveUdp = check_and_cast<UDPPacket *>(liveDatagram->getEncapsulatedPacket());
            if (!(compareUdpPacket(storedUdp, liveUdp))) {
                return false;
            }
            break;
        }

        case IP_PROT_TCP: {
            TCPSegment *storedTcp = check_and_cast<TCPSegment *>(storedDatagram->getEncapsulatedPacket());
            TCPSegment *liveTcp = check_and_cast<TCPSegment *>(liveDatagram->getEncapsulatedPacket());
            if (storedTcp->getSynBit()) { // SYN was sent. Store the sequence number for comparisons
                relSequenceOut = liveTcp->getSequenceNo();
            }
            if (storedTcp->getSynBit() && storedTcp->getAckBit()) {
                peerWindow = liveTcp->getWindow();
            }
            if (!(compareTcpPacket(storedTcp, liveTcp))) {
                return false;
            }
            break;
        }
        case IP_PROT_SCTP: {
            SCTPMessage *storedSctp = check_and_cast<SCTPMessage *>(storedDatagram->decapsulate());
            SCTPMessage *liveSctp = check_and_cast<SCTPMessage *>(liveDatagram->decapsulate());
            if (!(compareSctpPacket(storedSctp, liveSctp))) {
                return false;
            }
            delete storedSctp;
            delete liveSctp;
            break;
        }
        default: EV_INFO << "Transport protocol %d is not supported yet" << storedDatagram->getTransportProtocol();
    }
    return true;
}

bool PacketDrillApp::compareUdpPacket(UDPPacket *storedUdp, UDPPacket *liveUdp)
{
    return (storedUdp->getSourcePort() == liveUdp->getSourcePort()) &&
        (storedUdp->getDestinationPort() == liveUdp->getDestinationPort());
}

bool PacketDrillApp::compareTcpPacket(TCPSegment *storedTcp, TCPSegment *liveTcp)
{
    if (!(storedTcp->getSrcPort() == liveTcp->getSrcPort())) {
        return false;
    }
    if (!(storedTcp->getDestPort() == liveTcp->getDestPort())) {
        return false;
    }
    if (!(storedTcp->getSequenceNo() + relSequenceOut == liveTcp->getSequenceNo())) {
        return false;
    }
    if (!(storedTcp->getAckNo() == liveTcp->getAckNo())) {
        return false;
    }
    if (!(storedTcp->getUrgBit() == liveTcp->getUrgBit()) || !(storedTcp->getAckBit() == liveTcp->getAckBit()) ||
        !(storedTcp->getPshBit() == liveTcp->getPshBit()) || !(storedTcp->getRstBit() == liveTcp->getRstBit()) ||
        !(storedTcp->getSynBit() == liveTcp->getSynBit()) || !(storedTcp->getFinBit() == liveTcp->getFinBit())) {
        return false;
    }
    if (!(storedTcp->getUrgentPointer() == liveTcp->getUrgentPointer())) {
        return false;
    }

    if (storedTcp->getHeaderOptionArraySize() > 0 || liveTcp->getHeaderOptionArraySize()) {
     EV_DETAIL << "Options present";
        if (storedTcp->getHeaderOptionArraySize() == 0) {
            return true;
        }
        if (storedTcp->getHeaderOptionArraySize() != liveTcp->getHeaderOptionArraySize()) {
            return false;
        } else {
            TCPOption *storedOption, *liveOption;
            for (unsigned int i = 0; i < storedTcp->getHeaderOptionArraySize(); i++) {
                storedOption = storedTcp->getHeaderOption(i);
                liveOption = liveTcp->getHeaderOption(i);
                if (storedOption->getKind() == liveOption->getKind()) {
                    switch (storedOption->getKind()) {
                        case TCPOPT_EOL:
                        case TCPOPT_NOP:
                            if (!(storedOption->getLength() == liveOption->getLength())) {
                                return false;
                            }
                            break;
                        case TCPOPT_SACK_PERMITTED:
                            if (!(storedOption->getLength() == liveOption->getLength() &&
                                storedOption->getLength() == 2)) {
                                return false;
                            }
                            break;
                        case TCPOPT_WINDOW:
                            if (!(storedOption->getLength() == liveOption->getLength() &&
                                storedOption->getLength() == 3 &&
                                check_and_cast<TCPOptionWindowScale *>(storedOption)->getWindowScale()
                                 == check_and_cast<TCPOptionWindowScale *>(liveOption)->getWindowScale())) {
                                return false;
                            }
                            break;
                        case TCPOPT_SACK:
                            if (!(storedOption->getLength() == liveOption->getLength() &&
                                storedOption->getLength() > 2 && (storedOption->getLength() % 8) == 2 &&
                                check_and_cast<TCPOptionSack *>(storedOption)->getSackItemArraySize()
                                == check_and_cast<TCPOptionSack *>(liveOption)->getSackItemArraySize())) {
                                return false;
                            }
                            break;
                        case TCPOPT_TIMESTAMP:
                            if (!(storedOption->getLength() == liveOption->getLength() &&
                                storedOption->getLength() == 10 &&
                                check_and_cast<TCPOptionTimestamp *>(storedOption)->getSenderTimestamp()
                                == check_and_cast<TCPOptionTimestamp *>(liveOption)->getSenderTimestamp())) {
                                return false;
                            }
                            break;
                        default: EV_INFO << "Option not supported";
                    }
                } else {
                    EV_INFO << "Wrong sequence or option kind not present";
                    return false;
                }
            }
        }

    }
    return true;
}

bool PacketDrillApp::compareSctpPacket(SCTPMessage *storedSctp, SCTPMessage *liveSctp)
{
    if (!(storedSctp->getSrcPort() == liveSctp->getSrcPort())) {
        return false;
    }
    if (!(storedSctp->getDestPort() == liveSctp->getDestPort())) {
        return false;
    }
    if (!(storedSctp->getChunksArraySize() == liveSctp->getChunksArraySize())) {
        return false;
    }

    const uint32 numberOfChunks = storedSctp->getChunksArraySize();
    for (uint32 i = 0; i < numberOfChunks; i++) {
        SCTPChunk* storedHeader = (SCTPChunk*) (storedSctp->removeChunk());
        SCTPChunk* liveHeader = (SCTPChunk*) (liveSctp->removeChunk());
        if (!(storedHeader->getChunkType() == liveHeader->getChunkType()))
            return false;
        const uint8 type = storedHeader->getChunkType();

        if ((type != INIT && type != INIT_ACK) && (liveSctp->getTag() != localVTag)) {
            std::cout << " VTag " << liveSctp->getTag() << " incorrect. Should be " << localVTag << " peerVTag="
                    << peerVTag << endl;
            return false;
        }

        switch (type) {
            case DATA: {
                SCTPDataChunk* storedDataChunk = check_and_cast<SCTPDataChunk*>(storedHeader);
                SCTPDataChunk* liveDataChunk = check_and_cast<SCTPDataChunk*>(liveHeader);
                if (!(compareDataPacket(storedDataChunk, liveDataChunk))) {
                    delete storedDataChunk;
                    delete liveDataChunk;
                    return false;
                }
                break;
            }
            case INIT: {
                SCTPInitChunk* storedInitChunk = check_and_cast<SCTPInitChunk*>(storedHeader);
                SCTPInitChunk* liveInitChunk = check_and_cast<SCTPInitChunk*>(liveHeader);
                if (!(compareInitPacket(storedInitChunk, liveInitChunk))) {
                    delete storedInitChunk;
                    delete liveInitChunk;
                    return false;
                }
                break;
            }
            case INIT_ACK: {
                SCTPInitAckChunk* storedInitAckChunk = check_and_cast<SCTPInitAckChunk*>(storedHeader);
                SCTPInitAckChunk* liveInitAckChunk = check_and_cast<SCTPInitAckChunk*>(liveHeader);
                if (!(compareInitAckPacket(storedInitAckChunk, liveInitAckChunk))) {
                    delete storedInitAckChunk;
                    delete liveInitAckChunk;
                    return false;
                }
                break;
            }
            case SACK: {
                SCTPSackChunk* storedSackChunk = check_and_cast<SCTPSackChunk*>(storedHeader);
                SCTPSackChunk* liveSackChunk = check_and_cast<SCTPSackChunk*>(liveHeader);
                if (!(compareSackPacket(storedSackChunk, liveSackChunk))) {
                    delete storedSackChunk;
                    delete liveSackChunk;
                    return false;
                }
                break;
            }
            case COOKIE_ECHO: {
                SCTPCookieEchoChunk* storedCookieEchoChunk = check_and_cast<SCTPCookieEchoChunk*>(storedHeader);
                if (!(storedCookieEchoChunk->getFlags() & FLAG_CHUNK_VALUE_NOCHECK))
                    printf("COOKIE_ECHO chunks should be compared\n");
                else
                    printf("Do not check cookie echo chunks\n");
                break;
            }
            case SHUTDOWN: {
                SCTPShutdownChunk* storedShutdownChunk = check_and_cast<SCTPShutdownChunk*>(storedHeader);
                SCTPShutdownChunk* liveShutdownChunk = check_and_cast<SCTPShutdownChunk*>(liveHeader);
                if (!(storedShutdownChunk->getFlags() & FLAG_SHUTDOWN_CHUNK_CUM_TSN_NOCHECK))
                    if (!(peerCumTsn == liveShutdownChunk->getCumTsnAck())) {
                        delete storedShutdownChunk;
                        delete liveShutdownChunk;
                        return false;
                    }
                break;
            }
            case SHUTDOWN_COMPLETE: {
                SCTPShutdownCompleteChunk* storedShutdownCompleteChunk = check_and_cast<SCTPShutdownCompleteChunk*>(
                        storedHeader);
                SCTPShutdownCompleteChunk* liveShutdownCompleteChunk = check_and_cast<SCTPShutdownCompleteChunk*>(
                        liveHeader);
                if (!(storedShutdownCompleteChunk->getFlags() & FLAG_CHUNK_FLAGS_NOCHECK))
                    if (!(storedShutdownCompleteChunk->getTBit() == liveShutdownCompleteChunk->getTBit())) {
                        delete storedShutdownCompleteChunk;
                        delete liveShutdownCompleteChunk;
                        return false;
                    }
                break;
            }
            case ABORT: {
                SCTPAbortChunk* storedAbortChunk = check_and_cast<SCTPAbortChunk*>(storedHeader);
                SCTPAbortChunk* liveAbortChunk = check_and_cast<SCTPAbortChunk*>(liveHeader);
                if (!(storedAbortChunk->getFlags() & FLAG_CHUNK_FLAGS_NOCHECK))
                    if (!(storedAbortChunk->getT_Bit() == liveAbortChunk->getT_Bit())) {
                        delete storedAbortChunk;
                        delete liveAbortChunk;
                        return false;
                    }
                break;
            }
            case HEARTBEAT: {
                SCTPHeartbeatChunk* heartbeatChunk = check_and_cast<SCTPHeartbeatChunk*>(liveHeader);
                peerHeartbeatTime = heartbeatChunk->getTimeField();
                break;
            }
            case COOKIE_ACK:
            case SHUTDOWN_ACK:
            case HEARTBEAT_ACK:
                break;
            default:
                printf("type %d not implemented\n", type);
        }
        delete storedHeader;
        delete liveHeader;
    }
    return true;
}

bool PacketDrillApp::compareDataPacket(SCTPDataChunk* storedDataChunk, SCTPDataChunk* liveDataChunk)
{
    uint32 flags = storedDataChunk->getFlags();
    if (!(flags & FLAG_CHUNK_LENGTH_NOCHECK))
        if (storedDataChunk->getByteLength() != liveDataChunk->getByteLength())
            return false;

    if (!(flags & FLAG_CHUNK_FLAGS_NOCHECK)) {
        if (!(storedDataChunk->getBBit() == liveDataChunk->getBBit()))
            return false;
        if (!(storedDataChunk->getEBit() == liveDataChunk->getEBit()))
            return false;
    }
    if (!(flags & FLAG_DATA_CHUNK_TSN_NOCHECK))
        if (!(storedDataChunk->getTsn() + initLocalTsn == liveDataChunk->getTsn()))
            return false;
    if (!(flags & FLAG_DATA_CHUNK_SID_NOCHECK))
        if (!(storedDataChunk->getSid() == liveDataChunk->getSid()))
            return false;
    if (!(flags & FLAG_DATA_CHUNK_SSN_NOCHECK))
        if (!(storedDataChunk->getSsn() == liveDataChunk->getSsn()))
            return false;
    if (!(flags & FLAG_DATA_CHUNK_PPID_NOCHECK))
        if ( !(storedDataChunk->getPpid() == liveDataChunk->getPpid()))
            return false;

    return true;
}

bool PacketDrillApp::compareInitPacket(SCTPInitChunk* storedInitChunk, SCTPInitChunk* liveInitChunk)
{
    uint32 flags = storedInitChunk->getFlags();
    peerVTag = liveInitChunk->getInitTag();
    initLocalTsn = liveInitChunk->getInitTSN();
    localCumTsn = initLocalTsn - 1;

    if (!(flags & FLAG_INIT_CHUNK_TSN_NOCHECK))
        initPeerTsn -= storedInitChunk->getInitTSN();
    if (!(flags & FLAG_INIT_CHUNK_TAG_NOCHECK))
        if (!(storedInitChunk->getInitTag() == liveInitChunk->getInitTag())) {
            return false;
        }

    if (!(flags & FLAG_INIT_CHUNK_A_RWND_NOCHECK))
        if (!(storedInitChunk->getA_rwnd() == liveInitChunk->getA_rwnd()))
            return false;
    peerInStreams = liveInitChunk->getNoInStreams();
    peerOutStreams = liveInitChunk->getNoOutStreams();
    if (!(flags & FLAG_INIT_CHUNK_OS_NOCHECK))
        if (!(storedInitChunk->getNoOutStreams() == liveInitChunk->getNoOutStreams()))
            return false;
    if (!(flags & FLAG_INIT_CHUNK_IS_NOCHECK))
        if (!(storedInitChunk->getNoInStreams() == liveInitChunk->getNoInStreams()))
            return false;

    return true;
}

bool PacketDrillApp::compareInitAckPacket(SCTPInitAckChunk* storedInitAckChunk, SCTPInitAckChunk* liveInitAckChunk)
{
    uint32 flags = storedInitAckChunk->getFlags();
    peerVTag = liveInitAckChunk->getInitTag();
    initPeerTsn = liveInitAckChunk->getInitTSN();
    localCumTsn = initPeerTsn - 1;
    peerCumTsn = initLocalTsn - 1;

    if (!(flags & FLAG_INIT_ACK_CHUNK_A_RWND_NOCHECK)) {
        if (!(storedInitAckChunk->getA_rwnd() == liveInitAckChunk->getA_rwnd()))
            return false;
    }

    if (!(flags & FLAG_INIT_ACK_CHUNK_OS_NOCHECK))
        if (!(min(storedInitAckChunk->getNoOutStreams(), peerInStreams) == liveInitAckChunk->getNoOutStreams()))
            return false;
    if (!(flags & FLAG_INIT_ACK_CHUNK_IS_NOCHECK))
        if (!(min(storedInitAckChunk->getNoInStreams(), peerOutStreams) == liveInitAckChunk->getNoInStreams()))
            return false;

    peerCookie = check_and_cast<SCTPCookie*>(liveInitAckChunk->getStateCookie());
    peerCookieLength = peerCookie->getByteLength();
    return true;
}

bool PacketDrillApp::compareSackPacket(SCTPSackChunk* storedSackChunk, SCTPSackChunk* liveSackChunk)
{
    uint32 flags = storedSackChunk->getFlags();
    if (!(flags & FLAG_SACK_CHUNK_CUM_TSN_NOCHECK))
        if (!(storedSackChunk->getCumTsnAck() == liveSackChunk->getCumTsnAck()))
            return false;

    peerCumTsn = liveSackChunk->getCumTsnAck();
    if (!(flags & FLAG_SACK_CHUNK_A_RWND_NOCHECK))
        if (!(storedSackChunk->getA_rwnd() == liveSackChunk->getA_rwnd()))
            return false;

    if (!(flags & FLAG_SACK_CHUNK_GAP_BLOCKS_NOCHECK))
        if (!(storedSackChunk->getNumGaps() == liveSackChunk->getNumGaps()))
            return false;

    if (storedSackChunk->getNumGaps() > 0)
    {
        for (int i = 0; i < storedSackChunk->getNumGaps(); i++)
        {
            if (!(storedSackChunk->getGapStart(i) == liveSackChunk->getGapStart(i))
                    || !(storedSackChunk->getGapStop(i) == liveSackChunk->getGapStop(i)))
            {
                return false;
            }
        }
    }

    if (!(flags & FLAG_SACK_CHUNK_DUP_TSNS_NOCHECK))
        if (!(storedSackChunk->getNumDupTsns() == liveSackChunk->getNumDupTsns()))
            return false;

    if (storedSackChunk->getNumDupTsns() > 0)
    {
        for (int i = 0; i < storedSackChunk->getNumDupTsns(); i++)
        {
            if (!(storedSackChunk->getDupTsns(i) == liveSackChunk->getDupTsns(i)))
            {
                return false;
            }
        }
    }

    return true;
}

} // namespace INET
