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


#include <stdio.h>
#include <stdlib.h>

#include "inet/applications/common/SocketTag_m.h"
#include "inet/applications/packetdrill/PacketDrillApp.h"
#include "inet/applications/packetdrill/PacketDrillInfo_m.h"
#include "inet/applications/packetdrill/PacketDrillUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/configurator/ipv4/Ipv4NodeConfigurator.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"


namespace inet {

Define_Module(PacketDrillApp);

using namespace sctp;
using namespace tcp;

#define MSGKIND_START  0
#define MSGKIND_EVENT  1

PacketDrillApp::PacketDrillApp()
{
}

void PacketDrillApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // parameters
        msgArrived = false;
        recvFromSet = false;
        listenSet = false;
        acceptSet = false;
        establishedPending = false;
        socketOptionsArrived_ = false;
        abortSent = false;
        receivedPackets = new cPacketQueue("receiveQueue");
        outboundPackets = new cPacketQueue("outboundPackets");
        expectedMessageSize = 0;
        eventCounter = 0;
        numEvents = 0;
        localVTag = 0;
        eventTimer = new cMessage("event timer", MSGKIND_EVENT);
        simStartTime = simTime();
        simRelTime = simTime();
    } else if (stage == INITSTAGE_APPLICATION_LAYER) {
        if (operationalState != OPERATING)
            throw cRuntimeError("This module doesn't support starting in NOT_OPERATING state");
        pd = new PacketDrill(this);
        config = new PacketDrillConfig();
        script = new PacketDrillScript(par("scriptFile").stringValue());
        localAddress = L3Address(par("localAddress"));
        remoteAddress = L3Address(par("remoteAddress"));
        localPort = par("localPort");
        remotePort = par("remotePort");
        const char *crcModeString = par("crcMode");
        crcMode = parseCrcMode(crcModeString, false);
        const char *interface = par("interface");
        //const char *interfaceTableModule = par("interfaceTableModule");
        IInterfaceTable *interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        InterfaceEntry *interfaceEntry = interfaceTable->findInterfaceByName(interface);
        if (interfaceEntry == nullptr)
            throw cRuntimeError("TUN interface not found: %s", interface);
        auto idat = interfaceEntry->getProtocolDataForUpdate<Ipv4InterfaceData>();
        idat->setIPAddress(localAddress.toIpv4());
        tunSocket.setOutputGate(gate("socketOut"));
        tunSocket.open(interfaceEntry->getInterfaceId());
        tunInterfaceId = interfaceEntry->getInterfaceId();
        tunSocketId = tunSocket.getSocketId();

        cMessage* timeMsg = new cMessage("PacketDrillAppTimer", MSGKIND_START);
        scheduleAt(par("startTime"), timeMsg);
    }
}

void PacketDrillApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    if (recvFromSet) {
        recvFromSet = false;
        msgArrived = false;
        if (!(packet->getByteLength() == expectedMessageSize)) {
            throw cTerminationException("Packetdrill error: Received data has unexpected size");
        }
        if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
            eventCounter++;
            scheduleEvent();
        }
        delete packet;
    } else {
        PacketDrillInfo* info = new PacketDrillInfo();
        info->setLiveTime(getSimulation()->getSimTime());
        packet->setContextPointer(info);
        receivedPackets->insert(packet);
        msgArrived = true;
        if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
            eventCounter++;
            scheduleEvent();
        }
    }
}

// UdpSocket:

void PacketDrillApp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
}

void PacketDrillApp::socketClosed(UdpSocket *socket)
{
}

// TcpSocket:

void PacketDrillApp::socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent)
{
    if (recvFromSet)
    {
        auto *msg = new Request("data request", TCP_C_READ);
        TcpCommand *cmd = new TcpCommand();
        msg->addTag<SocketReq>()->setSocketId(tcpConnId);
        msg->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::tcp);
        msg->setControlInfo(cmd);
        send(msg, "socketOut");       //send to TCP
        recvFromSet = false;
        // send a receive request to TCP
    }
    msgArrived = true;
    delete msg;
}

void PacketDrillApp::socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo)
{
    // new TCP connection -- create new socket object and server process
    TcpSocket *newSocket = new TcpSocket(availableInfo);
    newSocket->setOutputGate(gate("socketOut"));
    newSocket->setCallback(this);
    socketMap.addSocket(newSocket);
    socket->accept(newSocket->getSocketId());
}

void PacketDrillApp::socketEstablished(TcpSocket *socket)
{
}

void PacketDrillApp::socketPeerClosed(TcpSocket *socket)
{
}

void PacketDrillApp::socketClosed(TcpSocket *socket)
{
    delete socketMap.removeSocket(socket);
}

void PacketDrillApp::socketFailure(TcpSocket *socket, int code)
{
    delete socketMap.removeSocket(socket);
}

// SctpSocket:

void PacketDrillApp::socketDataArrived(SctpSocket *socket, Packet *packet, bool urgent)
{
    PacketDrillEvent *event = check_and_cast<PacketDrillEvent *>(script->getEventList()->get(eventCounter));
    if (verifyTime(event->getTimeType(), event->getEventTime(), event->getEventTimeEnd(),
            event->getEventOffset(), getSimulation()->getSimTime(), "inbound packet") == STATUS_ERR)
    {
        delete packet;
        throw cTerminationException("Packetdrill error: Packet arrived at the wrong time");
    }
    if (!(packet->getByteLength() == expectedMessageSize)) {
        throw cTerminationException("Packetdrill error: Delivered message has wrong size");
    }
    msgArrived = false;
    recvFromSet = false;
    if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
        eventCounter++;
        scheduleEvent();
    }
}

void PacketDrillApp::socketDataNotificationArrived(SctpSocket *socket, Message *msg)
{
    if (recvFromSet) {
        Packet* cmsg = new Packet("ReceiveRequest", SCTP_C_RECEIVE);
        auto cmd = cmsg->addTag<SctpSendReq>();
        cmd->setSocketId(sctpAssocId);
        cmsg->addTag<SocketReq>()->setSocketId(sctpAssocId);
        cmsg->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
        cmd->setSid(0);
        send(cmsg, "socketOut");       //send to SCTP
        recvFromSet = false;
    }
    if (sctpSocket.getState() == SctpSocket::CLOSED) {
        sctpSocket.abort();
        abortSent = true;
    }
    if (!abortSent)
        msgArrived = true;
}

void PacketDrillApp::socketAvailable(SctpSocket *socket, Indication *indication)
{
    SctpSocket *newSocket = new SctpSocket(indication);
    newSocket->setOutputGate(gate("socketOut"));
    newSocket->setCallback(this);
    socketMap.addSocket(newSocket);
    int newSocketId = newSocket->getSocketId();
    sctpAssocId = newSocketId;
    EV_INFO << "Sending accept socket id request ..." << endl;
    socket->acceptSocket(newSocketId);
    delete indication;
}

void PacketDrillApp::socketEstablished(SctpSocket *socket, unsigned long int buffer)
{
    EV_INFO << "SCTP_I_ESTABLISHED" << endl;
}

void PacketDrillApp::socketOptionsArrived(SctpSocket *socket, Indication *indication)
{
    sctpSocket.setUserOptions((SocketOptions*)(indication->getContextPointer()));
    socketOptionsArrived_ = true;
    if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
        eventCounter++;
        scheduleEvent();
    }
    delete indication;
}

void PacketDrillApp::socketPeerClosed(SctpSocket *socket) {}

void PacketDrillApp::socketClosed(SctpSocket *socket)
{
    delete socketMap.removeSocket(socket);
}

void PacketDrillApp::socketFailure(SctpSocket *socket, int code)
{
    delete socketMap.removeSocket(socket);
}

void PacketDrillApp::socketStatusArrived(SctpSocket *socket, SctpStatusReq *status) { }
void PacketDrillApp::socketDeleted(SctpSocket *socket) {}
void PacketDrillApp::sendRequestArrived(SctpSocket *socket) {}
void PacketDrillApp::msgAbandonedArrived(SctpSocket *socket) {}
void PacketDrillApp::shutdownReceivedArrived(SctpSocket *socket) {}
void PacketDrillApp::sendqueueFullArrived(SctpSocket *socket) {}
void PacketDrillApp::sendqueueAbatedArrived(SctpSocket *socket, unsigned long int buffer) {}
void PacketDrillApp::addressAddedArrived(SctpSocket *socket, L3Address localAddr, L3Address remoteAddr) {}

void PacketDrillApp::socketDataArrived(TunSocket *socket, Packet *packet)
{
    // received from tunnel interface
    if (outboundPackets->getLength() == 0) {
        cEvent *nextMsg = getSimulation()->getScheduler()->guessNextEvent();
        if (nextMsg) {
            if ((simTime() + par("latency")) < nextMsg->getArrivalTime()) {
                delete (PacketDrillInfo *)packet->getContextPointer();
                delete packet;
                throw cTerminationException("Packetdrill error: Packet arrived at the wrong time");
            } else {
                PacketDrillInfo *info = new PacketDrillInfo();
                info->setLiveTime(getSimulation()->getSimTime());
                packet->setContextPointer(info);
                receivedPackets->insert(packet);
            }
        }
    } else {
        Packet *ipv4Packet = check_and_cast<Packet *>(outboundPackets->pop());
       // const auto& ipv4Header = ipv4Packet->peekAtFront<Ipv4Header>();
        Packet *liveIpv4Packet = packet;
       // const auto& liveIpv4Header = liveIpv4Packet->peekAtFront<Ipv4Header>();
        PacketDrillInfo *info = (PacketDrillInfo *)ipv4Packet->getContextPointer();
        if (verifyTime(static_cast<eventTime_t>(info->getTimeType()), info->getScriptTime(),
                info->getScriptTimeEnd(), info->getOffset(), getSimulation()->getSimTime(), "outbound packet")
                == STATUS_ERR) {
            throw cTerminationException("Packetdrill error: Packet arrived at the wrong time");
        }
        if (!compareDatagram(ipv4Packet, liveIpv4Packet)) {
            throw cTerminationException("Packetdrill error: Datagrams are not the same");
        }
        delete info;
        if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
            eventCounter++;
            scheduleEvent();
        }
        delete (PacketDrillInfo *)packet->getContextPointer();
        delete packet;
    }
}

void PacketDrillApp::socketClosed(TunSocket *socket)
{
    delete socketMap.removeSocket(socket);
}

void PacketDrillApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleTimer(msg);
    }
    else {
        if (! msg->arrivedOn("socketIn"))
            throw cRuntimeError("Message arrived on unknown gate %s", msg->getArrivalGate()->getFullName());

        ISocket *socket = socketMap.findSocketFor(msg);
        if (socket) {
            socket->processMessage(msg);
        }
        else if (udpSocket.belongsToSocket(msg)) {
            // received from UDP
            PacketDrillEvent *event = check_and_cast<PacketDrillEvent *>(script->getEventList()->get(eventCounter));
            if (verifyTime(event->getTimeType(), event->getEventTime(), event->getEventTimeEnd(),
                    event->getEventOffset(), getSimulation()->getSimTime(), "inbound packet") == STATUS_ERR) {
                delete msg;
                throw cTerminationException("Packetdrill error: Packet arrived at the wrong time");
            }
            udpSocket.processMessage(msg);
        }
        else if (tcpSocket.belongsToSocket(msg)) {
            tcpSocket.processMessage(msg);
        }
        else if (sctpSocket.belongsToSocket(msg)) {
            sctpSocket.processMessage(msg);
        }
        else if (tunSocket.belongsToSocket(msg)) {
            tunSocket.processMessage(msg);
            std::cout << __func__ << ":" << __LINE__ << endl;
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
        offsetLastEvent = (check_and_cast<PacketDrillEvent *>(script->getEventList()->get(eventCounter - 1)))->getEventTime() - simStartTime;
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
    PacketDrillEvent *event = check_and_cast<PacketDrillEvent *>(script->getEventList()->get(eventCounter));
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
        Packet *pk = event->getPacket()->getInetPacket();
        if (event->getPacket()->getDirection() == DIRECTION_INBOUND) { // < injected packet, will go through the stack bottom up.
            auto packetByteLength = pk->getDataLength();
            auto ipHeader = pk->removeAtFront<Ipv4Header>();
            // remove lower layer paddings:
            ASSERT(B(ipHeader->getTotalLengthField()) >= ipHeader->getChunkLength());
            if (ipHeader->getTotalLengthField() < packetByteLength)
                pk->setBackOffset(B(ipHeader->getTotalLengthField()) - ipHeader->getChunkLength());

            if (protocol == IP_PROT_TCP) {
                auto tcpHeader = pk->removeAtFront<TcpHeader>();
                tcpHeader->setAckNo(tcpHeader->getAckNo() + relSequenceOut);
                if (tcpHeader->getHeaderOptionArraySize() > 0) {
                    for (unsigned int i = 0; i < tcpHeader->getHeaderOptionArraySize(); i++) {
                        if (tcpHeader->getHeaderOption(i)->getKind() == TCPOPT_TIMESTAMP) {
                            TcpOptionTimestamp *option = new TcpOptionTimestamp();
                            option->setEchoedTimestamp(peerTS);
                            tcpHeader->setHeaderOption(i, option);
                        }
                    }
                }
                pk->insertAtFront(tcpHeader);
                snprintf(str, sizeof(str), "inbound %d", eventCounter);
                pk->setName(str);
            }
            else if (protocol == IP_PROT_SCTP) {
                auto sctpHeader = pk->removeAtFront<SctpHeader>();
                sctpHeader->setVTag(peerVTag);
                int32 noChunks = sctpHeader->getSctpChunksArraySize();
                for (int32 cc = 0; cc < noChunks; cc++) {
                    SctpChunk *chunk = const_cast<SctpChunk *>(sctpHeader->getSctpChunks(cc));
                    unsigned char chunkType = chunk->getSctpChunkType();
                    switch (chunkType) {
                        case INIT: {
                            SctpInitChunk* init = check_and_cast<SctpInitChunk*>(chunk);
                            peerInStreams = init->getNoInStreams();
                            peerOutStreams = init->getNoOutStreams();
                            initPeerTsn = init->getInitTsn();
                            localVTag = init->getInitTag();
                            peerCumTsn = initPeerTsn - 1;
                            break;
                        }
                        case INIT_ACK: {
                            SctpInitAckChunk* initack = check_and_cast<SctpInitAckChunk*>(chunk);
                            localVTag = initack->getInitTag();
                            initPeerTsn = initack->getInitTsn();
                            peerCumTsn = initPeerTsn - 1;
                            break;
                        }
                        case COOKIE_ECHO: {
                            SctpCookieEchoChunk* cookieEcho = check_and_cast<SctpCookieEchoChunk*>(chunk);
                            int tempLength = cookieEcho->getByteLength();
                            peerCookie->setName("CookieEchoStateCookie");
                            cookieEcho->setStateCookie(peerCookie);
                            peerCookie = nullptr;
                            cookieEcho->setByteLength(SCTP_COOKIE_ACK_LENGTH + peerCookieLength);
                            int length = B(sctpHeader->getChunkLength()).get() - tempLength + cookieEcho->getByteLength();
                            sctpHeader->setChunkLength(B(length));
                            break;
                        }
                        case SACK: {
                            SctpSackChunk* sack = check_and_cast<SctpSackChunk*>(chunk);
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
                            sctpHeader->setSctpChunks(cc, sack);
                            break;
                        }
                        case RE_CONFIG:{
                            SctpStreamResetChunk* reconfig = check_and_cast<SctpStreamResetChunk*>(chunk);
                            for (unsigned int i = 0; i < reconfig->getParametersArraySize(); i++) {
                                auto *parameter = reconfig->getParametersForUpdate(i);
                                switch (parameter->getParameterType()) {
                                    case STREAM_RESET_RESPONSE_PARAMETER: {
                                        SctpStreamResetResponseParameter *param = check_and_cast<SctpStreamResetResponseParameter *>(parameter);
                                        param->setSrResSn(seqNumMap[param->getSrResSn()]);
                                        if (param->getReceiversNextTsn() != 0) {
                                            param->setReceiversNextTsn(param->getReceiversNextTsn() + localDiffTsn);
                                        }
                                        break;
                                    }
                                    case OUTGOING_RESET_REQUEST_PARAMETER: {
                                        auto *param = check_and_cast<SctpOutgoingSsnResetRequestParameter *>(parameter);
                                        if (findSeqNumMap(param->getSrResSn())) {
                                            param->setSrResSn(seqNumMap[param->getSrResSn()]);
                                        }
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
                pk->insertAtFront(sctpHeader);
                pk->setName("inboundSctp");
            }
            else {
                // other protocol
            }
            ipHeader->setTotalLengthField(ipHeader->getChunkLength() + pk->getDataLength());
            pk->insertAtFront(ipHeader);
            tunSocket.send(pk);
        } else if (event->getPacket()->getDirection() == DIRECTION_OUTBOUND) { // >
            if (receivedPackets->getLength() > 0) {
                Packet *livePacket = check_and_cast<Packet *>(receivedPackets->pop());
                if (pk && livePacket) {
                    PacketDrillInfo *liveInfo = (PacketDrillInfo *)livePacket->getContextPointer();
                    if (verifyTime(event->getTimeType(), event->getEventTime(),
                            event->getEventTimeEnd(), event->getEventOffset(), liveInfo->getLiveTime(),
                            "outbound packet") == STATUS_ERR) {
                        throw cTerminationException("Packetdrill error: Timing error");
                    }
                    if (!compareDatagram(pk, livePacket)) {
                        throw cTerminationException("Packetdrill error: Datagrams are not the same");
                    }
                    delete liveInfo;
                    if (!eventTimer->isScheduled() && eventCounter < numEvents - 1) {
                        eventCounter++;
                        scheduleEvent();
                    }
                }
                delete livePacket;
                delete pk;
            } else {
                if (protocol == IP_PROT_SCTP) {
                    const auto& ipHeader = pk->peekAtFront<Ipv4Header>();
                    const auto& sctpHeader = pk->peekDataAt<SctpHeader>(ipHeader->getChunkLength());
                    const SctpChunk *sctpChunk = sctpHeader->getSctpChunks(0);
                    if (sctpChunk->getSctpChunkType() == INIT) {
                        auto *init = check_and_cast<const SctpInitChunk*>(sctpChunk);
                        initLocalTsn = init->getInitTsn();
                        peerVTag = init->getInitTag();
                        localCumTsn = initLocalTsn - 1;
                        sctpSocket.setInboundStreams(init->getNoInStreams());
                        sctpSocket.setOutboundStreams(init->getNoOutStreams());
                    }
                    else if (sctpChunk->getSctpChunkType() == INIT_ACK) {
                        auto *initack = check_and_cast<const SctpInitAckChunk*>(sctpChunk);
                        initLocalTsn = initack->getInitTsn();
                        peerVTag = initack->getInitTag();
                        localCumTsn = initLocalTsn - 1;
                    }
                }
                PacketDrillInfo *info = new PacketDrillInfo("outbound");
                info->setScriptTime(event->getEventTime());
                info->setScriptTimeEnd(event->getEventTimeEnd());
                info->setOffset(event->getEventOffset());
                info->setTimeType(event->getTimeType());
                pk->setContextPointer(info);
                snprintf(str, sizeof(str), "outbound %d", eventCounter);
                pk->setName(str);
                outboundPackets->insert(pk);
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
            if ((socketOptionsArrived_ && !recvFromSet && outboundPackets->getLength() == 0) &&
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
    Packet *pk = new Packet("IPCleanup");
    SctpAbortChunk *abortChunk = new SctpAbortChunk("Abort");
    abortChunk->setSctpChunkType(ABORT);
    abortChunk->setT_Bit(1);
    abortChunk->setByteLength(SCTP_ABORT_CHUNK_LENGTH);
    auto sctpmsg = makeShared<SctpHeader>();
    sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));
    sctpmsg->setSrcPort(remotePort);
    sctpmsg->setDestPort(localPort);
    sctpmsg->setVTag(peerVTag);
    pk->setName("SCTPCleanUp");
    sctpmsg->setChecksumOk(true);
    sctpmsg->setCrcMode(crcMode);
    sctpmsg->insertSctpChunks(abortChunk);
    pk->insertAtFront(sctpmsg);
    auto ipv4Header = makeShared<Ipv4Header>();
    ipv4Header->setSrcAddress(remoteAddress.toIpv4());
    ipv4Header->setDestAddress(localAddress.toIpv4());
    ipv4Header->setIdentification(0);
    ipv4Header->setVersion(4);
    ipv4Header->setHeaderLength(IPv4_MIN_HEADER_LENGTH);
    ipv4Header->setProtocolId(IP_PROT_SCTP);
    ipv4Header->setTimeToLive(32);
    ipv4Header->setMoreFragments(0);
    ipv4Header->setDontFragment(0);
    ipv4Header->setFragmentOffset(0);
    ipv4Header->setTypeOfService(0);
    ipv4Header->setCrcMode(crcMode);
    ipv4Header->setCrc(0);
    ipv4Header->setTotalLengthField(ipv4Header->getChunkLength() + pk->getDataLength());
    pk->insertAtFront(ipv4Header);
    EV_DETAIL << "Send Abort to cleanup association." << endl;

    tunSocket.send(pk);
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
    int result = STATUS_OK;

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
        result = syscallSocket(syscall, args, &error);
    } else if (!strcmp(name, "bind")) {
        result = syscallBind(syscall, args, &error);
    } else if (!strcmp(name, "listen")) {
        result = syscallListen(syscall, args, &error);
    } else if (!strcmp(name, "write") || !strcmp(name, "send")) {
        result = syscallWrite(syscall, args, &error);
    } else if (!strcmp(name, "read")) {
        result = syscallRead(event, syscall, args, &error);
    } else if (!strcmp(name, "sendto")) {
        result = syscallSendTo(syscall, args, &error);
    } else if (!strcmp(name, "recvfrom")) {
        result = syscallRecvFrom(event, syscall, args, &error);
    } else if (!strcmp(name, "close")) {
        result = syscallClose(syscall, args, &error);
    } else if (!strcmp(name, "shutdown")) {
        result = syscallShutdown(syscall, args, &error);
    } else if (!strcmp(name, "connect")) {
        result = syscallConnect(syscall, args, &error);
    } else if (!strcmp(name, "accept")) {
        result = syscallAccept(syscall, args, &error);
    } else if (!strcmp(name, "setsockopt")) {
        result = syscallSetsockopt(syscall, args, &error);
    } else if (!strcmp(name, "getsockopt")) {
        result = syscallGetsockopt(syscall, args, &error);
    } else if (!strcmp(name, "sctp_sendmsg")) {
        result = syscallSctpSendmsg(syscall, args, &error);
    } else if (!strcmp(name, "sctp_send")) {
        result = syscallSctpSend(syscall, args, &error);
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
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS)) {
        return STATUS_ERR;
    }
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(1));
    if (!exp || exp->getS32(&type, error)) {
        return STATUS_ERR;
    }
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(2));
    if (!exp || exp->getS32(&protocol, error)) {
        return STATUS_ERR;
    }

    switch (protocol) {
        case IP_PROT_UDP:
            udpSocket.setOutputGate(gate("socketOut"));
            udpSocket.bind(localPort);
            break;

        case IP_PROT_TCP:
            tcpSocket.setOutputGate(gate("socketOut"));
            tcpSocket.bind(localPort);
            break;
        case IP_PROT_SCTP:
            sctpSocket.setOutputGate(gate("socketOut"));
            sctpAssocId = sctpSocket.getSocketId();
            if (sctpSocket.getOutboundStreams() == -1) {
                sctpSocket.setOutboundStreams(par("outboundStreams"));
            }
            if (sctpSocket.getInboundStreams() == -1) {
                sctpSocket.setInboundStreams(par("inboundStreams"));
            }
            sctpSocket.bind(localAddress, localPort);
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
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(1));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(2));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;

    switch (protocol) {
        case IP_PROT_UDP:
            break;

        case IP_PROT_TCP:
            if (tcpSocket.getState() == TcpSocket::NOT_BOUND) {
                tcpSocket.bind(localAddress, localPort);
            }
            break;
        case IP_PROT_SCTP:
            if (sctpSocket.getState() == SctpSocket::NOT_BOUND)
            {
                sctpSocket.bind(localAddress, localPort);
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
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(1));
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

    PacketDrillExpression* exp = syscall->result;
    if (!exp || exp->getS32(&script_accepted_fd, error))
        return STATUS_ERR;
    if (establishedPending) {
        if (protocol == IP_PROT_TCP)
            tcpSocket.setState(TcpSocket::CONNECTED);
        else if (protocol == IP_PROT_SCTP)
            sctpSocket.setState(SctpSocket::CONNECTED);
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
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(1));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(2));
    if (!exp || exp->getS32(&count, error))
        return STATUS_ERR;

    switch (protocol)
    {
        case IP_PROT_TCP: {
            Packet *payload = new Packet("Write");
            payload->setByteLength(syscall->result->getNum());
            tcpSocket.send(payload);
            break;
        }
        case IP_PROT_SCTP: {
            Packet* cmsg = new Packet("AppData", SCTP_C_SEND_ORDERED);
            auto applicationData = makeShared<BytesChunk>();
            uint32 sendBytes = syscall->result->getNum();
            std::vector<uint8_t> vec;
            vec.resize(sendBytes);
            for (uint32 i = 0; i < sendBytes; i++)
                vec[i] = (bytesSent + i) & 0xFF;
            applicationData->setBytes(vec);
            applicationData->addTag<CreationTimeTag>()->setCreationTime(simTime());

            cmsg->insertAtBack(applicationData);
            auto sendCommand = cmsg->addTag<SctpSendReq>();
            sendCommand->setLast(true);
            sendCommand->setSocketId(-1);
            sendCommand->setSendUnordered(false);
            sendCommand->setSid(0);

            sctpSocket.send(cmsg);
            break;
        }
        default:
            EV_INFO << "Protocol not supported for this socket call";
            break;
    }

    return STATUS_OK;
}


int PacketDrillApp::syscallConnect(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd;
    PacketDrillExpression *exp;

    if (args->getLength() != 3)
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(1));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(2));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;

    switch (protocol) {
        case IP_PROT_UDP:
            break;

        case IP_PROT_TCP:
            tcpSocket.connect(remoteAddress, remotePort);
            break;
        case IP_PROT_SCTP: {
            sctpSocket.setTunInterface(tunInterfaceId);
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
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(1));
    if (!exp || exp->getS32(&level, error))
        return STATUS_ERR;
    if (level != IPPROTO_SCTP) {
        return STATUS_ERR;
    }
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(2));
    if (!exp || exp->getS32(&optname, error))
        return STATUS_ERR;

    exp = check_and_cast<PacketDrillExpression *>(args->get(3));

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
            Message *cmsg = new Message("SCTP_C_STREAM_RESET", SCTP_C_STREAM_RESET);
            auto rinfo = cmsg->addTag<SctpResetReq>();
            rinfo->setSocketId(-1);
            rinfo->setFd(rs->srs_assoc_id->getNum());
            rinfo->setRemoteAddr(sctpSocket.getRemoteAddr());
            if (rs->srs_number_streams->getNum() > 0 && rs->srs_stream_list != nullptr) {
                rinfo->setStreamsArraySize(rs->srs_number_streams->getNum());
                cQueue *qu = rs->srs_stream_list->getList();
                uint16 i = 0;
                for (cQueue::Iterator iter(*qu); !iter.end(); iter++, i++) {
                    rinfo->setStreams(i, check_and_cast<PacketDrillExpression *>(*iter)->getNum());
                    qu->remove((*iter));
                }
                qu->clear();
            }
            if (rs->srs_flags->getNum() == SCTP_STREAM_RESET_OUTGOING) {
                rinfo->setRequestType(RESET_OUTGOING);
            } else if (rs->srs_flags->getNum() == SCTP_STREAM_RESET_INCOMING) {
                rinfo->setRequestType(RESET_INCOMING);
            } else if (rs->srs_flags->getNum() == (SCTP_STREAM_RESET_OUTGOING | SCTP_STREAM_RESET_INCOMING)) {
                rinfo->setRequestType(RESET_BOTH);
            }
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
            Message *cmsg = new Message("SCTP_C_ADD_STREAMS", SCTP_C_ADD_STREAMS);
            auto rinfo = cmsg->addTag<SctpResetReq>();
            rinfo->setSocketId(-1);
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
                    break;
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

            PacketDrillExpression *exp2 = check_and_cast<PacketDrillExpression *>(exp->getList()->pop());
            exp2->getS32(&value, error);
            switch (optname)
            {
                case SCTP_NODELAY:
                    sctpSocket.setNagle(value? 0 : 1);
                    break;
                case SCTP_RESET_ASSOC: {
                    Message *cmsg = new Message("SCTP_C_STREAM_RESET", SCTP_C_RESET_ASSOC);
                    auto rinfo = cmsg->addTag<SctpResetReq>();
                    rinfo->setSocketId(-1);
                    rinfo->setFd(value);
                    rinfo->setRemoteAddr(sctpSocket.getRemoteAddr());
                    rinfo->setRequestType(SSN_TSN);
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
            break;
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
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(1));
    if (!exp || exp->getS32(&level, error))
        return STATUS_ERR;
    if (level != IPPROTO_SCTP) {
        return STATUS_ERR;
    }
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(2));
    if (!exp || exp->getS32(&optname, error))
        return STATUS_ERR;

    exp = check_and_cast<PacketDrillExpression *>(args->get(3));
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
        break;
    }
    return STATUS_OK;
}

int PacketDrillApp::syscallSendTo(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd, count, flags;
    PacketDrillExpression *exp;

    if (args->getLength() != 6)
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(1));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(2));
    if (!exp || exp->getS32(&count, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(3));
    if (!exp || exp->getS32(&flags, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(4));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(5));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;

    Packet *payload = new Packet("SendTo");
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
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(1));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(2));
    if (!exp || exp->getS32(&count, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(3));
    /*ToDo: handle address parameter */
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(4));
    /*ToDo: handle tolen parameter */
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(5));
    if (!exp || exp->getU32(&ppid, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(6));
    if (!exp || exp->getU32(&flags, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(7));
    if (!exp || exp->getU16(&stream_no, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(8));
    if (!exp || exp->getU32(&ttl, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(9));
    if (!exp || exp->getU32(&context, error))
        return STATUS_ERR;

    Packet* cmsg = new Packet("AppData");
    uint32 sendBytes = syscall->result->getNum();
    auto applicationData = makeShared<BytesChunk>();
    std::vector<uint8_t> vec;
    vec.resize(sendBytes);
    for (uint32 i = 0; i < sendBytes; i++)
        vec[i] = (bytesSent + i) & 0xFF;
    applicationData->setBytes(vec);
    applicationData->addTag<CreationTimeTag>()->setCreationTime(simTime());
    cmsg->insertAtBack(applicationData);

    auto sendCommand = cmsg->addTag<SctpSendReq>();
    sendCommand->setLast(true);
    sendCommand->setSocketId(sctpAssocId);
    sendCommand->setSid(stream_no);
    sendCommand->setPpid(ppid);
    if (flags == SCTP_UNORDERED) {
        sendCommand->setSendUnordered(true);
    }

    sctpSocket.send(cmsg);
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
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(1));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(2));
    if (!exp || exp->getS32(&count, error))
        return STATUS_ERR;
    exp = check_and_cast<PacketDrillExpression *>(args->get(3));
    if (exp->getType() == EXPR_SCTP_SNDRCVINFO) {
        struct sctp_sndrcvinfo_expr *info = exp->getSndRcvInfo();
        ssn = info->sinfo_ssn->getNum();
        sid = info->sinfo_stream->getNum();
        ppid = info->sinfo_ppid->getNum();
    }
    Packet* cmsg = new Packet("AppData");
    auto applicationData = makeShared<BytesChunk>();
    uint32 sendBytes = syscall->result->getNum();
    std::vector<uint8_t> vec;
    vec.resize(sendBytes);
    for (uint32 i = 0; i < sendBytes; i++)
        vec[i] = (bytesSent + i) & 0xFF;
    applicationData->setBytes(vec);
    applicationData->addTag<CreationTimeTag>()->setCreationTime(simTime());
    cmsg->insertAtBack(applicationData);

    auto sendCommand = cmsg->addTag<SctpSendReq>();
    sendCommand->setLast(true);
    sendCommand->setSocketId(-1);
    sendCommand->setSid(sid);
    sendCommand->setPpid(ppid);
    sendCommand->setSsn(ssn);
    sendCommand->setSendUnordered(false);

    sctpSocket.send(cmsg);
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
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(1));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(2));
    if (!exp || exp->getS32(&count, error))
        return STATUS_ERR;

    if ((expectedMessageSize = syscall->result->getNum()) > 0) {
        if (msgArrived || receivedPackets->getLength() > 0) {
            switch (protocol) {
                case IP_PROT_TCP: {
                    Request *msg = new Request("dataRequest", TCP_C_READ);
                    TcpCommand *tcpcmd = new TcpCommand();
                    msg->addTag<SocketReq>()->setSocketId(tcpConnId);
                    msg->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::tcp);
                    msg->setControlInfo(tcpcmd);
                    send(msg, "socketOut");       //send to TCP
                    break;
                }
                case IP_PROT_SCTP: {
                    Packet* pkt = new Packet("dataRequest", SCTP_C_RECEIVE);
                    auto sctpcmd = pkt->addTag<SctpSendReq>();
                    sctpcmd->setSocketId(sctpAssocId);
                    sctpcmd->setSid(0);
                    pkt->addTag<SocketReq>()->setSocketId(sctpAssocId);
                    pkt->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
                    send(pkt, "socketOut");       //send to SCTP
                    break;
                }
                default:
                    EV_INFO << "Protocol not supported for this system call.";
                    break;
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
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || exp->getS32(&script_fd, err))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(1));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(2));
    if (!exp || exp->getS32(&count, err))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(3));
    if (!exp || exp->getS32(&flags, err))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(4));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;
    exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(5));
    if (!exp || (exp->getType() != EXPR_ELLIPSIS))
        return STATUS_ERR;

    if (msgArrived) {
        cPacket *msg = (receivedPackets->pop());
        msgArrived = false;
        recvFromSet = false;
        if (!(msg->getByteLength() == syscall->result->getNum())) {
            delete msg;
            throw cTerminationException("Packetdrill error: Wrong payload length");
        }
        PacketDrillInfo *info = (PacketDrillInfo *)msg->getContextPointer();
        if (verifyTime(event->getTimeType(), event->getEventTime(), event->getEventTimeEnd(),
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
    PacketDrillExpression *exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;

    switch (protocol) {
        case IP_PROT_UDP: {
            EV_DETAIL << "close UDP socket\n";
            udpSocket.close();
            break;
        }

        case IP_PROT_TCP: {
            Request *msg = new Request("close", TCP_C_CLOSE);
            TcpCommand *cmd = new TcpCommand();
            msg->addTag<SocketReq>()->setSocketId(tcpConnId);
            msg->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::tcp);
            msg->setControlInfo(cmd);
            send(msg, "socketOut");       //send to TCP
            break;
        }
        case IP_PROT_SCTP: {
            sctpSocket.close(script_fd);
            break;
        }
        default:
            EV_INFO << "Protocol " << protocol << " is not supported for this system call\n";
            break;
    }
    return STATUS_OK;
}

int PacketDrillApp::syscallShutdown(struct syscall_spec *syscall, cQueue *args, char **error)
{
    int script_fd;
printf("syscallShutdown\n");
    if (args->getLength() != 2)
        return STATUS_ERR;
    PacketDrillExpression *exp = check_and_cast_nullable<PacketDrillExpression *>(args->get(0));
    if (!exp || exp->getS32(&script_fd, error))
        return STATUS_ERR;

    switch (protocol) {
        case IP_PROT_SCTP: {
            sctpSocket.shutdown(script_fd);
            break;
        }
        default:
            EV_INFO << "Protocol " << protocol << " is not supported for this system call\n";
            break;
    }
    return STATUS_OK;
}

void PacketDrillApp::finish()
{
    EV_INFO << "PacketDrillApp finished\n";
}

PacketDrillApp::~PacketDrillApp()
{
    cancelAndDelete(eventTimer);
    delete pd;
    delete receivedPackets;
    delete outboundPackets;
    delete config;
    delete script;
    socketMap.deleteSockets();
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

bool PacketDrillApp::compareDatagram(Packet *storedPacket, Packet *livePacket)
{
    const auto& storedDatagram = storedPacket->peekAtFront<Ipv4Header>();
    const auto& liveDatagram = livePacket->peekAtFront<Ipv4Header>();

   /* if (!(storedDatagram->getSrcAddress() == liveDatagram->getSrcAddress())) {
        return false;
    }*/
    std::cout << __LINE__ << endl;
    if (!(storedDatagram->getDestAddress() == liveDatagram->getDestAddress())) {
        return false;
    }
    if (!(storedDatagram->getProtocolId() == liveDatagram->getProtocolId())) {
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
    switch (storedDatagram->getProtocolId()) {
        case IP_PROT_UDP: {
            const auto& storedUdp = storedPacket->peekDataAt<UdpHeader>(storedDatagram->getChunkLength());
            const auto& liveUdp = livePacket->peekDataAt<UdpHeader>(liveDatagram->getChunkLength());
            if (!(compareUdpHeader(storedUdp, liveUdp))) {
                return false;
            }
            break;
        }

        case IP_PROT_TCP: {
            const auto& storedTcp = storedPacket->peekDataAt<TcpHeader>(storedDatagram->getChunkLength());
            const auto& liveTcp = livePacket->peekDataAt<TcpHeader>(liveDatagram->getChunkLength());
            if (storedTcp->getSynBit()) { // SYN was sent. Store the sequence number for comparisons
                relSequenceOut = liveTcp->getSequenceNo();
            }
            if (storedTcp->getSynBit() && storedTcp->getAckBit()) {
                peerWindow = liveTcp->getWindow();
            }
            if (!(compareTcpHeader(storedTcp, liveTcp))) {
                return false;
            }
            break;
        }
        case IP_PROT_SCTP: {
            const auto& storedSctp = storedPacket->peekDataAt<SctpHeader>(storedDatagram->getChunkLength());
            const auto& liveSctp = livePacket->peekDataAt<SctpHeader>(liveDatagram->getChunkLength());
            if (!(compareSctpPacket(storedSctp, liveSctp))) {
                EV_DETAIL << "SCTP packets are not the same" << endl;
                return false;
            }
            break;
        }
        default:
            EV_INFO << "Transport protocol %d is not supported yet" << storedDatagram->getProtocolId();
            break;
    }
    return true;
}

bool PacketDrillApp::compareUdpHeader(const Ptr<const UdpHeader>& storedUdp, const Ptr<const UdpHeader>& liveUdp)
{
    return (storedUdp->getSourcePort() == liveUdp->getSourcePort())
            && (storedUdp->getDestinationPort() == liveUdp->getDestinationPort())
            && (storedUdp->getTotalLengthField() == liveUdp->getTotalLengthField())
            ;
}

bool PacketDrillApp::compareTcpHeader(const Ptr<const TcpHeader>& storedTcp, const Ptr<const TcpHeader>& liveTcp)
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
//            const TcpOption *liveOption;
//            for (unsigned int i = 0; i < liveTcp->getHeaderOptionArraySize(); i++) {
//                liveOption = liveTcp->getHeaderOption(i);
//            }
            return false;
        } else {
            const TcpOption *storedOption, *liveOption;
            for (unsigned int i = 0; i < storedTcp->getHeaderOptionArraySize(); i++) {
                storedOption = storedTcp->getHeaderOption(i);
                liveOption = liveTcp->getHeaderOption(i);
                if (storedOption->getKind() == liveOption->getKind()) {
                    switch (storedOption->getKind()) {
                        case TCPOPTION_END_OF_OPTION_LIST:
                        case TCPOPTION_NO_OPERATION:
                            if (!(storedOption->getLength() == liveOption->getLength())) {
                                return false;
                            }
                            break;
                        case TCPOPTION_SACK_PERMITTED:
                            if (!(storedOption->getLength() == liveOption->getLength() &&
                                storedOption->getLength() == 2)) {
                                return false;
                            }
                            break;
                        case TCPOPTION_WINDOW_SCALE:
                            if (!(storedOption->getLength() == liveOption->getLength() &&
                                storedOption->getLength() == 3 &&
                                check_and_cast<const TcpOptionWindowScale *>(storedOption)->getWindowScale()
                                 == check_and_cast<const TcpOptionWindowScale *>(liveOption)->getWindowScale())) {
                                return false;
                            }
                            break;
                        case TCPOPTION_SACK:
                            if (!(storedOption->getLength() == liveOption->getLength() &&
                                storedOption->getLength() > 2 && (storedOption->getLength() % 8) == 2 &&
                                check_and_cast<const TcpOptionSack *>(storedOption)->getSackItemArraySize()
                                == check_and_cast<const TcpOptionSack *>(liveOption)->getSackItemArraySize())) {
                                return false;
                            }
                            break;
                        case TCPOPTION_TIMESTAMP:
                            if (!(storedOption->getLength() == liveOption->getLength() &&
                                storedOption->getLength() == 10 &&
                                check_and_cast<const TcpOptionTimestamp *>(storedOption)->getSenderTimestamp()
                                == check_and_cast<const TcpOptionTimestamp *>(liveOption)->getSenderTimestamp())) {
                                return false;
                            }
                            break;
                        default:
                            EV_INFO << "TCP Option type=" << storedOption->getKind() << " not supported";
                            break;
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

bool PacketDrillApp::compareSctpPacket(const Ptr<const SctpHeader>& storedSctp, const Ptr<const SctpHeader>& liveSctp)
{
    if (!(storedSctp->getSrcPort() == liveSctp->getSrcPort())) {
        return false;
    }
    if (!(storedSctp->getDestPort() == liveSctp->getDestPort())) {
        return false;
    }
    if (!(storedSctp->getSctpChunksArraySize() == liveSctp->getSctpChunksArraySize())) {
        return false;
    }

    const uint32 numberOfChunks = storedSctp->getSctpChunksArraySize();
    for (uint32 i = 0; i < numberOfChunks; i++) {
        const SctpChunk* storedHeader = storedSctp->getSctpChunks(i);
        const SctpChunk* liveHeader = liveSctp->getSctpChunks(i);
        if (!(storedHeader->getSctpChunkType() == liveHeader->getSctpChunkType())) {
            return false;
        }
        const uint8 type = storedHeader->getSctpChunkType();

        if ((type != INIT && type != INIT_ACK) && type != ABORT && (liveSctp->getVTag() != localVTag)) {
            EV_DETAIL << " VTag " << liveSctp->getVTag() << " incorrect. Should be " << localVTag << " peerVTag="
                    << peerVTag << endl;
            return false;
        }

        switch (type) {
            case DATA: {
                auto *storedDataChunk = check_and_cast<const SctpDataChunk*>(storedHeader);
                auto *liveDataChunk = check_and_cast<const SctpDataChunk*>(liveHeader);
                if (!(compareDataPacket(storedDataChunk, liveDataChunk))) {
                    EV_DETAIL << "DATA chunks are not the same" << endl;
                    return false;
                }
                break;
            }
            case INIT: {
                auto *storedInitChunk = check_and_cast<const SctpInitChunk*>(storedHeader);
                auto *liveInitChunk = check_and_cast<const SctpInitChunk*>(liveHeader);
                if (!(compareInitPacket(storedInitChunk, liveInitChunk))) {
                    EV_DETAIL << "INIT chunks are not the same" << endl;
                    return false;
                }
                break;
            }
            case INIT_ACK: {
                auto *storedInitAckChunk = check_and_cast<const SctpInitAckChunk*>(storedHeader);
                auto *liveInitAckChunk = check_and_cast<const SctpInitAckChunk*>(liveHeader);
                if (!(compareInitAckPacket(storedInitAckChunk, liveInitAckChunk))) {
                    EV_DETAIL << "INIT-ACK chunks are not the same" << endl;
                    return false;
                }
                break;
            }
            case SACK: {
                auto *storedSackChunk = check_and_cast<const SctpSackChunk*>(storedHeader);
                auto *liveSackChunk = check_and_cast<const SctpSackChunk*>(liveHeader);
                if (!(compareSackPacket(storedSackChunk, liveSackChunk))) {
                    EV_DETAIL << "SACK chunks are not the same" << endl;
                    return false;
                }
                break;
            }
            case COOKIE_ECHO: {
                auto *storedCookieEchoChunk = check_and_cast<const SctpCookieEchoChunk*>(storedHeader);
                if (!(storedCookieEchoChunk->getFlags() & FLAG_CHUNK_VALUE_NOCHECK))
                    printf("COOKIE_ECHO chunks should be compared\n");
                else
                    printf("Do not check cookie echo chunks\n");
                break;
            }
            case SHUTDOWN: {
                auto *storedShutdownChunk = check_and_cast<const SctpShutdownChunk*>(storedHeader);
                auto *liveShutdownChunk = check_and_cast<const SctpShutdownChunk*>(liveHeader);
                if (!(storedShutdownChunk->getFlags() & FLAG_SHUTDOWN_CHUNK_CUM_TSN_NOCHECK)) {
                    if (!(storedShutdownChunk->getCumTsnAck() == liveShutdownChunk->getCumTsnAck())) {
                        EV_DETAIL << "SHUTDOWN chunks are not the same" << endl;
                        return false;
                    }
                }
                break;
            }
            case SHUTDOWN_COMPLETE: {
                auto *storedShutdownCompleteChunk = check_and_cast<const SctpShutdownCompleteChunk*>(storedHeader);
                auto *liveShutdownCompleteChunk = check_and_cast<const SctpShutdownCompleteChunk*>(liveHeader);
                if (!(storedShutdownCompleteChunk->getFlags() & FLAG_CHUNK_FLAGS_NOCHECK))
                    if (!(storedShutdownCompleteChunk->getTBit() == liveShutdownCompleteChunk->getTBit())) {
                        EV_DETAIL << "SHUTDOWN-COMPLETE chunks are not the same" << endl;
                        return false;
                    }
                break;
            }
            case ABORT: {
                auto *storedAbortChunk = check_and_cast<const SctpAbortChunk*>(storedHeader);
                auto *liveAbortChunk = check_and_cast<const SctpAbortChunk*>(liveHeader);
                if (!(storedAbortChunk->getFlags() & FLAG_CHUNK_FLAGS_NOCHECK))
                    if (!(storedAbortChunk->getT_Bit() == liveAbortChunk->getT_Bit())) {
                        EV_DETAIL << "ABORT chunks are not the same" << endl;
                        return false;
                    }
                break;
            }
            case ERRORTYPE: {
                auto *storedErrorChunk = check_and_cast<const SctpErrorChunk*>(storedHeader);
                auto *liveErrorChunk = check_and_cast<const SctpErrorChunk*>(liveHeader);
                if (!(storedErrorChunk->getParametersArraySize() == liveErrorChunk->getParametersArraySize())) {
                    return false;
                }
                if (storedErrorChunk->getParametersArraySize() > 0) {
                // Only Cause implemented so far.
                    auto *storedcause = check_and_cast<const SctpSimpleErrorCauseParameter *>(storedErrorChunk->getParameters(0));
                    auto *livecause = check_and_cast<const SctpSimpleErrorCauseParameter *>(liveErrorChunk->getParameters(0));
                    if (!(storedcause->getValue() == livecause->getValue())) {
                        return false;
                    }
                }
                break;
            }
            case HEARTBEAT: {
                auto *heartbeatChunk = check_and_cast<const SctpHeartbeatChunk*>(liveHeader);
                peerHeartbeatTime = heartbeatChunk->getTimeField();
                break;
            }
            case COOKIE_ACK:
            case SHUTDOWN_ACK:
            case HEARTBEAT_ACK:
                break;
            case RE_CONFIG: {
                auto *liveReconfigChunk = check_and_cast<const SctpStreamResetChunk*>(liveHeader);
                // liveReconfigChunk->setName("livereconfig");          //FIXME Why???
                auto *storedReconfigChunk = check_and_cast<const SctpStreamResetChunk*>(storedHeader);
                if (!(compareReconfigPacket(storedReconfigChunk, liveReconfigChunk))) {
                    EV_DETAIL << "RECONFIG chunks are not the same" << endl;
                    return false;
                }
                break;
            }
            default:
                printf("type %d not implemented\n", type);
                break;
        }
    }
    return true;
}

bool PacketDrillApp::compareDataPacket(const SctpDataChunk* storedDataChunk, const SctpDataChunk* liveDataChunk)
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

bool PacketDrillApp::compareInitPacket(const SctpInitChunk* storedInitChunk, const SctpInitChunk* liveInitChunk)
{
    uint32 flags = storedInitChunk->getFlags();
    peerVTag = liveInitChunk->getInitTag();
    localDiffTsn = liveInitChunk->getInitTsn() - initLocalTsn;
    initPeerTsn = liveInitChunk->getInitTsn();
    localCumTsn = initPeerTsn - 1;
    peerCumTsn = initLocalTsn - 1;

    if (!(flags & FLAG_INIT_CHUNK_TSN_NOCHECK))
        if (!(storedInitChunk->getInitTsn() + localDiffTsn == liveInitChunk->getInitTsn()))
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

bool PacketDrillApp::compareInitAckPacket(const SctpInitAckChunk* storedInitAckChunk, const SctpInitAckChunk* liveInitAckChunk)
{
    uint32 flags = storedInitAckChunk->getFlags();
    peerVTag = liveInitAckChunk->getInitTag();
    localDiffTsn = liveInitAckChunk->getInitTsn() - initLocalTsn;
    initPeerTsn = liveInitAckChunk->getInitTsn();
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
        if (!(storedInitAckChunk->getInitTsn() + localDiffTsn == liveInitAckChunk->getInitTsn()))
            return false;
    peerCookie = CHK(liveInitAckChunk->getStateCookie())->dup();        //FIXME hack: dup() called for generate a mutable copy
    peerCookieLength = peerCookie->getLength();
    return true;
}

bool PacketDrillApp::compareReconfigPacket(const SctpStreamResetChunk* storedReconfigChunk, const SctpStreamResetChunk* liveReconfigChunk)
{
    bool found = false;

    uint32 flags = storedReconfigChunk->getFlags();
    if (!(storedReconfigChunk->getParametersArraySize() == liveReconfigChunk->getParametersArraySize())) {
        return false;
    }
    for (unsigned int i = 0; i < storedReconfigChunk->getParametersArraySize(); i++) {
        auto *storedParameter = check_and_cast<const SctpParameter *>(storedReconfigChunk->getParameters(i));
        const SctpParameter *liveParameter = nullptr;
        found = false;
        switch (storedParameter->getParameterType()) {
            case OUTGOING_RESET_REQUEST_PARAMETER: {
                auto *storedoutparam = check_and_cast<const SctpOutgoingSsnResetRequestParameter *>(storedParameter);
                for (unsigned int j = 0; j < liveReconfigChunk->getParametersArraySize(); j++) {
                    liveParameter = check_and_cast<const SctpParameter *>(liveReconfigChunk->getParameters(j));
                    if (liveParameter->getParameterType() != OUTGOING_RESET_REQUEST_PARAMETER)
                        continue;
                    else {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
                auto *liveoutparam = check_and_cast<const SctpOutgoingSsnResetRequestParameter *>(liveParameter);
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
                auto *storedinparam = check_and_cast<const SctpIncomingSsnResetRequestParameter *>(storedParameter);
                for (unsigned int j = 0; j < liveReconfigChunk->getParametersArraySize(); j++) {
                    liveParameter = check_and_cast<const SctpParameter *>(liveReconfigChunk->getParameters(j));
                    if (liveParameter->getParameterType() != INCOMING_RESET_REQUEST_PARAMETER)
                        continue;
                    else {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
                auto *liveinparam = check_and_cast<const SctpIncomingSsnResetRequestParameter *>(liveParameter);
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
                auto *storedresparam = check_and_cast<const SctpStreamResetResponseParameter *>(storedParameter);
                liveParameter = check_and_cast<const SctpParameter *>(liveReconfigChunk->getParameters(i));
                if (liveParameter->getParameterType() != STREAM_RESET_RESPONSE_PARAMETER) {
                    break;
                }
                auto *liveresparam = check_and_cast<const SctpStreamResetResponseParameter *>(liveParameter);
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
                auto *storedinparam = check_and_cast<const SctpSsnTsnResetRequestParameter *>(storedParameter);
                for (unsigned int j = 0; j < liveReconfigChunk->getParametersArraySize(); j++) {
                    liveParameter = check_and_cast<const SctpParameter *>(liveReconfigChunk->getParameters(j));
                    if (liveParameter->getParameterType() != SSN_TSN_RESET_REQUEST_PARAMETER)
                        continue;
                    else {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
                auto *liveinparam = check_and_cast<const SctpSsnTsnResetRequestParameter *>(liveParameter);
                if (seqNumMap[storedinparam->getSrReqSn()] == 0) {
                    seqNumMap[storedinparam->getSrReqSn()] = liveinparam->getSrReqSn();
                } else if (!(seqNumMap[storedinparam->getSrReqSn()] == liveinparam->getSrReqSn())) {
                    return false;
                }
                break;
            }
            case ADD_INCOMING_STREAMS_REQUEST_PARAMETER: {
                found = false;
                auto *storedaddparam = check_and_cast<const SctpAddStreamsRequestParameter *>(storedParameter);
                for (unsigned int j = 0; j < liveReconfigChunk->getParametersArraySize(); j++) {
                    liveParameter = check_and_cast<const SctpParameter *>(liveReconfigChunk->getParameters(j));
                    if (liveParameter->getParameterType() != ADD_INCOMING_STREAMS_REQUEST_PARAMETER)
                        continue;
                    else {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
                auto *liveaddparam = check_and_cast<const SctpAddStreamsRequestParameter *>(liveParameter);
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
                auto *storedaddparam = check_and_cast<const SctpAddStreamsRequestParameter *>(storedParameter);
                for (unsigned int j = 0; j < liveReconfigChunk->getParametersArraySize(); j++) {
                    liveParameter = check_and_cast<const SctpParameter *>(liveReconfigChunk->getParameters(j));
                    if (liveParameter->getParameterType() != ADD_OUTGOING_STREAMS_REQUEST_PARAMETER)
                        continue;
                    else {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
                auto *liveaddparam = check_and_cast<const SctpAddStreamsRequestParameter *>(liveParameter);
                if (seqNumMap[storedaddparam->getSrReqSn()] == 0) {
                    seqNumMap[storedaddparam->getSrReqSn()] = liveaddparam->getSrReqSn();
                } else if (!(seqNumMap[storedaddparam->getSrReqSn()] == liveaddparam->getSrReqSn())) {
                    return false;
                }
                if (!(storedaddparam->getNumberOfStreams() == liveaddparam->getNumberOfStreams()))
                    return false;
                break;
            }
            default:
                printf("Reconfig Parameter %d not implemented\n", storedParameter->getParameterType());
                break;
        }
    }
    return true;
}

bool PacketDrillApp::compareSackPacket(const SctpSackChunk *storedSackChunk, const SctpSackChunk *liveSackChunk)
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

void PacketDrillApp::handleStartOperation(LifecycleOperation *operation)
{
    if (operation != nullptr)
        throw cRuntimeError("Lifecycle currently not implemented");
}

void PacketDrillApp::handleStopOperation(LifecycleOperation *operation)
{
    throw cRuntimeError("Lifecycle currently not implemented");
}

void PacketDrillApp::handleCrashOperation(LifecycleOperation *operation)
{
    throw cRuntimeError("Lifecycle currently not implemented");
}

} // namespace INET

