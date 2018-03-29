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

#include "PacketDrillApp.h"

#include <stdlib.h>
#include <stdio.h>

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "PacketDrillUtils.h"
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
    script = nullptr;
    config = nullptr;
    localPort = 1000;
    remotePort = 2000;
    protocol = 0;
    tcpConnId = -1;
    sctpAssocId = -1;
    pd = nullptr;
    msgArrived = false;
    recvFromSet = false;
    listenSet = false;
    acceptSet = false;
    establishedPending = false;
    abortSent = false;
    socketOptionsArrived = false;
    receivedPackets = nullptr;
    outboundPackets = nullptr;
    expectedMessageSize = 0;
    relSequenceIn = 0;
    relSequenceOut = 0;
    peerTS = 0;
    peerWindow = 0;
    peerInStreams = 0;
    peerOutStreams = 0;
    peerCookie = nullptr;
    peerCookieLength = 0;
    initPeerTsn = 0;
    initLocalTsn = 0;
    localDiffTsn = 0;
    peerCumTsn = 0;
    localCumTsn = 0;
    eventCounter = 0;
    numEvents = 0;
    idInbound = 0;
    idOutbound = 0;
    localVTag = 0;
    peerVTag = 0;
    eventTimer = nullptr;

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
                            tcpConnId = check_and_cast<TCPCommand *>(msg->getControlInfo())->getConnId();
                            listenSet = false;
                            acceptSet = false;
                        } else {
                            tcpConnId = check_and_cast<TCPCommand *>(msg->getControlInfo())->getConnId();
                            establishedPending = true;
                        }
                    } else {
                        tcpSocket.setState(TCPSocket::CONNECTED);
                        tcpConnId = check_and_cast<TCPCommand *>(msg->getControlInfo())->getConnId();
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
                        cmd->setConnId(tcpConnId);
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
                            sctpAssocId = check_and_cast<SCTPCommand *>(msg->getControlInfo())->getAssocId();
                            listenSet = false;
                            acceptSet = false;
                        } else {
                            sctpAssocId = check_and_cast<SCTPCommand *>(msg->getControlInfo())->getAssocId();
                            establishedPending = true;
                        }
                    } else {
                        sctpSocket.setState(SCTPSocket::CONNECTED);
                        SCTPConnectInfo *connectInfo = check_and_cast<SCTPConnectInfo *>(msg->removeControlInfo());
                        sctpAssocId = connectInfo->getAssocId();
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
                        cmd->setAssocId(sctpAssocId);
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
                case SCTP_I_CLOSED:
                case SCTP_I_ABORT:
                case SCTP_I_SEND_STREAMS_RESETTED:
                case SCTP_I_RCV_STREAMS_RESETTED:
                case SCTP_I_PEER_CLOSED: {
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
    cancelEvent(eventTimer);
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
                int32 noChunks = sctp->getChunksArraySize();
                for (int32 cc = 0; cc < noChunks; cc++) {
                    SCTPChunk *chunk = const_cast<SCTPChunk *>(check_and_cast<const SCTPChunk *>(((SCTPMessage *)sctp)->getChunks(cc)));
                    unsigned char chunkType = chunk->getChunkType();
                    switch (chunkType) {
                        case INIT: {
                            SCTPInitChunk* init = check_and_cast<SCTPInitChunk*>(chunk);
                            peerInStreams = init->getNoInStreams();
                            peerOutStreams = init->getNoOutStreams();
                            initPeerTsn = init->getInitTSN();
                            localVTag = init->getInitTag();
                            peerCumTsn = initPeerTsn - 1;
                            break;
                        }
                        case INIT_ACK: {
                            SCTPInitAckChunk* initack = check_and_cast<SCTPInitAckChunk*>(chunk);
                            localVTag = initack->getInitTag();
                            initPeerTsn = initack->getInitTSN();
                            peerCumTsn = initPeerTsn - 1;
                            break;
                        }
                        case COOKIE_ECHO: {
                            SCTPCookieEchoChunk* cookieEcho = check_and_cast<SCTPCookieEchoChunk*>(chunk);
                            int tempLength = cookieEcho->getByteLength();
                            printf("copy peerCookie %p\n", peerCookie);
                            cookieEcho->setStateCookie(peerCookie->dup());
                            cookieEcho->getStateCookie()->setName("CookieEchoStateCookie");
                            cookieEcho->setByteLength(SCTP_COOKIE_ACK_LENGTH + peerCookieLength);
                            sctp->setByteLength(sctp->getByteLength() - tempLength + cookieEcho->getByteLength());
                            delete peerCookie;
                            peerCookie = nullptr;
                            break;
                        }
                        case SACK: {
                            SCTPSackChunk* sack = check_and_cast<SCTPSackChunk*>(chunk);
                            sack->setCumTsnAck(sack->getCumTsnAck() + localDiffTsn);
                            if (sack->getNumGaps() > 0) {
                                for (int i = 0; i < sack->getNumGaps(); i++) {
                                    sack->setGapStart(i, sack->getGapStart(i) + sack->getCumTsnAck());
                                    sack->setGapStop(i, sack->getGapStop(i) + sack->getCumTsnAck());
                                }
                            }
                            if (sack->getNumDupTsns() > 0) {
                                for (int i = 0; i < sack->getNumDupTsns(); i++) {
                                    sack->setDupTsns(i, sack->getDupTsns(i) + localDiffTsn);
                                }
                            }
                            sctp->replaceChunk(sack, cc);
                            break;
                        }
                        case RE_CONFIG:{
                            SCTPStreamResetChunk* reconfig = check_and_cast<SCTPStreamResetChunk*>(chunk);
                            for (unsigned int i = 0; i < reconfig->getParametersArraySize(); i++) {
                                SCTPParameter *parameter = check_and_cast<SCTPParameter *>(reconfig->getParameters(i));
                                switch (parameter->getParameterType()) {
                                    case STREAM_RESET_RESPONSE_PARAMETER: {
                                        SCTPStreamResetResponseParameter *param = check_and_cast<SCTPStreamResetResponseParameter *>(parameter);
                                        param->setSrResSn(seqNumMap[param->getSrResSn()]);
                                        if (param->getReceiversNextTsn() != 0) {
                                            param->setReceiversNextTsn(param->getReceiversNextTsn() + localDiffTsn);
                                        }
                                        break;
                                    }
                                    case OUTGOING_RESET_REQUEST_PARAMETER: {
                                        SCTPOutgoingSSNResetRequestParameter *param = check_and_cast<SCTPOutgoingSSNResetRequestParameter *>(parameter);
                                      /*  for (const auto & elem : seqNumMap) {
                                            std::cout << " myNum = " << elem.first << "  liveNum = " << elem.second << endl;
                                        }*/
                                        if (findSeqNumMap(param->getSrResSn())) {
                                            param->setSrResSn(seqNumMap[param->getSrResSn()]);
                                        }
                                        break;
                                    }
                                }
                            }
                            sctp->replaceChunk(reconfig, cc);
                            break;
                        }
                    }
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
                    if (!compareDatagram(ip, live)) {
                        delete liveInfo;
                        throw cTerminationException("Packetdrill error: Datagrams are not the same");
                    }
                    delete liveInfo;
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
    } else if (event->getType() == COMMAND_EVENT) {
        eventCounter++;
        scheduleEvent();
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
            if (eventCounter >= numEvents - 1 && outboundPackets->getLength() == 0) {
                closeAllSockets();
            }
            break;
        }

        default:
            throw cRuntimeError("Unknown message kind");
    }
}

void PacketDrillApp::closeAllSockets()
{
    SCTPAbortChunk *abortChunk = new SCTPAbortChunk("Abort");
    abortChunk->setChunkType(ABORT);
    abortChunk->setT_Bit(1);
    abortChunk->setByteLength(SCTP_ABORT_CHUNK_LENGTH);
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setByteLength(SCTP_COMMON_HEADER);
    sctpmsg->setSrcPort(remotePort);
    sctpmsg->setDestPort(localPort);
    sctpmsg->setTag(peerVTag);
    sctpmsg->setName("SCTPCleanUp");
    sctpmsg->setChecksumOk(true);
    sctpmsg->addChunk(abortChunk);
    IPv4Datagram *datagram = new IPv4Datagram("IPCleanup");
    datagram->setSrcAddress(remoteAddress.toIPv4());
    datagram->setDestAddress(localAddress.toIPv4());
    datagram->setIdentification(0);
    datagram->setVersion(4);
    datagram->setHeaderLength(20);
    datagram->setTransportProtocol(IPPROTO_SCTP);
    datagram->setTimeToLive(32);
    datagram->setMoreFragments(0);
    datagram->setDontFragment(0);
    datagram->setFragmentOffset(0);
    datagram->setTypeOfService(0);
    datagram->setByteLength(20);
    datagram->encapsulate(sctpmsg);
    EV_DETAIL << "Send Abort to cleanup association." << endl;
    send(datagram, "tunOut");
}

bool PacketDrillApp::findSeqNumMap(uint32 num)
{
    std::map<uint32, uint32>::iterator it;

    it = seqNumMap.find(num);
    if (it != seqNumMap.end())
        return true;
    return false;
}

void PacketDrillApp::runSystemCallEvent(PacketDrillEvent* event, struct syscall_spec *syscall)
{
    char *error = NULL;
    const char *name = syscall->name;
    cQueue *args = new cQueue("systemCallEventQueue");
    int result = 0;

    // Evaluate script symbolic expressions to get live numeric args for system calls.

    if (pd->evaluateExpressionList(syscall->arguments, args, &error)) {
        args->clear();
        delete(args);
        delete(syscall->arguments);
        free (syscall);
        free(error);
        return;
    }

    if (!strcmp(name, "socket")) {
        syscallSocket(syscall, args, &error);
    } else if (!strcmp(name, "bind")) {
        syscallBind(syscall, args, &error);
    } else if (!strcmp(name, "listen")) {
        syscallListen(syscall, args, &error);
    } else if (!strcmp(name, "write") || !strcmp(name, "send")) {
        syscallWrite(syscall, args, &error);
    } else if (!strcmp(name, "read")) {
        syscallRead((PacketDrillEvent*) event, syscall, args, &error);
    } else if (!strcmp(name, "sendto")) {
        syscallSendTo(syscall, args, &error);
    } else if (!strcmp(name, "recvfrom")) {
        syscallRecvFrom((PacketDrillEvent*)event, syscall, args, &error);
    } else if (!strcmp(name, "close")) {
        syscallClose(syscall, args, &error);
    } else if (!strcmp(name, "shutdown")) {
        syscallShutdown(syscall, args, &error);
    } else if (!strcmp(name, "connect")) {
        syscallConnect(syscall, args, &error);
    } else if (!strcmp(name, "accept")) {
        syscallAccept(syscall, args, &error);
    } else if (!strcmp(name, "setsockopt")) {
        syscallSetsockopt(syscall, args, &error);
    } else if (!strcmp(name, "getsockopt")) {
        syscallGetsockopt(syscall, args, &error);
    } else if (!strcmp(name, "sctp_sendmsg")) {
        syscallSctpSendmsg(syscall, args, &error);
    } else if (!strcmp(name, "sctp_send")) {
        syscallSctpSend(syscall, args, &error);
    } else {
        EV_INFO << "System call %s not known (yet)." << name;
    }
    args->clear();
    delete(args);
    delete(syscall->arguments);
    free (syscall);
    if (result == STATUS_ERR) {
        EV_ERROR << event->getLineNumber() << ": runtime error in " << syscall->name << " call: " << error << endl;
        closeAllSockets();
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
            sctpSocket.listen(0, true, 0, true, script_fd);
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
    int script_accepted_fd;
    if (!listenSet)
        return STATUS_ERR;

    PacketDrillExpression* exp = (PacketDrillExpression *)syscall->result;
    if (!exp || exp->getS32(&script_accepted_fd, error))
        return STATUS_ERR;
    if (establishedPending) {
        if (protocol == IP_PROT_TCP)
            tcpSocket.setState(TCPSocket::CONNECTED);
        else if (protocol == IP_PROT_SCTP)
            sctpSocket.setState(SCTPSocket::CONNECTED);
        establishedPending = false;
        sctpSocket.accept(sctpAssocId, script_accepted_fd);
    } else {
        acceptSet = true;
    }

    return STATUS_OK;
}

int PacketDrillApp::syscallWrite(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, count;
    PacketDrillExpression* exp;

    if (args->getLength() > 4)
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
            sendCommand->setAssocId(-1);
            sendCommand->setSendUnordered(false);
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
            sctpSocket.connect(script_fd, remoteAddress, remotePort, 0, true);
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

    if (syscall->result->getNum() == -1) {
        if (exp->getType() == EXPR_SCTP_RESET_STREAMS) {
            delete exp->getResetStreams()->srs_stream_list;
        }
        return STATUS_OK;
    }
    switch (exp->getType()) {
        case EXPR_SCTP_INITMSG: {
            struct sctp_initmsg_expr *initmsg = exp->getInitmsg();
            sctpSocket.setOutboundStreams(initmsg->sinit_num_ostreams->getNum());
            sctpSocket.setInboundStreams(initmsg->sinit_max_instreams->getNum());
            sctpSocket.setMaxInitRetrans(initmsg->sinit_max_attempts->getNum());
            sctpSocket.setMaxInitRetransTimeout(initmsg->sinit_max_init_timeo->getNum());
            if (sctpSocket.getInboundStreams() > 0)
                sctpSocket.setAppLimited(true);
            break;
        }
        case EXPR_SCTP_RTOINFO: {
            struct sctp_rtoinfo_expr *rtoinfo = exp->getRtoinfo();
            sctpSocket.setRtoInfo(rtoinfo->srto_initial->getNum() * 1.0 / 1000, rtoinfo->srto_max->getNum() * 1.0 / 1000, rtoinfo->srto_min->getNum() * 1.0 / 1000);
            free (rtoinfo);
            break;
        }
        case EXPR_SCTP_SACKINFO: {
            struct sctp_sack_info_expr *sackinfo = exp->getSackinfo();
            sctpSocket.setSackPeriod(sackinfo->sack_delay->getNum() * 1.0 / 1000);
            sctpSocket.setSackFrequency(sackinfo->sack_freq->getNum());
            break;
        }
        case EXPR_SCTP_PEER_ADDR_PARAMS: {
            struct sctp_paddrparams_expr *expr_params = exp->getPaddrParams();
            if (expr_params->spp_flags->getNum() & SPP_HB_DISABLE)
                sctpSocket.setEnableHeartbeats(false);
            else if (expr_params->spp_flags->getNum() & SPP_HB_ENABLE)
                sctpSocket.setEnableHeartbeats(true);
            if (expr_params->spp_hbinterval->getNum() > 0)
                sctpSocket.setHbInterval(expr_params->spp_hbinterval->getNum());
            if (expr_params->spp_pathmaxrxt->getNum() > 0)
                sctpSocket.setPathMaxRetrans(expr_params->spp_pathmaxrxt->getNum());
            break;
        }
        case EXPR_SCTP_ASSOCPARAMS: {
            struct sctp_assocparams_expr *assoc_params = exp->getAssocParams();
            sctpSocket.setAssocMaxRtx(assoc_params->sasoc_asocmaxrxt->getNum());
            break;
        }
        case EXPR_SCTP_RESET_STREAMS: {
            struct sctp_reset_streams_expr *rs = exp->getResetStreams();
            cMessage *cmsg = new cMessage("SCTP_C_STREAM_RESET");
            SCTPResetInfo *rinfo = new SCTPResetInfo();
            rinfo->setAssocId(-1);
            rinfo->setFd(rs->srs_assoc_id->getNum());
            rinfo->setRemoteAddr(sctpSocket.getRemoteAddr());
            if (rs->srs_number_streams->getNum() > 0 && rs->srs_stream_list != nullptr) {
                rinfo->setStreamsArraySize(rs->srs_number_streams->getNum());
                cQueue *qu = rs->srs_stream_list->getList();
                uint16 i = 0;
                for (cQueue::Iterator iter(*qu); !iter.end(); iter++, i++) {
                    rinfo->setStreams(i, ((PacketDrillExpression *)(*iter))->getNum());
                    qu->remove((*iter));
                }
                qu->clear();
            }
            cmsg->setKind(SCTP_C_STREAM_RESET);
            if (rs->srs_flags->getNum() == SCTP_STREAM_RESET_OUTGOING) {
                rinfo->setRequestType(RESET_OUTGOING);
            } else if (rs->srs_flags->getNum() == SCTP_STREAM_RESET_INCOMING) {
                rinfo->setRequestType(RESET_INCOMING);
            } else if (rs->srs_flags->getNum() == (SCTP_STREAM_RESET_OUTGOING | SCTP_STREAM_RESET_INCOMING)) {
                rinfo->setRequestType(RESET_BOTH);
            }
            cmsg->setControlInfo(rinfo);
            sctpSocket.sendNotification(cmsg);
            delete (rs->srs_assoc_id);
            delete (rs->srs_flags);
            delete (rs->srs_number_streams);
            delete (rs->srs_stream_list);
            free (rs);
            break;
        }
        case EXPR_SCTP_ADD_STREAMS: {
            struct sctp_add_streams_expr *as = exp->getAddStreams();
            cMessage *cmsg = new cMessage("SCTP_C_STREAM_RESET");
            SCTPResetInfo *rinfo = new SCTPResetInfo();
            rinfo->setAssocId(-1);
            rinfo->setFd(as->sas_assoc_id->getNum());
            rinfo->setRemoteAddr(sctpSocket.getRemoteAddr());
            if (as->sas_instrms->getNum() != 0 && as->sas_outstrms->getNum() != 0) {
                rinfo->setRequestType(ADD_BOTH);
                rinfo->setInstreams(as->sas_instrms->getNum());
                rinfo->setOutstreams(as->sas_outstrms->getNum());
            } else if (as->sas_instrms->getNum() != 0) {
                rinfo->setRequestType(ADD_INCOMING);
                rinfo->setInstreams(as->sas_instrms->getNum());
            } else if (as->sas_outstrms->getNum() != 0) {
                rinfo->setRequestType(ADD_OUTGOING);
                rinfo->setOutstreams(as->sas_outstrms->getNum());
            }
            cmsg->setKind(SCTP_C_ADD_STREAMS);
            cmsg->setControlInfo(rinfo);
            sctpSocket.sendNotification(cmsg);
            delete (as->sas_assoc_id);
            delete (as->sas_instrms);
            delete (as->sas_outstrms);
            free (as);
            break;
        }

        case EXPR_SCTP_ASSOCVAL:
            switch (optname) {
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
                case SCTP_ENABLE_STREAM_RESET: {
                    struct sctp_assoc_value_expr *assocvalue = exp->getAssocval();
                    sctpSocket.setStreamReset(assocvalue->assoc_value->getNum());
                    delete (assocvalue->assoc_id);
                    delete (assocvalue->assoc_value);
                    free (assocvalue);
                    break;
                }
                default:
                    printf("Option name %d of type EXPR_SCTP_ASSOCVAL not known\n", optname);
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

            PacketDrillExpression *exp2 = (PacketDrillExpression*) (exp->getList()->pop());
            exp2->getS32(&value, error);
            switch (optname)
            {
                case SCTP_NODELAY:
                    sctpSocket.setNagle(value? 0 : 1);
                    break;
                case SCTP_RESET_ASSOC: {
                    cMessage *cmsg = new cMessage("SCTP_C_STREAM_RESET");
                    SCTPResetInfo *rinfo = new SCTPResetInfo();
                    rinfo->setAssocId(-1);
                    rinfo->setFd(value);
                    rinfo->setRemoteAddr(sctpSocket.getRemoteAddr());
                    rinfo->setRequestType(SSN_TSN);
                    cmsg->setKind(SCTP_C_RESET_ASSOC);
                    cmsg->setControlInfo(rinfo);
                    sctpSocket.sendNotification(cmsg);
                    break;
                }
            }
            break;
        }
        case EXPR_INTEGER:
            break;
        default:
            printf("Type %d not known\n", exp->getType());
    }
    return STATUS_OK;
}

int PacketDrillApp::syscallGetsockopt(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, level, optname;
    PacketDrillExpression *exp;

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
    switch (exp->getType()) {
        case EXPR_SCTP_STATUS: {
            struct sctp_status_expr *status = exp->getStatus();
            if (status->sstat_instrms->getType() != EXPR_ELLIPSIS)
                if (status->sstat_instrms->getNum() != sctpSocket.getInboundStreams()) {
                    printf("Number of Inbound Streams does not match\n");
                    return STATUS_ERR;
                }
            if (status->sstat_outstrms->getType() != EXPR_ELLIPSIS)
                if (status->sstat_outstrms->getNum() != sctpSocket.getOutboundStreams()) {
                    printf("Number of Outbound Streams does not match\n");
                    return STATUS_ERR;
                }
            break;
        }
        default: printf("Getsockopt option is not supported\n");
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

int PacketDrillApp::syscallSctpSendmsg(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, count;
    PacketDrillExpression *exp;
    uint32 flags, ppid, ttl, context;
    uint16 stream_no;

    if (args->getLength() != 10)
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
    /*ToDo: handle address parameter */
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(4);
    /*ToDo: handle tolen parameter */
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(5);
    if (!exp || exp->getU32(&ppid, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(6);
    if (!exp || exp->getU32(&flags, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(7);
    if (!exp || exp->getU16(&stream_no, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(8);
    if (!exp || exp->getU32(&ttl, error))
        return STATUS_ERR;
    exp = (PacketDrillExpression *)args->get(9);
    if (!exp || exp->getU32(&context, error))
        return STATUS_ERR;

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

    SCTPSendInfo* sendCommand = new SCTPSendInfo;
    sendCommand->setLast(true);
    sendCommand->setAssocId(sctpAssocId);
    sendCommand->setSid(stream_no);
    sendCommand->setPpid(ppid);
    if (flags == SCTP_UNORDERED) {
        sendCommand->setSendUnordered(true);
    }
    cmsg->setControlInfo(sendCommand);

    sctpSocket.sendMsg(cmsg);
    return STATUS_OK;
}

int PacketDrillApp::syscallSctpSend(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, count;
    PacketDrillExpression *exp;
    uint16 sid = 0, ssn = 0;
    uint32 ppid = 0;

    if (syscall->result->getNum() == -1) {
        return STATUS_OK;
    }
    if (args->getLength() != 5)
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
    if (exp->getType() == EXPR_SCTP_SNDRCVINFO) {
        struct sctp_sndrcvinfo_expr *info = exp->getSndRcvInfo();
        ssn = info->sinfo_ssn->getNum();
        sid = info->sinfo_stream->getNum();
        ppid = info->sinfo_ppid->getNum();
    }
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

    SCTPSendInfo* sendCommand = new SCTPSendInfo;
    sendCommand->setLast(true);
    sendCommand->setAssocId(-1);
    sendCommand->setSid(sid);
    sendCommand->setPpid(ppid);
    sendCommand->setSsn(ssn);
    sendCommand->setSendUnordered(false);
    cmsg->setControlInfo(sendCommand);

    sctpSocket.sendMsg(cmsg);
    return STATUS_OK;
}

int PacketDrillApp::syscallRead(PacketDrillEvent *event, struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, count;
    PacketDrillExpression* exp;

    if (syscall->result->getNum() == -1) {
        return STATUS_OK;
    }
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
        if (msgArrived || receivedPackets->getLength() > 0) {
            switch (protocol) {
                case IP_PROT_TCP: {
                    cMessage *msg = new cMessage("dataRequest");
                    msg->setKind(TCP_C_READ);
                    TCPCommand *tcpcmd = new TCPCommand();
                    tcpcmd->setConnId(tcpConnId);
                    msg->setControlInfo(tcpcmd);
                    send(msg, "tcpOut");
                    break;
                }
                case IP_PROT_SCTP: {
                    cPacket* pkt = new cPacket("dataRequest", SCTP_C_RECEIVE);
                    SCTPSendInfo *sctpcmd = new SCTPSendInfo();
                    sctpcmd->setAssocId(sctpAssocId);
                    sctpcmd->setSid(0);
                    pkt->setControlInfo(sctpcmd);
                    send(pkt, "sctpOut");
                    break;
                }
                default: EV_INFO << "Protocol not supported for this system call.";
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
            cmd->setConnId(tcpConnId);
            msg->setControlInfo(cmd);
            send(msg, "tcpOut");
            break;
        }
        case IP_PROT_SCTP: {
            sctpSocket.close(script_fd);
            break;
        }
        default:
            EV_INFO << "Protocol " << protocol << " is not supported for this system call\n";
    }
    return STATUS_OK;
}

int PacketDrillApp::syscallShutdown(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd;
printf("syscallShutdown\n");
    if (args->getLength() != 2)
        return STATUS_ERR;
    PacketDrillExpression *exp = (PacketDrillExpression *)args->get(0);
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;

    switch (protocol) {
        case IP_PROT_SCTP: {
            sctpSocket.shutdown(script_fd);
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
    if (eventTimer->isScheduled()) {
        cancelEvent(eventTimer);
    }
    delete eventTimer;
    delete pd;
    delete receivedPackets;
    delete outboundPackets;
    delete config;
    delete script;
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
                EV_DETAIL << "SCTP packets are not the same" << endl;
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
            TCPOption *liveOption;
            for (unsigned int i = 0; i < liveTcp->getHeaderOptionArraySize(); i++) {
                liveOption = liveTcp->getHeaderOption(i);
            }
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
        if (!(storedHeader->getChunkType() == liveHeader->getChunkType())) {
            return false;
        }
        const uint8 type = storedHeader->getChunkType();

        if ((type != INIT && type != INIT_ACK) && type != ABORT && (liveSctp->getTag() != localVTag)) {
            EV_DETAIL << " VTag " << liveSctp->getTag() << " incorrect. Should be " << localVTag << " peerVTag="
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
                    EV_DETAIL << "DATA chunks are not the same" << endl;
                    return false;
                }
                delete storedDataChunk;
                delete liveDataChunk;
                break;
            }
            case INIT: {
                SCTPInitChunk* storedInitChunk = check_and_cast<SCTPInitChunk*>(storedHeader);
                SCTPInitChunk* liveInitChunk = check_and_cast<SCTPInitChunk*>(liveHeader);
                if (!(compareInitPacket(storedInitChunk, liveInitChunk))) {
                    delete storedInitChunk;
                    delete liveInitChunk;
                    EV_DETAIL << "INIT chunks are not the same" << endl;
                    return false;
                }
                delete storedInitChunk;
                delete liveInitChunk;
                break;
            }
            case INIT_ACK: {
                SCTPInitAckChunk* storedInitAckChunk = check_and_cast<SCTPInitAckChunk*>(storedHeader);
                SCTPInitAckChunk* liveInitAckChunk = check_and_cast<SCTPInitAckChunk*>(liveHeader);
                if (!(compareInitAckPacket(storedInitAckChunk, liveInitAckChunk))) {
                    delete storedInitAckChunk;
                    delete liveInitAckChunk;
                    EV_DETAIL << "INIT-ACK chunks are not the same" << endl;
                    return false;
                }
                delete storedInitAckChunk;
                delete liveInitAckChunk;
                break;
            }
            case SACK: {
                SCTPSackChunk* storedSackChunk = check_and_cast<SCTPSackChunk*>(storedHeader);
                SCTPSackChunk* liveSackChunk = check_and_cast<SCTPSackChunk*>(liveHeader);
                if (!(compareSackPacket(storedSackChunk, liveSackChunk))) {
                    delete storedSackChunk;
                    delete liveSackChunk;
                    EV_DETAIL << "SACK chunks are not the same" << endl;
                    return false;
                }
                delete storedSackChunk;
                delete liveSackChunk;
                break;
            }
            case COOKIE_ECHO: {
                SCTPCookieEchoChunk* storedCookieEchoChunk = check_and_cast<SCTPCookieEchoChunk*>(storedHeader);
                if (!(storedCookieEchoChunk->getFlags() & FLAG_CHUNK_VALUE_NOCHECK))
                    printf("COOKIE_ECHO chunks should be compared\n");
                else
                    printf("Do not check cookie echo chunks\n");
                delete storedCookieEchoChunk;
                if (numberOfChunks == 1)
                    delete liveHeader;
                break;
            }
            case SHUTDOWN: {
                SCTPShutdownChunk* storedShutdownChunk = check_and_cast<SCTPShutdownChunk*>(storedHeader);
                SCTPShutdownChunk* liveShutdownChunk = check_and_cast<SCTPShutdownChunk*>(liveHeader);
                if (!(storedShutdownChunk->getFlags() & FLAG_SHUTDOWN_CHUNK_CUM_TSN_NOCHECK)) {
                    if (!(storedShutdownChunk->getCumTsnAck() == liveShutdownChunk->getCumTsnAck())) {
                        delete storedShutdownChunk;
                        delete liveShutdownChunk;
                        EV_DETAIL << "SHUTDOWN chunks are not the same" << endl;
                        return false;
                    }
                }
                delete storedShutdownChunk;
                delete liveShutdownChunk;
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
                        EV_DETAIL << "SHUTDOWN-COMPLETE chunks are not the same" << endl;
                        return false;
                    }
                delete storedShutdownCompleteChunk;
                delete liveShutdownCompleteChunk;
                break;
            }
            case ABORT: {
                SCTPAbortChunk* storedAbortChunk = check_and_cast<SCTPAbortChunk*>(storedHeader);
                SCTPAbortChunk* liveAbortChunk = check_and_cast<SCTPAbortChunk*>(liveHeader);
                if (!(storedAbortChunk->getFlags() & FLAG_CHUNK_FLAGS_NOCHECK))
                    if (!(storedAbortChunk->getT_Bit() == liveAbortChunk->getT_Bit())) {
                        delete storedAbortChunk;
                        delete liveAbortChunk;
                        EV_DETAIL << "ABORT chunks are not the same" << endl;
                        return false;
                    }
                delete storedAbortChunk;
                delete liveAbortChunk;
                break;
            }
            case ERRORTYPE: {
                SCTPErrorChunk* storedErrorChunk = check_and_cast<SCTPErrorChunk*>(storedHeader);
                SCTPErrorChunk* liveErrorChunk = check_and_cast<SCTPErrorChunk*>(liveHeader);
                if (!(storedErrorChunk->getParametersArraySize() == liveErrorChunk->getParametersArraySize())) {
                    delete storedErrorChunk;
                    delete liveErrorChunk;
                    return false;
                }
                if (storedErrorChunk->getParametersArraySize() > 0) {
                // Only Cause implemented so far.
                    SCTPSimpleErrorCauseParameter *storedcause = check_and_cast<SCTPSimpleErrorCauseParameter *>(storedErrorChunk->getParameters(0));
                    SCTPSimpleErrorCauseParameter *livecause = check_and_cast<SCTPSimpleErrorCauseParameter *>(liveErrorChunk->getParameters(0));
                    if (!(storedcause->getValue() == livecause->getValue())) {
                        delete storedcause;
                        delete livecause;
                        delete storedErrorChunk;
                        delete liveErrorChunk;
                        return false;
                    }
                }
                delete storedErrorChunk;
                delete liveErrorChunk;
                break;
            }
            case HEARTBEAT: {
                SCTPHeartbeatChunk* heartbeatChunk = check_and_cast<SCTPHeartbeatChunk*>(liveHeader);
                peerHeartbeatTime = heartbeatChunk->getTimeField();
                delete heartbeatChunk;
                break;
            }
            case COOKIE_ACK:
            case SHUTDOWN_ACK:
            case HEARTBEAT_ACK:
                delete storedHeader;
                delete liveHeader;
                break;
            case RE_CONFIG: {
                SCTPStreamResetChunk* liveReconfigChunk = check_and_cast<SCTPStreamResetChunk*>(liveHeader);
                liveReconfigChunk->setName("livereconfig");
                SCTPStreamResetChunk* storedReconfigChunk = check_and_cast<SCTPStreamResetChunk*>(storedHeader);
                if (!(compareReconfigPacket(storedReconfigChunk, liveReconfigChunk))) {
                    delete storedReconfigChunk;
                    delete liveReconfigChunk;
                    EV_DETAIL << "RECONFIG chunks are not the same" << endl;
                    return false;
                }
                delete storedReconfigChunk;
                delete liveReconfigChunk;
                break;
            }
            default:
                printf("type %d not implemented\n", type);
        }
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
        if (!(storedDataChunk->getTsn() + localDiffTsn == liveDataChunk->getTsn()))
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
    localDiffTsn = liveInitChunk->getInitTSN() - initLocalTsn;
    initPeerTsn = liveInitChunk->getInitTSN();
    localCumTsn = initPeerTsn - 1;
    peerCumTsn = initLocalTsn - 1;

    if (!(flags & FLAG_INIT_CHUNK_TSN_NOCHECK))
        if (!(storedInitChunk->getInitTSN() + localDiffTsn == liveInitChunk->getInitTSN()))
            return false;
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
    localDiffTsn = liveInitAckChunk->getInitTSN() - initLocalTsn;
    initPeerTsn = liveInitAckChunk->getInitTSN();
    localCumTsn = initPeerTsn - 1;
    peerCumTsn = initLocalTsn - 1;
    if (!(flags & FLAG_INIT_ACK_CHUNK_A_RWND_NOCHECK))
        if (!(storedInitAckChunk->getA_rwnd() == liveInitAckChunk->getA_rwnd()))
            return false;
    if (!(flags & FLAG_INIT_ACK_CHUNK_OS_NOCHECK))
        if (!(min(storedInitAckChunk->getNoOutStreams(), peerInStreams) == liveInitAckChunk->getNoOutStreams()))
            return false;
    if (!(flags & FLAG_INIT_ACK_CHUNK_IS_NOCHECK))
        if (!(min(storedInitAckChunk->getNoInStreams(), peerOutStreams) == liveInitAckChunk->getNoInStreams()))
            return false;
    if (!(flags & FLAG_INIT_ACK_CHUNK_TSN_NOCHECK))
        if (!(storedInitAckChunk->getInitTSN() + localDiffTsn == liveInitAckChunk->getInitTSN()))
            return false;
    peerCookie = check_and_cast<SCTPCookie*>(liveInitAckChunk->getStateCookie());
    peerCookieLength = peerCookie->getByteLength();
    return true;
}

bool PacketDrillApp::compareReconfigPacket(SCTPStreamResetChunk* storedReconfigChunk, SCTPStreamResetChunk* liveReconfigChunk)
{
    bool found = false;

    uint32 flags = storedReconfigChunk->getFlags();
    if (!(storedReconfigChunk->getParametersArraySize() == liveReconfigChunk->getParametersArraySize())) {
        return false;
    }
    for (unsigned int i = 0; i < storedReconfigChunk->getParametersArraySize(); i++) {
        SCTPParameter *storedParameter = check_and_cast<SCTPParameter *>(storedReconfigChunk->getParameters(i));
        SCTPParameter *liveParameter = nullptr;
        found = false;
        switch (storedParameter->getParameterType()) {
            case OUTGOING_RESET_REQUEST_PARAMETER: {
                SCTPOutgoingSSNResetRequestParameter *storedoutparam = check_and_cast<SCTPOutgoingSSNResetRequestParameter *>(storedParameter);
                for (unsigned int j = 0; j < liveReconfigChunk->getParametersArraySize(); j++) {
                    liveParameter = check_and_cast<SCTPParameter *>(liveReconfigChunk->getParameters(j));
                    if (liveParameter->getParameterType() != OUTGOING_RESET_REQUEST_PARAMETER)
                        continue;
                    else {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
                SCTPOutgoingSSNResetRequestParameter *liveoutparam = check_and_cast<SCTPOutgoingSSNResetRequestParameter *>(liveParameter);
                if (seqNumMap[storedoutparam->getSrReqSn()] == 0) {
                    seqNumMap[storedoutparam->getSrReqSn()] = liveoutparam->getSrReqSn();
                } else if (!(flags & FLAG_RECONFIG_REQ_SN_NOCHECK))
                    if (!(seqNumMap[storedoutparam->getSrReqSn()] == liveoutparam->getSrReqSn())) {
                        return false;
                }
                if (seqNumMap[storedoutparam->getSrResSn()] == 0) {
                    seqNumMap[storedoutparam->getSrResSn()] = liveoutparam->getSrResSn();
                }
                if (!(flags & FLAG_RECONFIG_LAST_TSN_NOCHECK))
                    if (!(storedoutparam->getLastTsn() + localDiffTsn == liveoutparam->getLastTsn()))
                        return false;
                if (!(storedoutparam->getStreamNumbersArraySize() == liveoutparam->getStreamNumbersArraySize()))
                    return false;
                if (storedoutparam->getStreamNumbersArraySize() > 0) {
                    for (uint16 i = 0; i < storedoutparam->getStreamNumbersArraySize(); i++) {
                        if (!(storedoutparam->getStreamNumbers(i) == liveoutparam->getStreamNumbers(i)))
                            return false;
                    }
                }
                break;
            }
            case INCOMING_RESET_REQUEST_PARAMETER: {
                found = false;
                SCTPIncomingSSNResetRequestParameter *storedinparam = check_and_cast<SCTPIncomingSSNResetRequestParameter *>(storedParameter);
                for (unsigned int j = 0; j < liveReconfigChunk->getParametersArraySize(); j++) {
                    liveParameter = check_and_cast<SCTPParameter *>(liveReconfigChunk->getParameters(j));
                    if (liveParameter->getParameterType() != INCOMING_RESET_REQUEST_PARAMETER)
                        continue;
                    else {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
                SCTPIncomingSSNResetRequestParameter *liveinparam = check_and_cast<SCTPIncomingSSNResetRequestParameter *>(liveParameter);
                if (seqNumMap[storedinparam->getSrReqSn()] == 0) {
                    seqNumMap[storedinparam->getSrReqSn()] = liveinparam->getSrReqSn();
                } else if (!(seqNumMap[storedinparam->getSrReqSn()] == liveinparam->getSrReqSn())) {
                    return false;
                }
                if (!(storedinparam->getStreamNumbersArraySize() == liveinparam->getStreamNumbersArraySize()))
                    return false;
                if (storedinparam->getStreamNumbersArraySize() > 0) {
                    for (uint16 i = 0; i < storedinparam->getStreamNumbersArraySize(); i++) {
                        if (!(storedinparam->getStreamNumbers(i) == liveinparam->getStreamNumbers(i)))
                            return false;
                    }
                }
                break;
            }
            case STREAM_RESET_RESPONSE_PARAMETER: {
                SCTPStreamResetResponseParameter *storedresparam = check_and_cast<SCTPStreamResetResponseParameter *>(storedParameter);
                liveParameter = check_and_cast<SCTPParameter *>(liveReconfigChunk->getParameters(i));
                if (liveParameter->getParameterType() != STREAM_RESET_RESPONSE_PARAMETER) {
                    break;
                }
                SCTPStreamResetResponseParameter *liveresparam = check_and_cast<SCTPStreamResetResponseParameter *>(liveParameter);
                if (!(storedresparam->getSrResSn() == liveresparam->getSrResSn())) {
                    return false;
                }
                if (!(flags & FLAG_RECONFIG_RESULT_NOCHECK))
                    if (!(storedresparam->getResult() == liveresparam->getResult()))
                        return false;
                if (storedresparam->getSendersNextTsn() != 0 && storedresparam->getResult() == PERFORMED) {
                    if (!(flags & FLAG_RECONFIG_SENDER_NEXT_TSN_NOCHECK))
                        if (!(storedresparam->getSendersNextTsn() + localDiffTsn == liveresparam->getSendersNextTsn()))
                            return false;
                    if (!(flags & FLAG_RECONFIG_RECEIVER_NEXT_TSN_NOCHECK))
                        if (!(storedresparam->getReceiversNextTsn() == liveresparam->getReceiversNextTsn()))
                            return false;
                }
                break;
            }
            case SSN_TSN_RESET_REQUEST_PARAMETER: {
                found = false;
                SCTPSSNTSNResetRequestParameter *storedinparam = check_and_cast<SCTPSSNTSNResetRequestParameter *>(storedParameter);
                for (unsigned int j = 0; j < liveReconfigChunk->getParametersArraySize(); j++) {
                    liveParameter = check_and_cast<SCTPParameter *>(liveReconfigChunk->getParameters(j));
                    if (liveParameter->getParameterType() != SSN_TSN_RESET_REQUEST_PARAMETER)
                        continue;
                    else {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
                SCTPSSNTSNResetRequestParameter *liveinparam = check_and_cast<SCTPSSNTSNResetRequestParameter *>(liveParameter);
                if (seqNumMap[storedinparam->getSrReqSn()] == 0) {
                    seqNumMap[storedinparam->getSrReqSn()] = liveinparam->getSrReqSn();
                } else if (!(seqNumMap[storedinparam->getSrReqSn()] == liveinparam->getSrReqSn())) {
                    return false;
                }
                break;
            }
            case ADD_INCOMING_STREAMS_REQUEST_PARAMETER: {
                found = false;
                SCTPAddStreamsRequestParameter *storedaddparam = check_and_cast<SCTPAddStreamsRequestParameter *>(storedParameter);
                for (unsigned int j = 0; j < liveReconfigChunk->getParametersArraySize(); j++) {
                    liveParameter = check_and_cast<SCTPParameter *>(liveReconfigChunk->getParameters(j));
                    if (liveParameter->getParameterType() != ADD_INCOMING_STREAMS_REQUEST_PARAMETER)
                        continue;
                    else {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
                SCTPAddStreamsRequestParameter *liveaddparam = check_and_cast<SCTPAddStreamsRequestParameter *>(liveParameter);
                if (seqNumMap[storedaddparam->getSrReqSn()] == 0) {
                    seqNumMap[storedaddparam->getSrReqSn()] = liveaddparam->getSrReqSn();
                } else if (!(seqNumMap[storedaddparam->getSrReqSn()] == liveaddparam->getSrReqSn())) {
                    return false;
                }
                if (!(storedaddparam->getNumberOfStreams() == liveaddparam->getNumberOfStreams()))
                    return false;
                break;
            }
            case ADD_OUTGOING_STREAMS_REQUEST_PARAMETER: {
                found = false;
                SCTPAddStreamsRequestParameter *storedaddparam = check_and_cast<SCTPAddStreamsRequestParameter *>(storedParameter);
                for (unsigned int j = 0; j < liveReconfigChunk->getParametersArraySize(); j++) {
                    liveParameter = check_and_cast<SCTPParameter *>(liveReconfigChunk->getParameters(j));
                    if (liveParameter->getParameterType() != ADD_OUTGOING_STREAMS_REQUEST_PARAMETER)
                        continue;
                    else {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
                SCTPAddStreamsRequestParameter *liveaddparam = check_and_cast<SCTPAddStreamsRequestParameter *>(liveParameter);
                if (seqNumMap[storedaddparam->getSrReqSn()] == 0) {
                    seqNumMap[storedaddparam->getSrReqSn()] = liveaddparam->getSrReqSn();
                } else if (!(seqNumMap[storedaddparam->getSrReqSn()] == liveaddparam->getSrReqSn())) {
                    return false;
                }
                if (!(storedaddparam->getNumberOfStreams() == liveaddparam->getNumberOfStreams()))
                    return false;
                break;
            }
            default: printf("Reconfig Parameter %d not implemented\n", storedParameter->getParameterType());
        }
    }
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

    if (storedSackChunk->getNumGaps() > 0) {
        for (int i = 0; i < storedSackChunk->getNumGaps(); i++)
        {
            if (!(storedSackChunk->getGapStart(i) == (liveSackChunk->getGapStart(i) - peerCumTsn))
                    || !(storedSackChunk->getGapStop(i) == (liveSackChunk->getGapStop(i) - peerCumTsn))) {
                return false;
            }
        }
    }

    if (!(flags & FLAG_SACK_CHUNK_DUP_TSNS_NOCHECK))
        if (!(storedSackChunk->getNumDupTsns() == liveSackChunk->getNumDupTsns()))
            return false;

    if (storedSackChunk->getNumDupTsns() > 0) {
        for (int i = 0; i < storedSackChunk->getNumDupTsns(); i++) {
            if (!(storedSackChunk->getDupTsns(i) == liveSackChunk->getDupTsns(i))) {
                return false;
            }
        }
    }

    return true;
}

} // namespace INET
