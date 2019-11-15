//
// Copyright (C) 2005-2010 Irene Ruengeler
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#endif // ifdef WITH_IPv6

#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/transportlayer/sctp/Sctp.h"
#include "inet/transportlayer/sctp/SctpAlgorithm.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpQueue.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {

namespace sctp {

void SctpAssociation::calculateRcvBuffer()
{
    uint32 sumDelivery = 0;
    uint32 sumOrdered = 0;
    uint32 sumUnOrdered = 0;
    for (SctpReceiveStreamMap::const_iterator iterator = receiveStreams.begin();
         iterator != receiveStreams.end(); iterator++)
    {
        const SctpReceiveStream *stream = iterator->second;
        sumDelivery += stream->getDeliveryQ()->getQueueSize();
        sumOrdered += stream->getOrderedQ()->getQueueSize();
        sumUnOrdered += stream->getUnorderedQ()->getQueueSize();
    }
    EV_DEBUG << "DeliveryQ= " << sumDelivery
             << ", OrderedQ=" << sumOrdered
             << ", UnorderedQ=" << sumUnOrdered
             << ", bufferedMessages=" << state->bufferedMessages
             << endl;
}

void SctpAssociation::listOrderedQ()
{
    for (auto & elem : receiveStreams) {
        EV_DEBUG << "stream " << elem.second->getStreamId() << ":\n";
        elem.second->getOrderedQ()->printQueue();
        EV_DEBUG << "\n";
    }
}

uint32 SctpAssociation::getBytesInFlightOfStream(uint16 sid)
{
    auto streamIterator = sendStreams.find(sid);
    assert(streamIterator != sendStreams.end());
    return streamIterator->second->getBytesInFlight();
}

bool SctpAssociation::orderedQueueEmptyOfStream(uint16 sid)
{
    auto streamIterator = sendStreams.find(sid);
    assert(streamIterator != sendStreams.end());
    return streamIterator->second->getStreamQ()->isEmpty();
}

bool SctpAssociation::unorderedQueueEmptyOfStream(uint16 sid)
{
    auto streamIterator = sendStreams.find(sid);
    assert(streamIterator != sendStreams.end());
    return streamIterator->second->getUnorderedStreamQ()->isEmpty();
}


bool SctpAssociation::getFragInProgressOfStream(uint16 sid)
{
    auto streamIterator = sendStreams.find(sid);
    assert(streamIterator != sendStreams.end());
    return streamIterator->second->getFragInProgress();
}

void SctpAssociation::setFragInProgressOfStream(uint16 sid, bool frag)
{
    auto streamIterator = sendStreams.find(sid);
    assert(streamIterator != sendStreams.end());
    return streamIterator->second->setFragInProgress(frag);
}

void SctpAssociation::checkPseudoCumAck(const SctpPathVariables *path)
{
    uint32 earliestOutstandingTsn = path->pseudoCumAck;
    uint32 rtxEarliestOutstandingTsn = path->rtxPseudoCumAck;

    retransmissionQ->findEarliestOutstandingTsnsForPath(
            path->remoteAddress,
            earliestOutstandingTsn, rtxEarliestOutstandingTsn);

    if (tsnGt(path->pseudoCumAck, earliestOutstandingTsn) ||
        tsnGt(path->rtxPseudoCumAck, rtxEarliestOutstandingTsn)) {
        std::cerr << "WRONG PSEUDO CUM-ACK!" << endl
                  << "pseudoCumAck=" << path->pseudoCumAck << ", earliestOutstandingTsn=" << earliestOutstandingTsn << endl
                  << "rtxPseudoCumAck=" << path->rtxPseudoCumAck << ", rtxEarliestOutstandingTsn=" << rtxEarliestOutstandingTsn << endl;
    }
}

void SctpAssociation::printSctpPathMap() const
{
    EV_DEBUG << "Sctp PathMap:" << endl;
    for (const auto & elem : sctpPathMap) {
        const SctpPathVariables *path = elem.second;
        EV_DEBUG << " - " << path->remoteAddress << ":  osb=" << path->outstandingBytes
                 << " cwnd=" << path->cwnd << endl;
    }
}

const char *SctpAssociation::stateName(int32 state)
{
#define CASE(x)    case x: \
        s = (char *)#x + 7; break
    const char *s = "unknown";
    switch (state) {
        CASE(SCTP_S_CLOSED);
        CASE(SCTP_S_COOKIE_WAIT);
        CASE(SCTP_S_COOKIE_ECHOED);
        CASE(SCTP_S_ESTABLISHED);
        CASE(SCTP_S_SHUTDOWN_PENDING);
        CASE(SCTP_S_SHUTDOWN_SENT);
        CASE(SCTP_S_SHUTDOWN_RECEIVED);
        CASE(SCTP_S_SHUTDOWN_ACK_SENT);
    }
    return s;
#undef CASE
}

const char *SctpAssociation::eventName(int32 event)
{
#define CASE(x)    case x: \
        s = (char *)#x + 7; break
    const char *s = "unknown";
    switch (event) {
        CASE(SCTP_E_OPEN_PASSIVE);
        CASE(SCTP_E_ASSOCIATE);
        CASE(SCTP_E_SHUTDOWN);
        CASE(SCTP_E_CLOSE);
        CASE(SCTP_E_ABORT);
        CASE(SCTP_E_SEND);
        CASE(SCTP_E_RCV_INIT);
        CASE(SCTP_E_RCV_ABORT);
        CASE(SCTP_E_RCV_VALID_COOKIE_ECHO);
        CASE(SCTP_E_RCV_INIT_ACK);
        CASE(SCTP_E_RCV_COOKIE_ACK);
        CASE(SCTP_E_RCV_SHUTDOWN);
        CASE(SCTP_E_RCV_SHUTDOWN_ACK);
        CASE(SCTP_E_RCV_SHUTDOWN_COMPLETE);
        CASE(SCTP_E_TIMEOUT_INIT_TIMER);
        CASE(SCTP_E_TIMEOUT_SHUTDOWN_TIMER);
        CASE(SCTP_E_TIMEOUT_RTX_TIMER);
        CASE(SCTP_E_TIMEOUT_HEARTBEAT_TIMER);
        CASE(SCTP_E_RECEIVE);
        CASE(SCTP_E_DUP_RECEIVED);
        CASE(SCTP_E_PRIMARY);
        CASE(SCTP_E_QUEUE_MSGS_LIMIT);
        CASE(SCTP_E_QUEUE_BYTES_LIMIT);
        CASE(SCTP_E_NO_MORE_OUTSTANDING);
        CASE(SCTP_E_IGNORE);
        CASE(SCTP_E_DELIVERED);
        CASE(SCTP_E_SEND_SHUTDOWN_ACK);
        CASE(SCTP_E_STOP_SENDING);
        CASE(SCTP_E_STREAM_RESET);
        CASE(SCTP_E_RESET_ASSOC);
        CASE(SCTP_E_ADD_STREAMS);
        CASE(SCTP_E_SEND_ASCONF);
        CASE(SCTP_E_SET_STREAM_PRIO);
        CASE(SCTP_E_ACCEPT);
        CASE(SCTP_E_ACCEPT_SOCKET_ID);
    }
    return s;
#undef CASE
}

//TODO move this function to contrib
const char *SctpAssociation::indicationName(int32 code)
{
#define CASE(x)    case x: \
        s = (char *)#x + 7; break
    const char *s = "unknown";
    switch (code) {
        CASE(SCTP_I_DATA);
        CASE(SCTP_I_DATA_NOTIFICATION);
        CASE(SCTP_I_ESTABLISHED);
        CASE(SCTP_I_PEER_CLOSED);
        CASE(SCTP_I_CLOSED);
        CASE(SCTP_I_CONNECTION_REFUSED);
        CASE(SCTP_I_CONNECTION_RESET);
        CASE(SCTP_I_TIMED_OUT);
        CASE(SCTP_I_STATUS);
        CASE(SCTP_I_ABORT);
        CASE(SCTP_I_SHUTDOWN_RECEIVED);
        CASE(SCTP_I_SEND_MSG);
        CASE(SCTP_I_SENDQUEUE_FULL);
        CASE(SCTP_I_SENDQUEUE_ABATED);
        CASE(SCTP_I_ABANDONED);
        CASE(SCTP_I_SEND_STREAMS_RESETTED);
        CASE(SCTP_I_RCV_STREAMS_RESETTED);
        CASE(SCTP_I_RESET_REQUEST_FAILED);
        CASE(SCTP_I_ADDRESS_ADDED);
        CASE(SCTP_I_SENDSOCKETOPTIONS);
        CASE(SCTP_I_AVAILABLE);
    }
    return s;
#undef CASE
}

uint16 SctpAssociation::chunkToInt(const char *type)
{
    EV_STATICCONTEXT;

    if (strcmp(type, "DATA") == 0)
        return DATA;
    if (strcmp(type, "INIT") == 0)
        return INIT;
    if (strcmp(type, "INIT_ACK") == 0)
        return INIT_ACK;
    if (strcmp(type, "SACK") == 0)
        return SACK;
    if (strcmp(type, "HEARTBEAT") == 0)
        return HEARTBEAT;
    if (strcmp(type, "HEARTBEAT_ACK") == 0)
        return HEARTBEAT_ACK;
    if (strcmp(type, "ABORT") == 0)
        return ABORT;
    if (strcmp(type, "SHUTDOWN") == 0)
        return SHUTDOWN;
    if (strcmp(type, "SHUTDOWN_ACK") == 0)
        return SHUTDOWN_ACK;
    if (strcmp(type, "ERRORTYPE") == 0)
        return ERRORTYPE;
    if (strcmp(type, "COOKIE_ECHO") == 0)
        return COOKIE_ECHO;
    if (strcmp(type, "COOKIE_ACK") == 0)
        return COOKIE_ACK;
    if (strcmp(type, "SHUTDOWN_COMPLETE") == 0)
        return SHUTDOWN_COMPLETE;
    if (strcmp(type, "AUTH") == 0)
        return AUTH;
    if (strcmp(type, "NR-SACK") == 0)
        return NR_SACK;
    if (strcmp(type, "ASCONF_ACK") == 0)
        return ASCONF_ACK;
    if (strcmp(type, "PKTDROP") == 0)
        return PKTDROP;
    if (strcmp(type, "RE_CONFIG") == 0)
        return RE_CONFIG;
    if (strcmp(type, "FORWARD_TSN") == 0)
        return FORWARD_TSN;
    if (strcmp(type, "ASCONF") == 0)
        return ASCONF;
    if (strcmp(type, "IFORWARD_TSN") == 0)
        return IFORWARD_TSN;
    EV_WARN << "ChunkConversion not successful\n";
    return 0xffff;
}

void SctpAssociation::printAssocBrief()
{
    EV_DETAIL << "Connection " << assocId << " "
              << localAddr << ":" << localPort << " to " << remoteAddr << ":" << remotePort
              << "  on app[" << appGateIndex << "],assocId=" << assocId
              << "  in " << stateName(fsm->getState()) << "\n";
}

void SctpAssociation::printSegmentBrief(SctpHeader *sctpmsg)
{
    EV_STATICCONTEXT;

    EV_DETAIL << "." << sctpmsg->getSrcPort() << " > "
              << "." << sctpmsg->getDestPort() << ": "
              << "initTag " << sctpmsg->getVTag() << "\n";
}

SctpAssociation *SctpAssociation::cloneAssociation()
{
    SctpAssociation *assoc = new SctpAssociation(sctpMain, appGateIndex, assocId, rt, ift);
    const char *queueClass = transmissionQ->getClassName();
    assoc->transmissionQ = check_and_cast<SctpQueue *>(inet::utils::createOne(queueClass));
    assoc->retransmissionQ = check_and_cast<SctpQueue *>(inet::utils::createOne(queueClass));

    const char *sctpAlgorithmClass = sctpAlgorithm->getClassName();
    assoc->sctpAlgorithm = check_and_cast<SctpAlgorithm *>(inet::utils::createOne(sctpAlgorithmClass));
    assoc->sctpAlgorithm->setAssociation(assoc);
    assoc->sctpAlgorithm->initialize();
    assoc->state = assoc->sctpAlgorithm->createStateVariables();

    if (sctpMain->par("auth").boolValue()) {
        const char *chunks = sctpMain->par("chunks");
        bool asc = false;
        bool asca = false;
        char *chunkscopy = (char *)malloc(strlen(chunks) + 1);
        strcpy(chunkscopy, chunks);
        char *token;
        token = strtok(chunkscopy, ",");
        while (token != nullptr) {
            if (chunkToInt(token) == ASCONF)
                asc = true;
            if (chunkToInt(token) == ASCONF_ACK)
                asca = true;
            if (!typeInOwnChunkList(chunkToInt(token))) {
                this->state->chunkList.push_back(chunkToInt(token));
            }
            token = strtok(nullptr, ",");
        }
        if (sctpMain->par("addIP").boolValue()) {
            if (!asc && !typeInOwnChunkList(ASCONF))
                state->chunkList.push_back(ASCONF);
            if (!asca && !typeInOwnChunkList(ASCONF_ACK))
                state->chunkList.push_back(ASCONF_ACK);
        }
        free(chunkscopy);
    }

    assoc->state->active = false;
    assoc->state->fork = true;
    assoc->localAddr = localAddr;
    assoc->localPort = localPort;
    assoc->localAddressList = localAddressList;
    assoc->listening = true;

    assoc->outboundStreams = outboundStreams;
    assoc->inboundStreams = inboundStreams;
    assoc->fd = fd;

    FSM_Goto((*assoc->fsm), SCTP_S_CLOSED);
    sctpMain->printInfoAssocMap();
    return assoc;
}

void SctpAssociation::sendToIP(Packet *pkt, const Ptr<SctpHeader>& sctpmsg,
        L3Address dest)
{
    // Final touches on the segment before sending
    sctpmsg->setSrcPort(localPort);
    sctpmsg->setDestPort(remotePort);
    sctpmsg->setCrc(0);
    sctpmsg->setCrcMode(sctpMain->crcMode);
    sctpmsg->setChecksumOk(true);
    EV_INFO << "SendToIP: localPort=" << localPort << " remotePort=" << remotePort << " dest=" << dest << "\n";
    const SctpChunk *chunk = sctpmsg->peekFirstChunk();
    uint8 chunkType = chunk->getSctpChunkType();
    if (chunkType == ABORT) {
        const SctpAbortChunk *abortChunk = check_and_cast<const SctpAbortChunk *>(chunk);
        if (abortChunk->getT_Bit() == 1) {
            sctpmsg->setVTag(peerVTag);
        }
        else {
            sctpmsg->setVTag(localVTag);
        }
    }
    else if (sctpmsg->getVTag() == 0) {
        sctpmsg->setVTag(localVTag);
    }


    EV_INFO << "insertTransportProtocolHeader sctpmsg\n";
    insertTransportProtocolHeader(pkt, Protocol::sctp, sctpmsg);

    if (sctpMain->par("udpEncapsEnabled").boolValue()) {
        auto udpHeader = makeShared<UdpHeader>();
        udpHeader->setSourcePort(SCTP_UDP_PORT);
        udpHeader->setDestinationPort(SCTP_UDP_PORT);
        udpHeader->setTotalLengthField(udpHeader->getChunkLength() + pkt->getTotalLength());
        EV_INFO << "Packet: " << pkt << endl;
        udpHeader->setCrcMode(sctpMain->crcMode);
        insertTransportProtocolHeader(pkt, Protocol::udp, udpHeader);
        EV_INFO << "After udp header added " << pkt << endl;
        pkt->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::udp);
    } else {
        pkt->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::sctp);
    }

    IL3AddressType *addressType = dest.getAddressType();
    pkt->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());

    if (sctpMain->getInterfaceId() != -1) {
        pkt->addTagIfAbsent<InterfaceReq>()->setInterfaceId(sctpMain->getInterfaceId());
    }
    auto addresses = pkt->addTagIfAbsent<L3AddressReq>();
    // addresses->setSrcAddress(localAddr);
    addresses->setDestAddress(dest);
    pkt->addTagIfAbsent<SocketReq>()->setSocketId(assocId);
    EV_INFO << "send packet " << pkt << " to ipOut\n";
    check_and_cast<Sctp *>(getSimulation()->getContextModule())->send(pkt, "ipOut");

    if (chunkType == HEARTBEAT) {
        SctpPathVariables *path = getPath(dest);
        path->numberOfHeartbeatsSent++;
        path->vectorPathHb->record(path->numberOfHeartbeatsSent);
    }
    else if (chunkType == HEARTBEAT_ACK) {
        SctpPathVariables *path = getPath(dest);
        path->numberOfHeartbeatAcksSent++;
        path->vectorPathHbAck->record(path->numberOfHeartbeatAcksSent);
    }

    EV_INFO << "Sent to " << dest << endl;
}

void SctpAssociation::signalConnectionTimeout()
{
    sendIndicationToApp(SCTP_I_TIMED_OUT);
}

void SctpAssociation::sendIndicationToApp(int32 code, int32 value)
{
    EV_INFO << "sendIndicationToApp: " << indicationName(code) << endl;
    assert(code != SCTP_I_SENDQUEUE_ABATED);

    Indication *msg = new Indication(indicationName(code), code);

    SctpCommandReq *indication = msg->addTag<SctpCommandReq>();
    indication->setSocketId(assocId);
    indication->setLocalAddr(localAddr);
    indication->setLocalPort(localPort);
    indication->setRemoteAddr(remoteAddr);
    indication->setRemotePort(remotePort);
    msg->addTag<SocketInd>()->setSocketId(assocId);
    sctpMain->send(msg, "appOut");
}

void SctpAssociation::sendAvailableIndicationToApp()
{
    EV_INFO << "sendAvailableIndicationToApp: localPort="
            << localPort << " remotePort=" << remotePort << endl;

    Indication *msg = new Indication(indicationName(SCTP_I_AVAILABLE), SCTP_I_AVAILABLE);

    auto availableIndication = msg->addTag<SctpAvailableReq>();
   // SctpAvailableInfo *availableIndication = new SctpAvailableInfo("SctpAvailableInfo");
    availableIndication->setSocketId(listeningAssocId);
    availableIndication->setLocalAddr(localAddr);
    availableIndication->setRemoteAddr(remoteAddr);
    availableIndication->setLocalPort(localPort);
    availableIndication->setRemotePort(remotePort);
    availableIndication->setNewSocketId(assocId);
    msg->addTag<SocketInd>()->setSocketId(listeningAssocId);
  //  msg->setControlInfo(availableIndication);
    sctpMain->send(msg, "appOut");
}

void SctpAssociation::sendEstabIndicationToApp()
{
    EV_INFO << "sendEstabIndicationToApp: localPort="
            << localPort << " remotePort=" << remotePort << " assocId=" << assocId << endl;

    Indication *msg = new Indication(indicationName(SCTP_I_ESTABLISHED), SCTP_I_ESTABLISHED);

    auto establishIndication = msg->addTag<SctpConnectReq>();
   // SctpConnectInfo *establishIndication = new SctpConnectInfo("ConnectInfo");
    establishIndication->setSocketId(assocId);
    establishIndication->setLocalAddr(localAddr);
    establishIndication->setRemoteAddr(remoteAddr);
    establishIndication->setLocalPort(localPort);
    establishIndication->setRemotePort(remotePort);
    establishIndication->setRemoteAddresses(remoteAddressList);
    establishIndication->setInboundStreams(inboundStreams);
    establishIndication->setOutboundStreams(outboundStreams);
    establishIndication->setNumMsgs(state->sendQueueLimit);
    msg->addTag<SocketInd>()->setSocketId(assocId);
   // msg->setControlInfo(establishIndication);
    sctpMain->send(msg, "appOut");

    char vectorName[128];
    for (uint16 i = 0; i < inboundStreams; i++) {
        snprintf(vectorName, sizeof(vectorName), "Stream %d Throughput", i);
        streamThroughputVectors[i] = new cOutVector(vectorName);
    }
}

void SctpAssociation::sendToApp(cMessage *msg)
{
    auto& tags = getTags(msg);
    tags.addTagIfAbsent<SocketInd>()->setSocketId(assocId);
    sctpMain->send(msg, "appOut");
}

void SctpAssociation::initAssociation(SctpOpenReq *openCmd)
{
    EV_INFO << "SctpAssociationUtil:initAssociation\n";
    // create send/receive queues
    const char *queueClass = openCmd->getQueueClass();
    transmissionQ = check_and_cast<SctpQueue *>(inet::utils::createOne(queueClass));

    retransmissionQ = check_and_cast<SctpQueue *>(inet::utils::createOne(queueClass));
    inboundStreams = openCmd->getInboundStreams();
    outboundStreams = openCmd->getOutboundStreams();
    // create algorithm
    const char *sctpAlgorithmClass = openCmd->getSctpAlgorithmClass();
    if (!sctpAlgorithmClass || !sctpAlgorithmClass[0])
        sctpAlgorithmClass = sctpMain->par("sctpAlgorithmClass");
    sctpAlgorithm = check_and_cast<SctpAlgorithm *>(inet::utils::createOne(sctpAlgorithmClass));
    sctpAlgorithm->setAssociation(this);
    sctpAlgorithm->initialize();
    // create state block
    state = sctpAlgorithm->createStateVariables();

    if (sctpMain->par("auth").boolValue()) {
        const char *chunks = sctpMain->par("chunks");
        bool asc = false;
        bool asca = false;
        char *chunkscopy = (char *)malloc(strlen(chunks) + 1);
        strcpy(chunkscopy, chunks);
        char *token;
        token = strtok(chunkscopy, ",");
        while (token != nullptr) {
            if (chunkToInt(token) == ASCONF)
                asc = true;
            if (chunkToInt(token) == ASCONF_ACK)
                asca = true;
            this->state->chunkList.push_back(chunkToInt(token));
            token = strtok(nullptr, ",");
        }
        if (sctpMain->par("addIP").boolValue()) {
            if (!asc)
                state->chunkList.push_back(ASCONF);
            if (!asca)
                state->chunkList.push_back(ASCONF_ACK);
        }
        free(chunkscopy);
    }
}

void SctpAssociation::sendInit()
{
    AddressVector adv;
    uint32 length = SCTP_INIT_CHUNK_LENGTH;

    if (remoteAddr.isUnspecified() || remotePort == 0)
        throw cRuntimeError("Error processing command ASSOCIATE: foreign socket unspecified");

    if (localPort == 0)
        throw cRuntimeError("Error processing command ASSOCIATE: local port unspecified");

    state->setPrimaryPath(getPath(remoteAddr));
    // create message consisting of INIT chunk
    const auto& sctpmsg = makeShared<SctpHeader>();
    sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));
    SctpInitChunk *initChunk = new SctpInitChunk();
    initChunk->setSctpChunkType(INIT);
    initChunk->setInitTag((uint32)(fmod(RNGCONTEXT intrand(INT32_MAX), 1.0 + (double)(unsigned)0xffffffffUL)) & 0xffffffffUL);

    peerVTag = initChunk->getInitTag();
    EV_INFO << "INIT from " << localAddr << ":InitTag=" << peerVTag << "\n";
    initChunk->setA_rwnd(sctpMain->par("arwnd"));
    state->localRwnd = sctpMain->par("arwnd");
    initChunk->setNoOutStreams(outboundStreams);
    initInboundStreams = inboundStreams;
    initChunk->setNoInStreams(inboundStreams);
    initChunk->setInitTsn(1000);
    initChunk->setMsg_rwnd(sctpMain->par("messageAcceptLimit"));
    state->nextTsn = initChunk->getInitTsn();
    state->lastTsn = initChunk->getInitTsn() + state->numRequests - 1;
    state->streamResetSequenceNumber = state->nextTsn;
    state->asconfSn = 1000;

    initTsn = initChunk->getInitTsn();
#ifdef WITH_IPv4
    initChunk->setIpv4Supported(true);
#else
    initChunk->setIpv4Supported(false);
#endif
#ifdef WITH_IPv6
    initChunk->setIpv6Supported(true);
#else
    initChunk->setIpv6Supported(false);
#endif
    if (localAddressList.front().isUnspecified()) {
        for (int32 i = 0; i < ift->getNumInterfaces(); ++i) {
#ifdef WITH_IPv4
            if (auto ipv4Data = ift->getInterface(i)->findProtocolData<Ipv4InterfaceData>()) {
                adv.push_back(ipv4Data->getIPAddress());
            }
            else
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
            if (auto ipv6Data = ift->getInterface(i)->findProtocolData<Ipv6InterfaceData>()) {
                for (int32 j = 0; j < ipv6Data->getNumAddresses(); j++) {
                    EV_DETAIL << "add address " << ipv6Data->getAddress(j) << "\n";
                    adv.push_back(ipv6Data->getAddress(j));
                }
            }
            else
#endif // ifdef WITH_IPv6
            ;
        }
    }
    else {
        adv = localAddressList;
        EV_DETAIL << "gebundene Adresse " << localAddr << " wird hinzugefuegt\n";    // todo
    }
    if (initChunk->getIpv4Supported() || initChunk->getIpv6Supported()) {
        length += 8;
    }
    uint32 addrNum = 0;
    bool friendly = false;
    if (sctpMain->hasPar("natFriendly")) {
        friendly = sctpMain->par("natFriendly");
    }
    if (remoteAddr.getType() == L3Address::IPv6) {
        for (auto & elem : adv) {
            if (!friendly) {
                initChunk->setAddressesArraySize(addrNum + 1);
                initChunk->setAddresses(addrNum++, (elem));
                length += 20;
            }
            sctpMain->addLocalAddress(this, (elem));
            state->localAddresses.push_back((elem));
            if (localAddr.isUnspecified())
                localAddr = (elem);
        }
    }
    else if (remoteAddr.getType() == L3Address::IPv4) {
        int rlevel = getAddressLevel(remoteAddr);
        EV_DETAIL << "level of remote address=" << rlevel << "\n";
        for (auto & elem : adv) {
            int addressLevel = getAddressLevel(elem);
            EV_DETAIL << "level of address " << (elem) << " = " << addressLevel << "\n";
            if (addressLevel >= rlevel) {
                initChunk->setAddressesArraySize(addrNum + 1);
                initChunk->setAddresses(addrNum++, (elem));
                length += 8;
                sctpMain->addLocalAddress(this, (elem));
                state->localAddresses.push_back((elem));
                if (localAddr.toIpv4().getInt() == 0)
                    localAddr = (elem);
            }
            else if (rlevel == 4 && addressLevel == 3 && friendly) {
                sctpMain->addLocalAddress(this, (elem));
                state->localAddresses.push_back((elem));
                if (localAddr.toIpv4().getInt() == 0)
                    localAddr = (elem);
            }
        }
    }
    else
        throw cRuntimeError("Unknown address type: %d", (int)(remoteAddr.getType()));

    uint16 count = 0;
    if (sctpMain->auth == true) {
        initChunk->setSepChunksArraySize(++count);
        initChunk->setSepChunks(count - 1, AUTH);
        state->keyVector[0] = (uint8_t)RANDOM;
        state->keyVector[2] = 36;
        for (int32 k = 0; k < 32; k++) {
            initChunk->setRandomArraySize(k + 1);
            initChunk->setRandom(k, (uint8)(RNGCONTEXT intrand(256)));
            state->keyVector[k + 2] = initChunk->getRandom(k);
        }
        state->sizeKeyVector = 36;
        state->keyVector[state->sizeKeyVector] = (uint8_t)CHUNKS;
        state->sizeKeyVector += 2;
        state->keyVector[state->sizeKeyVector] = state->chunkList.size() + 4;
        state->sizeKeyVector += 2;
        initChunk->setSctpChunkTypesArraySize(state->chunkList.size());
        int32 k = 0;
        for (auto & elem : state->chunkList) {
            initChunk->setSctpChunkTypes(k, (elem));
            state->keyVector[state->sizeKeyVector] = (elem);
            state->sizeKeyVector++;
            k++;
        }
        state->keyVector[state->sizeKeyVector] = (uint8_t)HMAC_ALGO;
        state->sizeKeyVector += 2;
        state->keyVector[state->sizeKeyVector] = 1 + 4;
        state->sizeKeyVector += 2;
        state->keyVector[state->sizeKeyVector] = 1;
        state->sizeKeyVector++;
        initChunk->setHmacTypesArraySize(1);
        initChunk->setHmacTypes(0, 1);
        length += initChunk->getSctpChunkTypesArraySize() + 50;
    }
    if (sctpMain->pktdrop) {
        initChunk->setSepChunksArraySize(++count);
        initChunk->setSepChunks(count - 1, PKTDROP);
    }
    if (state->streamReset == true) {
        initChunk->setSepChunksArraySize(++count);
        initChunk->setSepChunks(count - 1, RE_CONFIG);
    }
    if (sctpMain->par("addIP").boolValue()) {
        initChunk->setSepChunksArraySize(++count);
        initChunk->setSepChunks(count - 1, ASCONF);
        initChunk->setSepChunksArraySize(++count);
        initChunk->setSepChunks(count - 1, ASCONF_ACK);
    }
    if (count > 0) {
        length += ADD_PADDING(SCTP_SUPPORTED_EXTENSIONS_PARAMETER_LENGTH + count);
    }
    if (state->prMethod != 0) {
        initChunk->setForwardTsn(true);
        length += 4;
    }
    sctpMain->printInfoAssocMap();
    initChunk->setByteLength(length);
    sctpmsg->insertSctpChunks(initChunk);
    // set path variables
    if (remoteAddressList.size() > 0) {
        for (auto & elem : remoteAddressList) {
            EV_DEBUG << " get new path for " << (elem) << " at line " << __LINE__ << "\n";
            SctpPathVariables *path = new SctpPathVariables((elem), this, rt);
            sctpPathMap[(elem)] = path;
            qCounter.roomTransQ[(elem)] = 0;
            qCounter.bookedTransQ[(elem)] = 0;
            qCounter.roomRetransQ[(elem)] = 0;
        }
    }
    else {
        EV_DEBUG << " get new path for " << remoteAddr << " at line " << __LINE__ << "\n";
        SctpPathVariables *path = new SctpPathVariables(remoteAddr, this, rt);
        sctpPathMap[remoteAddr] = path;
        qCounter.roomTransQ[remoteAddr] = 0;
        qCounter.bookedTransQ[remoteAddr] = 0;
        qCounter.roomRetransQ[remoteAddr] = 0;
    }
    // send it
    state->initChunk = check_and_cast<SctpInitChunk *>(initChunk->dup());
    printSctpPathMap();
    EV_INFO << getFullPath() << " sendInit: localVTag=" << localVTag << " peerVTag=" << peerVTag << "\n";
    Packet *fp = new Packet("INIT");
    EV_INFO << "Length sctpmsg " << B(sctpmsg->getChunkLength()).get() << endl;
    sendToIP(fp, sctpmsg);
    sctpMain->assocList.push_back(this);
}

void SctpAssociation::retransmitInit()
{
    const auto& sctpmsg = makeShared<SctpHeader>();
    sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));
    SctpInitChunk *sctpinit;    // = new SctpInitChunk("INIT");

    EV_INFO << "Retransmit InitChunk=" << &sctpinit << "\n";

    sctpinit = check_and_cast<SctpInitChunk *>(state->initChunk->dup());
    sctpinit->setSctpChunkType(INIT);
    sctpmsg->insertSctpChunks(sctpinit);

    Packet *fp = new Packet("INIT RTX");
    sendToIP(fp, sctpmsg);
}

void SctpAssociation::sendInitAck(SctpInitChunk *initChunk)
{
    uint32 length = SCTP_INIT_CHUNK_LENGTH;

    state->setPrimaryPath(getPath(remoteAddr));
    // create segment
    const auto& sctpinitack = makeShared<SctpHeader>();
    sctpinitack->setChunkLength(B(SCTP_COMMON_HEADER));

    sctpinitack->setSrcPort(localPort);
    sctpinitack->setDestPort(remotePort);
    EV_INFO << "sendInitAck at " << localAddr << ". Provided InitTag=" << initChunk->getInitTag() << "\n";
    SctpInitAckChunk *initAckChunk = new SctpInitAckChunk();
    initAckChunk->setSctpChunkType(INIT_ACK);
    SctpCookie *cookie = new SctpCookie();
    cookie->setCreationTime(simTime());
    cookie->setLocalTieTagArraySize(32);
    cookie->setPeerTieTagArraySize(32);
    if (fsm->getState() == SCTP_S_CLOSED) {
        while (peerVTag == 0) {
            peerVTag = (uint32)RNGCONTEXT intrand(INT32_MAX);
        }
        initAckChunk->setInitTag(peerVTag);
        initAckChunk->setInitTsn(2000);
        state->nextTsn = initAckChunk->getInitTsn();
        state->lastTsn = initAckChunk->getInitTsn() + state->numRequests - 1;
        state->asconfSn = 2000;
        state->streamResetSequenceNumber = state->nextTsn;
        cookie->setLocalTag(localVTag);
        cookie->setPeerTag(peerVTag);
        for (int32 i = 0; i < 32; i++) {
            cookie->setLocalTieTag(i, 0);
            cookie->setPeerTieTag(i, 0);
        }
        sctpinitack->setVTag(localVTag);
        EV_INFO << "state=closed: localVTag=" << localVTag << " peerVTag=" << peerVTag << "\n";
    }
    else if (fsm->getState() == SCTP_S_COOKIE_WAIT || fsm->getState() == SCTP_S_COOKIE_ECHOED) {
        initAckChunk->setInitTag(peerVTag);
        EV_INFO << "different state:set InitTag in InitAck: " << initAckChunk->getInitTag() << "\n";
        initAckChunk->setInitTsn(state->nextTsn);
        initPeerTsn = initChunk->getInitTsn();
        state->gapList.forwardCumAckTsn(initPeerTsn - 1);
        cookie->setLocalTag(initChunk->getInitTag());
        cookie->setPeerTag(peerVTag);
        for (int32 i = 0; i < 32; i++) {
            cookie->setPeerTieTag(i, (uint8)(RNGCONTEXT intrand(256)));
            state->peerTieTag[i] = cookie->getPeerTieTag(i);
            if (fsm->getState() == SCTP_S_COOKIE_ECHOED) {
                cookie->setLocalTieTag(i, (uint8)(RNGCONTEXT intrand(256)));
                state->localTieTag[i] = cookie->getLocalTieTag(i);
            }
            else
                cookie->setLocalTieTag(i, 0);
        }
        sctpinitack->setVTag(initChunk->getInitTag());
        EV_DETAIL << "VTag in InitAck: " << sctpinitack->getVTag() << "\n";
    }
    else {
        EV_INFO << "other state\n";
        uint32 tag = 0;
        while (tag == 0) {
            tag = (uint32)(fmod(RNGCONTEXT intrand(INT32_MAX), 1.0 + (double)(unsigned)0xffffffffUL)) & 0xffffffffUL;
        }
        initAckChunk->setInitTag(tag);
        initAckChunk->setInitTsn(state->nextTsn);
        cookie->setLocalTag(localVTag);
        cookie->setPeerTag(peerVTag);
        for (int32 i = 0; i < 32; i++) {
            cookie->setPeerTieTag(i, state->peerTieTag[i]);
            cookie->setLocalTieTag(i, state->localTieTag[i]);
        }
        sctpinitack->setVTag(initChunk->getInitTag());
    }
    cookie->setLength(SCTP_COOKIE_LENGTH + 4);
    initAckChunk->setStateCookie(cookie);
    initAckChunk->setCookieArraySize(0);
    initAckChunk->setA_rwnd(sctpMain->par("arwnd"));
    state->localRwnd = sctpMain->par("arwnd");
    initAckChunk->setMsg_rwnd(sctpMain->par("messageAcceptLimit"));
    initAckChunk->setNoOutStreams((unsigned int)min(outboundStreams, initChunk->getNoInStreams()));
    initAckChunk->setNoInStreams((unsigned int)min(inboundStreams, initChunk->getNoOutStreams()));
    initTsn = initAckChunk->getInitTsn();
#ifdef WITH_IPv4
    initAckChunk->setIpv4Supported(true);
#else
    initAckChunk->setIpv4Supported(false);
#endif
#ifdef WITH_IPv6
    initAckChunk->setIpv6Supported(true);
#else
    initAckChunk->setIpv6Supported(false);
#endif
    if (initAckChunk->getIpv4Supported() || initAckChunk->getIpv6Supported()) {
        length += 8;
    }
    uint32 addrNum = 0;
    bool friendly = false;
    if (sctpMain->hasPar("natFriendly")) {
        friendly = sctpMain->par("natFriendly");
    }
    if (!friendly)
        for (auto & elem : state->localAddresses) {
            initAckChunk->setAddressesArraySize(addrNum + 1);
            initAckChunk->setAddresses(addrNum++, (elem));
            if ((elem).getType() == L3Address::IPv4) {
                length += 8;
            } else if ((elem).getType() == L3Address::IPv6) {
                length += 20;
            }
        }

    uint16 count = 0;
    if (sctpMain->auth == true) {
        initAckChunk->setSepChunksArraySize(++count);
        initAckChunk->setSepChunks(count - 1, AUTH);
        for (int32 k = 0; k < 32; k++) {
            initAckChunk->setRandomArraySize(k + 1);
            initAckChunk->setRandom(k, (uint8)(RNGCONTEXT intrand(256)));
        }
        initAckChunk->setSctpChunkTypesArraySize(state->chunkList.size());
        int32 k = 0;
        for (auto & elem : state->chunkList) {
            initAckChunk->setSctpChunkTypes(k, (elem));
            k++;
        }
        initAckChunk->setHmacTypesArraySize(1);
        initAckChunk->setHmacTypes(0, 1);
        length += ADD_PADDING(initAckChunk->getSctpChunkTypesArraySize() + 48);
    }
    uint32 unknownLen = initChunk->getUnrecognizedParametersArraySize();
    if (unknownLen > 0) {
        EV_INFO << "Found unrecognized Parameters in INIT chunk with a length of " << unknownLen << " bytes.\n";
        initAckChunk->setUnrecognizedParametersArraySize(unknownLen);
        for (uint32 i = 0; i < unknownLen; i++)
            initAckChunk->setUnrecognizedParameters(i, initChunk->getUnrecognizedParameters(i));
        length += unknownLen;
    }
    else
        initAckChunk->setUnrecognizedParametersArraySize(0);

    if (sctpMain->pktdrop) {
        initAckChunk->setSepChunksArraySize(++count);
        initAckChunk->setSepChunks(count - 1, PKTDROP);
    }

    if (state->streamReset == true) {
        initAckChunk->setSepChunksArraySize(++count);
        initAckChunk->setSepChunks(count - 1, RE_CONFIG);
    }
    if (sctpMain->par("addIP").boolValue()) {
        initAckChunk->setSepChunksArraySize(++count);
        initAckChunk->setSepChunks(count - 1, ASCONF);
        initAckChunk->setSepChunksArraySize(++count);
        initAckChunk->setSepChunks(count - 1, ASCONF_ACK);
    }
    if (count > 0) {
        length += ADD_PADDING(SCTP_SUPPORTED_EXTENSIONS_PARAMETER_LENGTH + count);
    }
    if (state->prMethod != 0) {
        initAckChunk->setForwardTsn(true);
        length += 4;
    }
    initAckChunk->setByteLength(length + initAckChunk->getCookieArraySize() + cookie->getLength());
    inboundStreams = ((initChunk->getNoOutStreams() < initAckChunk->getNoInStreams()) ? initChunk->getNoOutStreams() : initAckChunk->getNoInStreams());
    outboundStreams = ((initChunk->getNoInStreams() < initAckChunk->getNoOutStreams()) ? initChunk->getNoInStreams() : initAckChunk->getNoOutStreams());
    (this->*ssFunctions.ssInitStreams)(inboundStreams, outboundStreams);
    sctpinitack->insertSctpChunks(initAckChunk);
    Packet *fp = new Packet("INIT-ACK");
    if (fsm->getState() == SCTP_S_CLOSED) {
        sendToIP(fp, sctpinitack, state->initialPrimaryPath);
    }
    else {
        sendToIP(fp, sctpinitack);
    }
    sctpMain->assocList.push_back(this);
    printSctpPathMap();
}

void SctpAssociation::sendCookieEcho(SctpInitAckChunk *initAckChunk)
{
    SctpAuthenticationChunk *authChunk;
    const auto& sctpcookieecho = makeShared<SctpHeader>();
    sctpcookieecho->setChunkLength(B(SCTP_COMMON_HEADER));

    EV_INFO << "SctpAssociationUtil:sendCookieEcho\n";

    sctpcookieecho->setSrcPort(localPort);
    sctpcookieecho->setDestPort(remotePort);
    SctpCookieEchoChunk *cookieEchoChunk = new SctpCookieEchoChunk();
    cookieEchoChunk->setSctpChunkType(COOKIE_ECHO);
    int32 len = initAckChunk->getCookieArraySize();
    cookieEchoChunk->setCookieArraySize(len);
    if (len > 0) {
        for (int32 i = 0; i < len; i++)
            cookieEchoChunk->setCookie(i, initAckChunk->getCookie(i));
        cookieEchoChunk->setByteLength(SCTP_COOKIE_ACK_LENGTH + len);
    }
    else {
        SctpCookie *cookie = (SctpCookie *)(initAckChunk->getStateCookie());
        cookieEchoChunk->setStateCookie(cookie);
        cookieEchoChunk->setByteLength(SCTP_COOKIE_ACK_LENGTH + cookie->getLength());
    }
    uint32 unknownLen = initAckChunk->getUnrecognizedParametersArraySize();
    if (unknownLen > 0) {
        EV_INFO << "Found unrecognized Parameters in INIT-ACK chunk with a length of " << unknownLen << " bytes.\n";
        cookieEchoChunk->setUnrecognizedParametersArraySize(unknownLen);
        for (uint32 i = 0; i < unknownLen; i++)
            cookieEchoChunk->setUnrecognizedParameters(i, initAckChunk->getUnrecognizedParameters(i));
    }
    else
        cookieEchoChunk->setUnrecognizedParametersArraySize(0);
    state->cookieChunk = check_and_cast<SctpCookieEchoChunk *>(cookieEchoChunk->dup());
    if (len == 0) {
        state->cookieChunk->setStateCookie(initAckChunk->getStateCookie()->dup());
    }

    if (state->auth && state->peerAuth && typeInChunkList(COOKIE_ECHO)) {
        authChunk = createAuthChunk();
        sctpcookieecho->insertSctpChunks(authChunk);
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }

    sctpcookieecho->insertSctpChunks(cookieEchoChunk);
    Packet *fp = new Packet("COOKIE-ECHO");
    sendToIP(fp, sctpcookieecho);
}

void SctpAssociation::retransmitCookieEcho()
{
    SctpAuthenticationChunk *authChunk;
    const auto& sctpmsg = makeShared<SctpHeader>();
    sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));
    SctpCookieEchoChunk *cookieEchoChunk = check_and_cast<SctpCookieEchoChunk *>(state->cookieChunk->dup());
    if (cookieEchoChunk->getCookieArraySize() == 0) {
        cookieEchoChunk->setStateCookie(state->cookieChunk->getStateCookie()->dup());
    }
    if (state->auth && state->peerAuth && typeInChunkList(COOKIE_ECHO)) {
        authChunk = createAuthChunk();
        sctpmsg->insertSctpChunks(authChunk);
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpmsg->insertSctpChunks(cookieEchoChunk);

    EV_INFO << "retransmitCookieEcho localAddr=" << localAddr << "     remoteAddr" << remoteAddr << "\n";
    Packet *fp = new Packet("COOKIE-ECHO RTX");
    sendToIP(fp, sctpmsg);
}

void SctpAssociation::sendHeartbeat(const SctpPathVariables *path)
{
    SctpAuthenticationChunk *authChunk;
    const auto& sctpHeartbeatbeat = makeShared<SctpHeader>();
    sctpHeartbeatbeat->setChunkLength(B(SCTP_COMMON_HEADER));

    sctpHeartbeatbeat->setSrcPort(localPort);
    sctpHeartbeatbeat->setDestPort(remotePort);
    SctpHeartbeatChunk *heartbeatChunk = new SctpHeartbeatChunk();
    heartbeatChunk->setSctpChunkType(HEARTBEAT);
    heartbeatChunk->setRemoteAddr(path->remoteAddress);
    heartbeatChunk->setTimeField(simTime());
    heartbeatChunk->setByteLength(SCTP_HEARTBEAT_CHUNK_LENGTH + 12);
    if (state->auth && state->peerAuth && typeInChunkList(HEARTBEAT)) {
        authChunk = createAuthChunk();
        sctpHeartbeatbeat->insertSctpChunks(authChunk);
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpHeartbeatbeat->insertSctpChunks(heartbeatChunk);
    EV_INFO << "sendHeartbeat: sendToIP to " << path->remoteAddress << endl;
    Packet *fp = new Packet("HEARTBEAT");
    sendToIP(fp, sctpHeartbeatbeat, path->remoteAddress);
}

void SctpAssociation::sendHeartbeatAck(const SctpHeartbeatChunk *heartbeatChunk,
        const L3Address& src,
        const L3Address& dest)
{
    SctpAuthenticationChunk *authChunk;
    const auto& sctpHeartbeatAck = makeShared<SctpHeader>();
    sctpHeartbeatAck->setChunkLength(B(SCTP_COMMON_HEADER));
    sctpHeartbeatAck->setSrcPort(localPort);
    sctpHeartbeatAck->setDestPort(remotePort);
    SctpHeartbeatAckChunk *heartbeatAckChunk = new SctpHeartbeatAckChunk();
    heartbeatAckChunk->setSctpChunkType(HEARTBEAT_ACK);
    heartbeatAckChunk->setRemoteAddr(heartbeatChunk->getRemoteAddr());
    heartbeatAckChunk->setTimeField(heartbeatChunk->getTimeField());
    const int32 len = heartbeatChunk->getInfoArraySize();
    if (len > 0) {
        heartbeatAckChunk->setInfoArraySize(len);
        for (int32 i = 0; i < len; i++)
            heartbeatAckChunk->setInfo(i, heartbeatChunk->getInfo(i));
    }

    heartbeatAckChunk->setByteLength(heartbeatChunk->getByteLength());
    if (state->auth && state->peerAuth && typeInChunkList(HEARTBEAT_ACK)) {
        authChunk = createAuthChunk();
        sctpHeartbeatAck->insertSctpChunks(authChunk);
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpHeartbeatAck->insertSctpChunks(heartbeatAckChunk);

    EV_INFO << "sendHeartbeatAck: sendToIP from " << src << " to " << dest << endl;
    Packet *fp = new Packet("HEARTBEAT-ACK");
    sendToIP(fp, sctpHeartbeatAck, dest);
}

void SctpAssociation::sendCookieAck(const L3Address& dest)
{
    SctpAuthenticationChunk *authChunk;
    const auto& sctpcookieack = makeShared<SctpHeader>();
    sctpcookieack->setChunkLength(B(SCTP_COMMON_HEADER));

    EV_INFO << "SctpAssociationUtil:sendCookieACK\n";

    sctpcookieack->setSrcPort(localPort);
    sctpcookieack->setDestPort(remotePort);
    SctpCookieAckChunk *cookieAckChunk = new SctpCookieAckChunk();
    cookieAckChunk->setSctpChunkType(COOKIE_ACK);
    cookieAckChunk->setByteLength(SCTP_COOKIE_ACK_LENGTH);
    if (state->auth && state->peerAuth && typeInChunkList(COOKIE_ACK)) {
        authChunk = createAuthChunk();
        sctpcookieack->insertSctpChunks(authChunk);
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpcookieack->insertSctpChunks(cookieAckChunk);
    Packet *fp = new Packet("COOKIE-ACK");
    sendToIP(fp, sctpcookieack, dest);
}

void SctpAssociation::sendShutdownAck(const L3Address& dest)
{
    sendOnAllPaths(getPath(dest));
    if (getOutstandingBytes() == 0) {
        performStateTransition(SCTP_E_NO_MORE_OUTSTANDING);
        const auto& sctpshutdownack = makeShared<SctpHeader>();
        sctpshutdownack->setChunkLength(B(SCTP_COMMON_HEADER));

        EV_INFO << "SctpAssociationUtil:sendShutdownACK" << endl;

        sctpshutdownack->setSrcPort(localPort);
        sctpshutdownack->setDestPort(remotePort);
        SctpShutdownAckChunk *shutdownAckChunk = new SctpShutdownAckChunk();
        shutdownAckChunk->setSctpChunkType(SHUTDOWN_ACK);
        shutdownAckChunk->setByteLength(SCTP_COOKIE_ACK_LENGTH);
        sctpshutdownack->insertSctpChunks(shutdownAckChunk);
        state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT;
        state->initRetransCounter = 0;
        stopTimer(T2_ShutdownTimer);
        startTimer(T2_ShutdownTimer, state->initRexmitTimeout);
        stopTimer(T5_ShutdownGuardTimer);
        startTimer(T5_ShutdownGuardTimer, SHUTDOWN_GUARD_TIMEOUT);
        state->shutdownAckChunk = check_and_cast<SctpShutdownAckChunk *>(shutdownAckChunk->dup());
        Packet *fp = new Packet("SHUTDOWN-ACK");
        sendToIP(fp, sctpshutdownack, dest);
    }
}

void SctpAssociation::sendShutdownComplete()
{
    const auto& sctpshutdowncomplete = makeShared<SctpHeader>();
    sctpshutdowncomplete->setChunkLength(B(SCTP_COMMON_HEADER));

    EV_INFO << "SctpAssociationUtil:sendShutdownComplete\n";

    sctpshutdowncomplete->setSrcPort(localPort);
    sctpshutdowncomplete->setDestPort(remotePort);
    SctpShutdownCompleteChunk *shutdownCompleteChunk = new SctpShutdownCompleteChunk();
    shutdownCompleteChunk->setSctpChunkType(SHUTDOWN_COMPLETE);
    shutdownCompleteChunk->setTBit(0);
    shutdownCompleteChunk->setByteLength(SCTP_SHUTDOWN_ACK_LENGTH);
    sctpshutdowncomplete->insertSctpChunks(shutdownCompleteChunk);
    Packet *fp = new Packet("SHUTDOWN-COMPLETE");
    sendToIP(fp, sctpshutdowncomplete);
}

void SctpAssociation::sendAbort(uint16 tBit)
{
    SctpAuthenticationChunk *authChunk;
    const auto& msg = makeShared<SctpHeader>();
    msg->setChunkLength(B(SCTP_COMMON_HEADER));

    EV_INFO << "SctpAssociationUtil:sendABORT localPort=" << localPort << "    remotePort=" << remotePort << "\n";

    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SctpAbortChunk *abortChunk = new SctpAbortChunk();
    abortChunk->setSctpChunkType(ABORT);
    abortChunk->setT_Bit(tBit);
    abortChunk->setByteLength(SCTP_ABORT_CHUNK_LENGTH);
    if (state->auth && state->peerAuth && typeInChunkList(ABORT)) {
        authChunk = createAuthChunk();
        msg->insertSctpChunks(authChunk);
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    msg->insertSctpChunks(abortChunk);
    if (state->resetChunk != nullptr) {
        delete state->resetChunk;
    }
    Packet *fp = new Packet("ABORT");
    sendToIP(fp, msg, remoteAddr);
}

void SctpAssociation::sendShutdown()
{
    SctpAuthenticationChunk *authChunk;
    const auto& msg = makeShared<SctpHeader>();
    msg->setChunkLength(B(SCTP_COMMON_HEADER));

    EV_INFO << "SctpAssociationUtil:sendShutdown localPort=" << localPort << "     remotePort=" << remotePort << "\n";

    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SctpShutdownChunk *shutdownChunk = new SctpShutdownChunk();
    shutdownChunk->setSctpChunkType(SHUTDOWN);
    //shutdownChunk->setCumTsnAck(state->lastTsnAck);
    shutdownChunk->setCumTsnAck(state->gapList.getCumAckTsn());
    shutdownChunk->setByteLength(SCTP_SHUTDOWN_CHUNK_LENGTH);
    if (state->auth && state->peerAuth && typeInChunkList(SHUTDOWN)) {
        authChunk = createAuthChunk();
        msg->insertSctpChunks(authChunk);
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT;
    state->initRetransCounter = 0;
    stopTimer(T5_ShutdownGuardTimer);
    startTimer(T5_ShutdownGuardTimer, SHUTDOWN_GUARD_TIMEOUT);
    stopTimer(T2_ShutdownTimer);
    startTimer(T2_ShutdownTimer, state->initRexmitTimeout);
    state->shutdownChunk = check_and_cast<SctpShutdownChunk *>(shutdownChunk->dup());
    msg->insertSctpChunks(shutdownChunk);
    Packet *fp = new Packet("SHUTDOWN");
    sendToIP(fp, msg, remoteAddr);
    performStateTransition(SCTP_E_NO_MORE_OUTSTANDING);
}

void SctpAssociation::retransmitShutdown()
{
    const auto& sctpmsg = makeShared<SctpHeader>();
    sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));
    SctpShutdownChunk *shutdownChunk;
    shutdownChunk = check_and_cast<SctpShutdownChunk *>(state->shutdownChunk->dup());
    sctpmsg->insertSctpChunks(shutdownChunk);

    EV_INFO << "retransmitShutdown localAddr=" << localAddr << "  remoteAddr" << remoteAddr << "\n";

    Packet *fp = new Packet("SHUTDOWN RTX");
    sendToIP(fp, sctpmsg);
}

void SctpAssociation::retransmitShutdownAck()
{
    const auto& sctpmsg = makeShared<SctpHeader>();
    sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));
    SctpShutdownAckChunk *shutdownAckChunk;
    shutdownAckChunk = check_and_cast<SctpShutdownAckChunk *>(state->shutdownAckChunk->dup());
    sctpmsg->insertSctpChunks(shutdownAckChunk);

    EV_INFO << "retransmitShutdownAck localAddr=" << localAddr << "  remoteAddr" << remoteAddr << "\n";

    Packet *fp = new Packet("SHUTDOWN-ACK RTX");
    sendToIP(fp, sctpmsg);
}

void SctpAssociation::sendPacketDrop(const bool flag)
{
#if 0
    EV_INFO << "sendPacketDrop:\t";
    SctpHeader *drop = (SctpHeader *)state->sctpmsg->dup();        //FIXME is the c-style conversion need here?
    if (drop->getSctpChunksArraySize() == 1) {
        SctpChunk *header = (SctpChunk *)(drop->getSctpChunks(0));
        if (header->getSctpChunkType() == PKTDROP) {
          //  disposeOf(state->sctpmsg);
            delete drop;
            return;
        }
    }
    const auto& sctpmsg = makeShared<SctpHeader>();
    sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));
    SctpPacketDropChunk *pktdrop = new SctpPacketDropChunk();
    pktdrop->setSctpChunkType(PKTDROP);
    pktdrop->setCFlag(false);
    pktdrop->setTFlag(false);
    pktdrop->setBFlag(flag);
    pktdrop->setMFlag(false);
    pktdrop->setMaxRwnd(sctpMain->par("arwnd"));
    pktdrop->setQueuedData(state->queuedReceivedBytes);
    pktdrop->setTruncLength(0);
    pktdrop->setByteLength(SCTP_PKTDROP_CHUNK_LENGTH);
    uint16 mss = getPath(remoteAddr)->pmtu - SCTP_COMMON_HEADER - SCTP_DATA_CHUNK_LENGTH - IP_HEADER_LENGTH;
    if (B(drop->getChunkLength()).get() > mss) {
        uint16 diff = B(drop->getChunkLength()).get() - mss;
        pktdrop->setTruncLength(B(drop->getChunkLength()).get());
        SctpChunk *sctpchunk = (SctpChunk *)(drop->removeLastChunk());
        if (sctpchunk->getSctpChunkType() == DATA) {
            SctpDataChunk *dataChunk = check_and_cast<SctpDataChunk *>(sctpchunk);
          /*  auto& smsg = sctpchunk->Chunk::peek<SctpSimpleMessage>(Chunk::BackwardIterator(B(0)));*/

            SctpSimpleMessage *smsg = check_and_cast<SctpSimpleMessage *>(dataChunk->decapsulate());


            if (smsg->getDataLen() > diff) {
                uint16 newLength = smsg->getDataLen() - diff;
                smsg->setDataArraySize(newLength);
                for (uint16 i = 0; i < newLength; i++)
                    smsg->setData(i, 'a');
                smsg->setDataLen(newLength);
                smsg->setEncaps(false);
                smsg->setByteLength(newLength);
                dataChunk->encapsulate((cPacket *)smsg);
              //  dataChunk->insertAtBack(smsg);
                drop->insertSctpChunks(dataChunk);
            }
            else if (drop->getSctpChunksArraySize() == 1) {
                delete sctpchunk;
                delete pktdrop;
               // disposeOf(state->sctpmsg);
                EV_DETAIL << "laenge=" << B(drop->getChunkLength()).get() << " numberOfChunks=1\n";
                disposeOf(drop);
                return;
            }
        }
        else {
            delete pktdrop;
           // disposeOf(state->sctpmsg);
            EV_DETAIL << "laenge=" << B(drop->getChunkLength()).get() << " numberOfChunks=1\n";
            disposeOf(drop);
            return;
        }
        pktdrop->setTFlag(true);
    }
/**** ToDo Irene */
   // pktdrop->insertAtBack(drop);
   // pktdrop->encapsulate(drop);

    EV_DETAIL << "length of PKTDROP chunk=" << pktdrop->getByteLength() << "\n";
    sctpmsg->insertSctpChunks(pktdrop);
    EV_DETAIL << "total length now " << B(sctpmsg->getChunkLength()).get() << "\n";
   // disposeOf(state->sctpmsg);
    state->pktDropSent = true;
    sctpMain->numPktDropReports++;
    Packet *fp = new Packet("PKTDROP");
    sendToIP(fp, sctpmsg);
#endif
}

void SctpAssociation::scheduleSack()
{
    /* increase SACK counter, we received another data PACKET */
    if (state->firstChunkReceived)
        state->ackState++;
    else {
        state->ackState = sackFrequency;
        state->firstChunkReceived = true;
    }

    EV_DETAIL << "scheduleSack() : ackState is now: " << state->ackState << "\n";

    if (state->ackState <= sackFrequency - 1) {
        /* start a SACK timer if none is running, to expire 200 ms (or parameter) from now */
        if (!SackTimer->isScheduled()) {
            startTimer(SackTimer, sackPeriod);
        }
        /* else: leave timer running, and do nothing... */ else {
            /* is this possible at all ? Check this... */

            EV_DETAIL << "SACK timer running, but scheduleSack() called\n";
        }
    }
}

SctpForwardTsnChunk *SctpAssociation::createForwardTsnChunk(const L3Address& pid)
{
    uint16 chunkLength = SCTP_FORWARD_TSN_CHUNK_LENGTH;
    SctpDataVariables *chunk;
    typedef std::map<uint16, int16> SidMap;
    SidMap sidMap;

    EV_INFO << "Create forwardTsnChunk for " << pid << "\n";
    SctpForwardTsnChunk *forwChunk = new SctpForwardTsnChunk();
    forwChunk->setSctpChunkType(FORWARD_TSN);
    advancePeerTsn();
    forwChunk->setNewCumTsn(state->advancedPeerAckPoint);
    for (auto & elem : retransmissionQ->payloadQueue) {
        chunk = elem.second;
        EV_DETAIL << "tsn=" << chunk->tsn << " lastDestination=" << chunk->getLastDestination() << " abandoned=" << chunk->hasBeenAbandoned << "\n";
        if (chunk->getLastDestination() == pid && chunk->hasBeenAbandoned && chunk->tsn <= forwChunk->getNewCumTsn()) {
            if (chunk->ordered) {
                sidMap[chunk->sid] = chunk->ssn;
            }
            else {
                sidMap[chunk->sid] = -1;
            }
            /* Fake chunk retransmission */
            if (chunk->sendForwardIfAbandoned) {
                chunk->gapReports = 0;
                chunk->hasBeenFastRetransmitted = false;
                chunk->sendTime = simTime();
                chunk->numberOfRetransmissions++;
                chunk->sendForwardIfAbandoned = false;

                auto itt = transmissionQ->payloadQueue.find(chunk->tsn);
                if (itt != transmissionQ->payloadQueue.end()) {
                    transmissionQ->payloadQueue.erase(itt);
                    chunk->enqueuedInTransmissionQ = false;
                    auto i = qCounter.roomTransQ.find(pid);
                    i->second -= ADD_PADDING(chunk->len / 8 + SCTP_DATA_CHUNK_LENGTH);
                    auto ib = qCounter.bookedTransQ.find(pid);
                    ib->second -= chunk->booksize;
                }
            }
        }
    }
    forwChunk->setSidArraySize(sidMap.size());
    forwChunk->setSsnArraySize(sidMap.size());
    int32 i = 0;
    for (auto & elem : sidMap) {
        forwChunk->setSid(i, elem.first);
        forwChunk->setSsn(i, elem.second);
        chunkLength += 4;
        i++;
    }
    forwChunk->setByteLength(chunkLength);
    auto iter = sctpMain->assocStatMap.find(assocId);
    iter->second.numForwardTsn++;
    return forwChunk;
}

inline static bool writeCompressedValue(uint8_t *outputBuffer,
        const size_t outputBufferSize,
        size_t& pos,
        const uint16_t value)
{
    if (value < 0x80) {    // 1 byte: 0vvvvvvv
        if (pos + 1 > outputBufferSize) {
            return false;
        }
        outputBuffer[pos] = (uint8_t)value;
        pos += 1;
    }
    else if (value < 0x4000) {    // 2 bytes: 10vvvvvv vvvvvvvv
        if (pos + 2 > outputBufferSize) {
            return false;
        }
        outputBuffer[pos + 0] = 0x80 | (uint8_t)(value >> 8);
        outputBuffer[pos + 1] = (uint8_t)(value & 0xff);
        pos += 2;
    }
    else {    // 3 bytes: 11xxxxxx vvvvvvvv vvvvvvvv
        if (pos + 3 > outputBufferSize) {
            return false;
        }
        outputBuffer[pos + 0] = 0xc0;
        outputBuffer[pos + 1] = (uint8_t)(value >> 8);
        outputBuffer[pos + 2] = (uint8_t)(value & 0xff);
        pos += 3;
    }
    return true;
}

static uint32 compressGaps(const SctpGapList *gapList, const SctpGapList::GapType type, size_t& space)
{
    uint8 compressedDataBuffer[1 + 6 * gapList->getNumGaps(type)];    // Worst-case size

    size_t outputPos = 0;
    unsigned int entriesWritten = 0;
    uint32 last = gapList->getCumAckTsn();
    for (unsigned int i = 0; i < gapList->getNumGaps(type); i++) {
        assert(SctpAssociation::tsnGt(gapList->getGapStart(type, i), last + 1));
        assert(SctpAssociation::tsnGe(gapList->getGapStop(type, i), gapList->getGapStart(type, i)));
        const uint16 startOffset = gapList->getGapStart(type, i) - last;
        const uint16 stopOffset = gapList->getGapStop(type, i) - gapList->getGapStart(type, i);
        const size_t lastOutputPos = outputPos;
        if ((writeCompressedValue((uint8 *)&compressedDataBuffer, sizeof(compressedDataBuffer), outputPos, startOffset) == false) ||
            (writeCompressedValue((uint8 *)&compressedDataBuffer, sizeof(compressedDataBuffer), outputPos, stopOffset) == false) ||
            (outputPos + 1 > space))
        {
            outputPos = lastOutputPos;
            break;
        }
        entriesWritten++;
        last = gapList->getGapStop(type, i);
    }
    assert(writeCompressedValue((uint8 *)&compressedDataBuffer, sizeof(compressedDataBuffer), outputPos, 0x00) == true);
    space = outputPos;

    return entriesWritten;
}

static uint32 copyToRGaps(SctpSackChunk *sackChunk,
        const SctpGapList *gapList,
        const SctpGapList::GapType type,
        const bool compression,
        size_t& space)
{
    const uint32 count = gapList->getNumGaps(type);
    uint32 last = gapList->getCumAckTsn();
    uint32 keys = min((uint32)(space / 4), count);    // Each entry occupies 2+2 bytes => at most space/4 entries
    if (compression) {
        keys = count;    // Get all entries first, compress them later
    }
    sackChunk->setGapStartArraySize(keys);
    sackChunk->setGapStopArraySize(keys);
    sackChunk->setNumGaps(keys);

    for (uint32 key = 0; key < keys; key++) {
        // ====== Validity check ========================================
        assert(SctpAssociation::tsnGt(gapList->getGapStart(type, key), last + 1));
        assert(SctpAssociation::tsnGe(gapList->getGapStop(type, key), gapList->getGapStart(type, key)));
        sackChunk->setGapStart(key, gapList->getGapStart(type, key));
        sackChunk->setGapStop(key, gapList->getGapStop(type, key));
        last = gapList->getGapStop(type, key);
    }
    if (compression) {
        keys = compressGaps(gapList, type, space);
        sackChunk->setNrGapStartArraySize(keys);
        sackChunk->setNrGapStopArraySize(keys);
    }
    else
        space = 4 * keys;

    return keys;
}

static uint32 copyToNRGaps(SctpSackChunk *sackChunk,
        const SctpGapList *gapList,
        const SctpGapList::GapType type,
        const bool compression,
        size_t& space)
{
    const uint32 count = gapList->getNumGaps(type);
    uint32 last = gapList->getCumAckTsn();
    uint32 keys = min((uint32)(space / 4), count);    // Each entry occupies 2+2 bytes => at most space/4 entries
    if (compression) {
        keys = count;    // Get all entries first, compress them later
    }
    sackChunk->setNrGapStartArraySize(keys);
    sackChunk->setNrGapStopArraySize(keys);
    sackChunk->setNumNrGaps(keys);

    for (uint32 key = 0; key < keys; key++) {
        // ====== Validity check ========================================
        assert(SctpAssociation::tsnGt(gapList->getGapStart(type, key), last + 1));
        assert(SctpAssociation::tsnGe(gapList->getGapStop(type, key), gapList->getGapStart(type, key)));
        sackChunk->setNrGapStart(key, gapList->getGapStart(type, key));
        sackChunk->setNrGapStop(key, gapList->getGapStop(type, key));
        last = gapList->getGapStop(type, key);
    }
    if (compression) {
        keys = compressGaps(gapList, type, space);
        sackChunk->setNrGapStartArraySize(keys);
        sackChunk->setNrGapStopArraySize(keys);
    }
    else
        space = 4 * keys;

    return keys;
}

SctpSackChunk *SctpAssociation::createSack()
{
    EV_INFO << simTime() << "SctpAssociationUtil:createSACK localAddress=" << localAddr << "  remoteAddress=" << remoteAddr << "\n";

    EV_INFO << " localRwnd=" << state->localRwnd << " queuedBytes=" << state->queuedReceivedBytes << "\n";

    // ====== Get receiver window size to be advertised ======================
    uint32 arwnd = 0;
    uint32 msgRwnd = 0;
    calculateRcvBuffer();
    if ((state->messageAcceptLimit > 0 && (int32)(state->localMsgRwnd - state->bufferedMessages) <= 0)
        || (state->messageAcceptLimit == 0 && (int32)(state->localRwnd - state->queuedReceivedBytes - state->bufferedMessages * state->bytesToAddPerRcvdChunk) <= 0))
    {
        msgRwnd = 0;
    }
    else if ((state->messageAcceptLimit > 0 && (int32)(state->localMsgRwnd - state->bufferedMessages) < 3)
             || (state->messageAcceptLimit == 0 && state->localRwnd - state->queuedReceivedBytes - state->bufferedMessages * state->bytesToAddPerRcvdChunk < state->swsLimit) || state->swsMsgInvoked == true)
    {
        msgRwnd = 1;
        state->swsMsgInvoked = true;
    }
    else {
        if (state->messageAcceptLimit > 0) {
            msgRwnd = state->localMsgRwnd - state->bufferedMessages;
        }
        else {
            msgRwnd = state->localRwnd
                - state->queuedReceivedBytes
                - state->bufferedMessages * state->bytesToAddPerRcvdChunk;
        }
    }
    if (state->tellArwnd) {
        arwnd = msgRwnd;
    }
    else {
        // ====== Receiver buffer is full =====================================
        if ((int32)(state->localRwnd - state->queuedReceivedBytes) <= 0) {
            arwnd = 0;
            if (state->swsLimit > 0) {
                state->swsAvoidanceInvoked = true;
            }
        }
        // ====== Silly window syndrome avoidance =============================
        else if ((state->localRwnd - state->queuedReceivedBytes < state->swsLimit) ||
                 (state->swsAvoidanceInvoked == true))
        {
            arwnd = 1;
            if (state->swsLimit > 0)
                state->swsAvoidanceInvoked = true;
            EV_DETAIL << "arwnd=1; createSack : SWS Avoidance ACTIVE !!!\n";
        }
        // ====== There is space in the receiver buffer =======================
        else {
            arwnd = state->localRwnd - state->queuedReceivedBytes;
            EV_DETAIL << simTime() << " arwnd = " << state->localRwnd << " - " << state->queuedReceivedBytes << " = " << arwnd << "\n";
        }
    }

    // ====== Record statistics ==============================================
    if (state->messageAcceptLimit > 0) {
        advMsgRwnd->record(msgRwnd);
    }
    statisticsQueuedReceivedBytes->record(state->queuedReceivedBytes);
    advRwnd->record(arwnd);

    // ====== Create SACK chunk ==============================================
    SctpSackChunk *sackChunk = new SctpSackChunk();
    if (state->nrSack == true) {
        sackChunk->setSctpChunkType(NR_SACK);
       // sackChunk->setName("NR_SACK");
    }
    else {
        sackChunk->setSctpChunkType(SACK);
    }
    sackChunk->setCumTsnAck(state->gapList.getCumAckTsn());
    EV_DEBUG << "SACK: set cumTsnAck to " << sackChunk->getCumTsnAck() << endl;
    sackChunk->setA_rwnd(arwnd);
    sackChunk->setIsNrSack(state->nrSack);
    sackChunk->setSackSeqNum(++state->outgoingSackSeqNum);
    if (state->messageAcceptLimit > 0) {
        sackChunk->setMsg_rwnd(state->messageAcceptLimit - state->bufferedMessages);
    }
    else {
        sackChunk->setMsg_rwnd(0);
    }

    // ====== What has to be stored in the SACK? =============================
    const uint32 mtu = getPath(remoteAddr)->pmtu;

    uint32 hdrSize;
    if (remoteAddr.getType() == L3Address::IPv6)
        hdrSize = 40;
    else if (remoteAddr.getType() == L3Address::IPv4)
        hdrSize = 20;
    else
        throw cRuntimeError("Unknown address type");

    const uint32 allowedLength = mtu
        - hdrSize
        - SCTP_COMMON_HEADER
        - SCTP_SACK_CHUNK_LENGTH;
    uint32 numDups = state->dupList.size();
    uint32 numRevokableGaps = state->gapList.getNumGaps(SctpGapList::GT_Revokable);
    uint32 numNonRevokableGaps = state->gapList.getNumGaps(SctpGapList::GT_NonRevokable);
    size_t revokableGapsSpace = ~0;
    size_t nonRevokableGapsSpace = ~0;
    size_t sackHeaderLength = ~0;
    const uint32 totalGaps = state->gapList.getNumGaps(SctpGapList::GT_Any);
    bool compression = false;

    // ====== Record statistics ==============================================
    statisticsNumTotalGapBlocksStored->record(totalGaps);
    statisticsNumRevokableGapBlocksStored->record(numRevokableGaps);
    statisticsNumNonRevokableGapBlocksStored->record(numNonRevokableGaps);
    statisticsNumDuplicatesStored->record(numDups);

    // ====== Optimization ===================================================
    const int optR = (int)numRevokableGaps - (int)totalGaps;
    const int optNR = (int)numNonRevokableGaps - (int)totalGaps;

    if ((state->gapListOptimizationVariant == SctpStateVariables::GLOV_Shrunken) &&
        ((numRevokableGaps > 0) || (numNonRevokableGaps > 0)))
    {
        compression = true;
    }

    // ------ Optimization 1: R=ANY, NR=non-revokable ------
    if ((state->nrSack == true) &&
        (((optR > 0) && (state->gapListOptimizationVariant == SctpStateVariables::GLOV_Optimized1)) ||
         ((optR > 0) && (optR >= optNR) && (state->gapListOptimizationVariant >= SctpStateVariables::GLOV_Optimized2))))
    {
        assert(totalGaps < numRevokableGaps);
        sackHeaderLength = (compression == true) ? SCTP_COMPRESSED_NRSACK_CHUNK_LENGTH : SCTP_NRSACK_CHUNK_LENGTH;
        numRevokableGaps = copyToRGaps(sackChunk, &state->gapList, SctpGapList::GT_Any, compression, revokableGapsSpace);    // Add ALL
        numNonRevokableGaps = copyToNRGaps(sackChunk, &state->gapList, SctpGapList::GT_NonRevokable, compression, nonRevokableGapsSpace);    // Add NR-acks only
        assert(numRevokableGaps == totalGaps);
        // opt += optR;
    }
    // ------ Optimization 2: NR=ANY, R=difference ---------
    else if ((state->nrSack == true) &&
             (optNR > 0) && (state->gapListOptimizationVariant >= SctpStateVariables::GLOV_Optimized2))
    {
        assert(totalGaps < numNonRevokableGaps);
        sackChunk->setNrSubtractRGaps(true);
        sackHeaderLength = (compression == true) ? SCTP_COMPRESSED_NRSACK_CHUNK_LENGTH : SCTP_NRSACK_CHUNK_LENGTH;
        numRevokableGaps = copyToRGaps(sackChunk, &state->gapList, SctpGapList::GT_Revokable, compression, revokableGapsSpace);    // Add R-acks only
        numNonRevokableGaps = copyToNRGaps(sackChunk, &state->gapList, SctpGapList::GT_Any, compression, nonRevokableGapsSpace);    // Add ALL
        assert(numNonRevokableGaps == totalGaps);
        // opt += optNR;
    }
    else {
        // ------ Regular NR-SACK ---------------------------
        if (state->nrSack == true) {
            sackHeaderLength = SCTP_NRSACK_CHUNK_LENGTH;
            if (compression == true) {
                sackHeaderLength = SCTP_COMPRESSED_NRSACK_CHUNK_LENGTH;
            }
            numRevokableGaps = copyToRGaps(sackChunk, &state->gapList, SctpGapList::GT_Revokable, compression, revokableGapsSpace);    // Add R-acks only
            numNonRevokableGaps = copyToNRGaps(sackChunk, &state->gapList, SctpGapList::GT_NonRevokable, compression, nonRevokableGapsSpace);    // Add NR-acks only
        }
        // ------ Regular SACK ------------------------------
        else {
            sackHeaderLength = SCTP_SACK_CHUNK_LENGTH;
            numRevokableGaps = copyToRGaps(sackChunk, &state->gapList, SctpGapList::GT_Any, false, revokableGapsSpace);    // Add ALL
            numNonRevokableGaps = 0;
            nonRevokableGapsSpace = 0;
        }
    }

    // ====== SACK has to be shortened to fit in MTU ===========================
    uint32 sackLength = sackHeaderLength + revokableGapsSpace + nonRevokableGapsSpace + numDups * 4;
    if (sackLength > allowedLength) {
        // Strategy to reduce the SACK size:
        // - Report no duplicates (they are not used for congestion control)
        // - Split the remaining space equally between
        //   revokable and non-revokable GapAcks

        // ====== Drop duplicates list ========================================
        numDups = 0;
        sackLength -= 4 * numDups;

        if (sackLength > allowedLength) {
            // Unfortunately, dropping the duplicates has not solved the problem.
            //    => Now, the gap lists have to be shortened!

            auto iter = sctpMain->assocStatMap.find(assocId);
            iter->second.numOverfullSACKs++;
            // ====== Undo NR optimization ====================================
            if (sackChunk->getNrSubtractRGaps() == true) {
                sackChunk->setNrSubtractRGaps(false);    // Unset SubtractRGaps!
                // This optimization cannot work when lists have to be shortened.
                // Just use regular NR list.
            }
            revokableGapsSpace = allowedLength - sackHeaderLength;
            if (totalGaps < (state->gapList.getNumGaps(SctpGapList::GT_Revokable))) {
                numRevokableGaps = copyToRGaps(sackChunk, &state->gapList, SctpGapList::GT_Any, compression, revokableGapsSpace);    // Add ALL
            }
            else {
                numRevokableGaps = copyToRGaps(sackChunk, &state->gapList, SctpGapList::GT_Revokable, compression, revokableGapsSpace);    // Add R-acks only
            }
            sackLength = sackHeaderLength + revokableGapsSpace + nonRevokableGapsSpace + numDups * 4;

            // ====== Shorten gap lists ========================================
            if (state->smartOverfullSACKHandling) {
                if (sackHeaderLength + revokableGapsSpace < allowedLength) {
                    // Fill NR-acks up to allowed size
                    nonRevokableGapsSpace = allowedLength - sackHeaderLength - revokableGapsSpace;
                    numNonRevokableGaps = copyToNRGaps(sackChunk, &state->gapList, SctpGapList::GT_NonRevokable, compression, nonRevokableGapsSpace);    // Add NR-acks only
                    sackLength = sackHeaderLength + revokableGapsSpace + nonRevokableGapsSpace + numDups * 4;
                }
                else {
                    // Not even space to set R-acks => cut R-acks, no NR-acks!
                    nonRevokableGapsSpace = allowedLength - sackHeaderLength;
                    numRevokableGaps = copyToRGaps(sackChunk, &state->gapList, SctpGapList::GT_Any, compression, revokableGapsSpace);    // Add ALL
                }
            }
            else {
                if (sackLength > allowedLength) {
                    double revokableFraction = 1.0;
                    const uint32 blocksBeRemoved = (sackLength - allowedLength) / 4;
                    if (numRevokableGaps + numNonRevokableGaps > 0) {
                        revokableFraction = numRevokableGaps / (double)(numRevokableGaps + numNonRevokableGaps);
                    }
                    const uint32 removeRevokable = (uint32)ceil(blocksBeRemoved * revokableFraction);
                    const uint32 removeNonRevokable = (uint32)ceil(blocksBeRemoved * (1.0 - revokableFraction));
                    numRevokableGaps -= std::min(removeRevokable, numRevokableGaps);
                    numNonRevokableGaps -= std::min(removeNonRevokable, numNonRevokableGaps);
                    revokableGapsSpace = 4 * numRevokableGaps;
                    nonRevokableGapsSpace = 4 * numNonRevokableGaps;
                    numRevokableGaps = copyToRGaps(sackChunk, &state->gapList, SctpGapList::GT_Revokable, compression, revokableGapsSpace);    // Add R-acks only
                    numNonRevokableGaps = copyToNRGaps(sackChunk, &state->gapList, SctpGapList::GT_NonRevokable, compression, nonRevokableGapsSpace);    // Add NR-acks only
                    sackLength = sackHeaderLength + revokableGapsSpace + nonRevokableGapsSpace + numDups * 4;
                }
            }

            assert(sackLength <= allowedLength);

            // Update values in SACK chunk ...
            sackChunk->setNumGaps(numRevokableGaps);
            sackChunk->setNumNrGaps(numNonRevokableGaps);
        }
    }
    sackChunk->setNumDupTsns(numDups);
    sackChunk->setByteLength(sackLength);

    // ====== Apply limit ====================================================
    if (state->gapReportLimit < 1000000) {
        if (!compression) {
            numRevokableGaps = std::min(numRevokableGaps, state->gapReportLimit);
            numNonRevokableGaps = std::min(numNonRevokableGaps, state->gapReportLimit);
            // Update values in SACK chunk ...
            sackChunk->setNumGaps(numRevokableGaps);
            sackChunk->setNumNrGaps(numNonRevokableGaps);
            sackLength = sackHeaderLength + revokableGapsSpace + nonRevokableGapsSpace + numDups * 4;
        } else {
            assert(false);    // NOTE: IMPLEMENT ME!
        }
    }

    // ====== Add duplicates =================================================
    if (numDups > 0) {
        sackChunk->setDupTsnsArraySize(numDups);
        uint32 key = 0;
        for (auto & elem : state->dupList) {
            sackChunk->setDupTsns(key, elem);
            key++;
            if (key == numDups)
                break;
        }
        state->dupList.clear();
    }

    // ====== Record statistics ==============================================
    statisticsSACKLengthSent->record(sackLength);
    statisticsNumRevokableGapBlocksSent->record(numRevokableGaps);
    statisticsNumNonRevokableGapBlocksSent->record(numNonRevokableGaps);
    statisticsNumDuplicatesSent->record(numDups);

    // ====== Print information ==============================================
    EV_DEBUG << "createSack:"
             << " bufferedMessages=" << state->bufferedMessages
             << " msgRwnd=" << msgRwnd
             << " arwnd=" << sackChunk->getA_rwnd()
             << " cumAck=" << state->gapList.getCumAckTsn()
             << " numRGaps=" << numRevokableGaps
             << " numNRGaps=" << numNonRevokableGaps
             << " numDups=" << numDups
             << " gapList={" << state->gapList << "}"
             << endl;
    return sackChunk;
}

void SctpAssociation::sendSack()
{
    SctpAuthenticationChunk *authChunk;
    SctpSackChunk *sackChunk;

    EV_INFO << "Sending SACK" << endl;

    /* sack timer has expired, reset flag, and send SACK */
    stopTimer(SackTimer);
    state->ackState = 0;
    sackChunk = createSack();

    const auto& sctpmsg = makeShared<SctpHeader>();
    sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));
    if (state->auth && state->peerAuth && typeInChunkList(SACK)) {
        authChunk = createAuthChunk();
        sctpmsg->insertSctpChunks(authChunk);
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpmsg->insertSctpChunks(sackChunk);

    sendSACKviaSelectedPath(sctpmsg);
}

void SctpAssociation::sendDataArrivedNotification(uint16 sid)
{
    EV_INFO << "SendDataArrivedNotification\n";

    Indication *cmsg = new Indication("SCTP_I_DATA_NOTIFICATION", SCTP_I_DATA_NOTIFICATION);
    auto cmd = cmsg->addTag<SctpCommandReq>();
    cmd->setSocketId(assocId);
    cmd->setSid(sid);
    cmd->setNumMsgs(1);
   // cmsg->setControlInfo(cmd);

    sendToApp(cmsg);
}

void SctpAssociation::sendInvalidStreamError(uint16 sid)
{
    const auto& sctpmsg = makeShared<SctpHeader>();
    sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));
    SctpErrorChunk *errorChunk = new SctpErrorChunk();
    errorChunk->setSctpChunkType(ERRORTYPE);
    SctpSimpleErrorCauseParameter *cause = new SctpSimpleErrorCauseParameter();
    cause->setParameterType(INVALID_STREAM_IDENTIFIER);
    cause->setByteLength(8);
    cause->setValue(sid);
    errorChunk->setByteLength(4);
    errorChunk->addParameters(cause);
    sctpmsg->insertSctpChunks(errorChunk);

    stopTimer(SackTimer);
    state->ackState = 0;
    SctpSackChunk *sackChunk = createSack();

    if (state->auth && state->peerAuth && typeInChunkList(SACK)) {
        SctpAuthenticationChunk *authChunk = createAuthChunk();
        sctpmsg->insertSctpChunks(authChunk);
        auto it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpmsg->insertSctpChunks(sackChunk);
    sendSACKviaSelectedPath(sctpmsg);
}

void SctpAssociation::sendHMacError(const uint16 id)
{
    const auto& sctpmsg = makeShared<SctpHeader>();
    sctpmsg->setChunkLength(B(SCTP_COMMON_HEADER));
    SctpErrorChunk *errorChunk = new SctpErrorChunk();
    errorChunk->setSctpChunkType(ERRORTYPE);
    SctpSimpleErrorCauseParameter *cause = new SctpSimpleErrorCauseParameter();
    cause->setParameterType(UNSUPPORTED_HMAC);
    cause->setByteLength(6);
    cause->setValue(id);
    errorChunk->setByteLength(4);
    errorChunk->addParameters(cause);
    sctpmsg->insertSctpChunks(errorChunk);
}

void SctpAssociation::putInDeliveryQ(uint16 sid)
{
    SctpReceiveStream *rStream = receiveStreams.find(sid)->second;
    EV_INFO << "putInDeliveryQ: SSN=" << rStream->getExpectedStreamSeqNum()
            << " SID=" << sid
            << " QueueSize=" << rStream->getOrderedQ()->getQueueSize() << endl;
    while (rStream->getOrderedQ()->getQueueSize() > 0) {
        /* dequeue first from reassembly Q */
        SctpDataVariables *chunk =
            rStream->getOrderedQ()->dequeueChunkBySSN(rStream->getExpectedStreamSeqNum());
        if (chunk) {
            EV_DETAIL << "putInDeliveryQ::chunk " << chunk->tsn
                      << ", sid " << chunk->sid << " and ssn " << chunk->ssn
                      << " dequeued from ordered queue. queuedReceivedBytes="
                      << state->queuedReceivedBytes << " will be reduced by "
                      << chunk->len / 8 << endl;
            state->bufferedMessages--;
            state->queuedReceivedBytes -= chunk->len / 8;
            qCounter.roomSumRcvStreams -= ADD_PADDING(chunk->len / 8 + SCTP_DATA_CHUNK_LENGTH);

            if (rStream->getDeliveryQ()->checkAndInsertChunk(chunk->tsn, chunk)) {
                state->bufferedMessages++;
                state->queuedReceivedBytes += chunk->len / 8;

                EV_DETAIL << "data put in deliveryQ; queuedBytes now "
                          << state->queuedReceivedBytes << endl;
                qCounter.roomSumRcvStreams += ADD_PADDING(chunk->len / 8 + SCTP_DATA_CHUNK_LENGTH);
                int32 seqnum = rStream->getExpectedStreamSeqNum();
                rStream->setExpectedStreamSeqNum(++seqnum);
                if (rStream->getExpectedStreamSeqNum() > 65535) {
                    rStream->setExpectedStreamSeqNum(0);
                }
            }
        }
        else {
            break;
        }
    }
}

void SctpAssociation::pushUlp()
{
    int32 count = 0;

    for (unsigned int i = 0; i < inboundStreams; i++) {    //12.06.08
        putInDeliveryQ(i);
    }
    if (state->pushMessagesLeft <= 0) {
        state->pushMessagesLeft = state->messagesToPush;
    }
    bool restrict = false;
    if (state->pushMessagesLeft > 0) {
        restrict = true;
    }

    statisticsQueuedReceivedBytes->record(state->queuedReceivedBytes);

    EV_DETAIL << simTime() << " Calling pushUlp(" << state->queuedReceivedBytes
              << " bytes queued) ..." << endl
              << "messagesToPush=" << state->messagesToPush
              << " pushMessagesLeft=" << state->pushMessagesLeft
              << " restrict=" << restrict
              << " buffered Messages=" << state->bufferedMessages << endl;
    uint32 i = state->nextRSid;
    uint64 tempQueuedBytes = 0;
    do {
        auto iter = receiveStreams.find(i);
        SctpReceiveStream *rStream = iter->second;
        EV_DETAIL << "Size of stream " << iter->first << ": "
                  << rStream->getDeliveryQ()->getQueueSize() << endl;

        while ((!rStream->getDeliveryQ()->payloadQueue.empty()) &&
               (!restrict || (restrict && state->pushMessagesLeft > 0)))
        {
            SctpDataVariables *chunk = rStream->getDeliveryQ()->extractMessage();
            qCounter.roomSumRcvStreams -= ADD_PADDING(chunk->len / 8 + SCTP_DATA_CHUNK_LENGTH);

            if (state->pushMessagesLeft > 0)
                state->pushMessagesLeft--;

            // ====== Non-revokably acknowledge chunks of the message ==========
          /*  bool dummy;
            for (uint32 j = chunk->tsn; j < chunk->tsn + chunk->fragments; j++)
                state->gapList.updateGapList(j, dummy, false);*/

            tempQueuedBytes = state->queuedReceivedBytes;
            state->queuedReceivedBytes -= chunk->len / 8;
            state->bufferedMessages--;
            EV_INFO << "buffered Messages now " << state->bufferedMessages << endl;
            if (state->swsAvoidanceInvoked) {
                statisticsQueuedReceivedBytes->record(state->queuedReceivedBytes);
                /* now check, if user has read enough so that window opens up more than one MTU */
                if ((state->messageAcceptLimit > 0 &&
                     (int32)state->localMsgRwnd - state->bufferedMessages >= 3 &&
                     (int32)state->localMsgRwnd - state->bufferedMessages <= 8)
                    ||
                    (state->messageAcceptLimit == 0 &&
                     (int32)(state->localRwnd - state->queuedReceivedBytes - state->bufferedMessages * state->bytesToAddPerRcvdChunk) >= (int32)(state->swsLimit) &&
                     (int32)(state->localRwnd - state->queuedReceivedBytes - state->bufferedMessages * state->bytesToAddPerRcvdChunk) <= (int32)(state->swsLimit + state->assocPmtu)))
                {
                    state->swsMsgInvoked = false;
                    /* only if the window has opened up more than one MTU we will send a SACK */
                    state->swsAvoidanceInvoked = false;
                    EV_DETAIL << "pushUlp: Window opens up to " << (int32)state->localRwnd - state->queuedReceivedBytes << " bytes: sending a SACK. SWS Avoidance INACTIVE\n";

                    sendSack();
                }
            }
            else if ((int32)(state->swsLimit) == 0) {
                sendSack();
            }
            else if ((tempQueuedBytes > state->localRwnd * 3 / 4) && (state->queuedReceivedBytes <= state->localRwnd * 3 / 4)) {
                sendSack();
            }
            EV_DETAIL << "Push TSN " << chunk->tsn
                      << ": sid=" << chunk->sid << " ssn=" << chunk->ssn << endl;

            SctpSimpleMessage *smsg = check_and_cast<SctpSimpleMessage *>(chunk->userData);
            auto applicationPacket = new Packet("ApplicationPacket", SCTP_I_DATA);
            std::vector<uint8_t> vec;
            int sendBytes = smsg->getDataLen();
            vec.resize(sendBytes);
            for (int i = 0; i < sendBytes; i++)
                vec[i] = smsg->getData(i);
            auto applicationData = makeShared<BytesChunk>();
            applicationData->setBytes(vec);
            applicationData->addTag<CreationTimeTag>()->setCreationTime(smsg->getCreationTime());
            SctpRcvReq *cmd = applicationPacket->addTag<SctpRcvReq>();
            cmd->setSocketId(assocId);
            cmd->setGate(appGateIndex);
            cmd->setSid(chunk->sid);
            cmd->setSsn(chunk->ssn);
            cmd->setSendUnordered(!chunk->ordered);
            cmd->setLocalAddr(localAddr);
            cmd->setRemoteAddr(remoteAddr);
            cmd->setPpid(chunk->ppid);
            cmd->setTsn(chunk->tsn);
            cmd->setCumTsn(state->lastTsnAck);
            applicationPacket->insertAtBack(applicationData);
            state->numMsgsReq[count]--;
            EndToEndDelay->record(simTime() - chunk->firstSendTime);
            auto iter = sctpMain->assocStatMap.find(assocId);
            if (iter->second.numEndToEndMessages >= iter->second.startEndToEndDelay &&
                (iter->second.numEndToEndMessages < iter->second.stopEndToEndDelay || !iter->second.stopEndToEndDelay))
            {
                iter->second.cumEndToEndDelay += (simTime() - chunk->firstSendTime);
            }
            iter->second.numEndToEndMessages++;

            // set timestamp to sending time
            chunk->userData->setTimestamp(chunk->firstSendTime);
            delete smsg;
            delete chunk;
            sendToApp(applicationPacket);
        }
        i = (i + 1) % inboundStreams;
        count++;
    } while (i != state->nextRSid);

    state->nextRSid = (state->nextRSid + 1) % inboundStreams;
    if (restrict && state->bufferedMessages > 0) {
        for (auto & elem : receiveStreams) {
            if (!(elem.second->getDeliveryQ()->payloadQueue.empty())) {
                sendDataArrivedNotification(elem.second->getStreamId());
                break;
            }
        }
    }
    if ((state->queuedReceivedBytes == 0) && (fsm->getState() == SCTP_S_SHUTDOWN_ACK_SENT)) {
        EV_INFO << "SCTP_E_CLOSE" << endl;
        performStateTransition(SCTP_E_CLOSE);
    }
}

SctpDataChunk *SctpAssociation::transformDataChunk(SctpDataVariables *chunk)
{
    SctpDataChunk *dataChunk = new SctpDataChunk();
    SctpSimpleMessage *msg = check_and_cast<SctpSimpleMessage *>(chunk->userData->dup());
    dataChunk->setSctpChunkType(DATA);
    dataChunk->setBBit(chunk->bbit);
    dataChunk->setEBit(chunk->ebit);
    if (chunk->ordered) {
        dataChunk->setUBit(0);
    }
    else {
        dataChunk->setUBit(1);
    }
    dataChunk->setTsn(chunk->tsn);
    dataChunk->setSid(chunk->sid);
    dataChunk->setSsn(chunk->ssn);
    dataChunk->setPpid(chunk->ppid);
    dataChunk->setIBit(chunk->ibit);
    dataChunk->setEnqueuingTime(chunk->enqueuingTime);
    dataChunk->setFirstSendTime(chunk->firstSendTime);
    dataChunk->setByteLength(SCTP_DATA_CHUNK_LENGTH);
    msg->setByteLength(chunk->len / 8);
    dataChunk->encapsulate(msg);
    dataChunk->setLength(dataChunk->getByteLength());
    return dataChunk;
}

void SctpAssociation::addPath(const L3Address& addr)
{
    EV_INFO << "Add Path remote address: " << addr << "\n";

    auto i = sctpPathMap.find(addr);
    if (i == sctpPathMap.end()) {
        EV_DEBUG << " get new path for " << addr << " at line " << __LINE__ << "\n";
        SctpPathVariables *path = new SctpPathVariables(addr, this, rt);
        sctpPathMap[addr] = path;
        qCounter.roomTransQ[addr] = 0;
        qCounter.bookedTransQ[addr] = 0;
        qCounter.roomRetransQ[addr] = 0;
    }
    EV_INFO << "path added\n";
}

void SctpAssociation::removePath(const L3Address& addr)
{
    auto pathIterator = sctpPathMap.find(addr);
    if (pathIterator != sctpPathMap.end()) {
        SctpPathVariables *path = pathIterator->second;
        path->cwnd = 0;
        path->ssthresh = 0;
        recordCwndUpdate(path);

        stopTimer(path->HeartbeatTimer);
        delete path->HeartbeatTimer;
        stopTimer(path->HeartbeatIntervalTimer);
        delete path->HeartbeatIntervalTimer;
        stopTimer(path->T3_RtxTimer);
        delete path->T3_RtxTimer;
        stopTimer(path->CwndTimer);
        delete path->CwndTimer;
        sctpPathMap.erase(pathIterator);
        stopTimer(path->ResetTimer);
        delete path->ResetTimer;
        stopTimer(path->AsconfTimer);
        delete path->AsconfTimer;
        delete path;
    }
}

bool SctpAssociation::makeRoomForTsn(const uint32 tsn, const uint32 length, const bool uBit)
{
    EV_INFO << simTime() << ":\tmakeRoomForTsn:"
            << " tsn=" << tsn
            << " length=" << length
            << " highestTsn=" << state->gapList.getHighestTsnReceived() << endl;
    calculateRcvBuffer();

    // Reneging may not happen when it is turned off!
    assert(state->disableReneging == false);

    // Get the highest TSN of the GapAck blocks.
    uint32 tryTsn = state->gapList.getHighestTsnReceived();
    uint32 sum = 0;
    while ((sum < length) &&
           (tryTsn > state->gapList.getCumAckTsn()))
    {
        // ====== New TSN is larger than highest one in GapList? ==============
        if (tsnGt(tsn, tryTsn)) {
            // There is no space for a TSN that high!
            EV_DETAIL << "makeRoomForTsn:"
                      << " tsn=" << tryTsn
                      << " tryTsn=" << tryTsn << " -> no space" << endl;
            return false;
        }

        const uint32 oldSum = sum;
        // ====== Iterate all streams to find chunk with TSN "tryTsn" =========
        for (auto & elem : receiveStreams) {
            SctpReceiveStream *receiveStream = elem.second;

            // ====== Get chunk to drop ========================================
            SctpQueue *queue;
            if (uBit) {
                queue = receiveStream->getUnorderedQ();    // Look in unordered queue
            } else {
                queue = receiveStream->getOrderedQ();    // Look in ordered queue
            }
            SctpDataVariables *chunk = queue->getChunk(tryTsn);
            if (chunk == nullptr) {    // 12.06.08
                EV_DETAIL << tryTsn << " not found in orderedQ. Try deliveryQ" << endl;
                // Chunk is already in delivery queue.
                queue = receiveStream->getDeliveryQ();
                chunk = queue->getChunk(tryTsn);
            }

            // ====== A chunk has been found -> drop it ========================
            if (chunk != nullptr) {
                sum += chunk->len;
                if (queue->deleteMsg(tryTsn)) {
                    EV_INFO << tryTsn << " found and deleted" << endl;
                    state->bufferedMessages--;
                    state->queuedReceivedBytes -= chunk->len / 8;
                    if (ssnGt(receiveStream->getExpectedStreamSeqNum(), chunk->ssn)) {
                        receiveStream->setExpectedStreamSeqNum(chunk->ssn);
                    }

                    auto iter = sctpMain->assocStatMap.find(assocId);
                    iter->second.numChunksReneged++;
                }
                qCounter.roomSumRcvStreams -= ADD_PADDING(chunk->len / 8 + SCTP_DATA_CHUNK_LENGTH);
                state->gapList.removeFromGapList(tryTsn);

                break;
            } else {
                EV_INFO << "TSN " << tryTsn << " not found in stream "
                        << receiveStream->getStreamId() << endl;
            }
        }
        if (sum == oldSum) {
            EV_INFO << tryTsn << " not found in any stream" << endl;
        }
        tryTsn--;
    }

    return true;
}

bool SctpAssociation::tsnIsDuplicate(const uint32 tsn) const
{
    for (std::list<uint32>::const_iterator iterator = state->dupList.begin();
         iterator != state->dupList.end(); iterator++)
    {
        if ((*iterator) == tsn)
            return true;
    }
    return state->gapList.tsnInGapList(tsn);
}

SctpDataVariables *SctpAssociation::makeVarFromMsg(SctpDataChunk *dataChunk)
{
    SctpDataVariables *chunk = new SctpDataVariables();

    chunk->bbit = dataChunk->getBBit();
    chunk->ebit = dataChunk->getEBit();
    chunk->ibit = dataChunk->getIBit();
    chunk->sid = dataChunk->getSid();
    chunk->ssn = dataChunk->getSsn();
    chunk->ppid = dataChunk->getPpid();
    chunk->tsn = dataChunk->getTsn();
    if (!dataChunk->getUBit()) {
        chunk->ordered = true;
    }
    else {
        chunk->ordered = false;
    }
    SctpSimpleMessage *smsg = check_and_cast<SctpSimpleMessage *>(dataChunk->decapsulate());
            /*    auto& smsg = dataChunk->Chunk::peek<SctpSimpleMessage>(Chunk::BackwardIterator(B(0)));*/

    chunk->userData = smsg;
    EV_INFO << "smsg encapsulate? " << smsg->getEncaps() << endl;
    if (smsg->getEncaps())
        chunk->len = smsg->getByteLength() * 8;
    else
        chunk->len = smsg->getDataLen() * 8;
    chunk->firstSendTime = dataChunk->getFirstSendTime();
    calculateRcvBuffer();

    EV_INFO << "makeVarFromMsg: queuedBytes has been increased to "
            << state->queuedReceivedBytes << endl;
    return chunk;
}

void SctpAssociation::advancePeerTsn()
{
    // Rewrote code for efficiency, it consomed >40% of total CPU time before!
    // Find the highest TSN to advance to, not just the first one.
    auto iterator = retransmissionQ->payloadQueue.find(state->advancedPeerAckPoint + 1);
    while (iterator != retransmissionQ->payloadQueue.end()) {
        if (iterator->second->hasBeenAbandoned) {
            state->advancedPeerAckPoint = iterator->second->tsn;
            state->ackPointAdvanced = true;
            iterator++;
        }
        else {
            if (iterator->second->hasBeenAcked == true)
                iterator++;
            else
                break;
        }
    }

    EV_INFO << "advancedPeerTsnAck now=" << state->advancedPeerAckPoint << endl;
}

SctpDataVariables *SctpAssociation::getOutboundDataChunk(const SctpPathVariables *path,
        int32 availableSpace,
        int32 availableCwnd)
{
    /* are there chunks in the transmission queue ? If Yes -> dequeue and return it */
    EV_INFO << "getOutboundDataChunk(" << path->remoteAddress << "):"
            << " availableSpace=" << availableSpace
            << " availableCwnd=" << availableCwnd
            << endl;
    if (!transmissionQ->payloadQueue.empty()) {
        for (auto it = transmissionQ->payloadQueue.begin();
             it != transmissionQ->payloadQueue.end(); it++)
        {
            SctpDataVariables *chunk = it->second;
            if ((chunkHasBeenAcked(chunk) == false) && !chunk->hasBeenAbandoned &&
                (chunk->getNextDestinationPath() == path))
            {
                const int32 len = ADD_PADDING(chunk->len / 8 + SCTP_DATA_CHUNK_LENGTH);
                EV_DETAIL << "getOutboundDataChunk() found chunk " << chunk->tsn
                          << " in the transmission queue, length=" << len << endl;
                if ((len <= availableSpace) &&
                    ((int32)chunk->booksize <= availableCwnd))
                {
                    // T.D. 05.01.2010: The bookkeeping counters may only be decreased when
                    //                        this chunk is actually dequeued. Therefore, the check
                    //                        for "chunkHasBeenAcked==false" has been moved into the
                    //                        "if" statement above!
                    transmissionQ->payloadQueue.erase(it);
                    chunk->enqueuedInTransmissionQ = false;
                    auto i = qCounter.roomTransQ.find(path->remoteAddress);
                    i->second -= ADD_PADDING(chunk->len / 8 + SCTP_DATA_CHUNK_LENGTH);
                    auto ib = qCounter.bookedTransQ.find(path->remoteAddress);
                    ib->second -= chunk->booksize;
                    return chunk;
                }
            }
        }
    }
    EV_INFO << "no chunk found in transmissionQ\n";
    return nullptr;
}

bool SctpAssociation::chunkMustBeAbandoned(SctpDataVariables *chunk, SctpPathVariables *sackPath)
{
    switch (chunk->prMethod) {
        case PR_TTL:
            if (chunk->expiryTime > 0 && chunk->expiryTime <= simTime()) {
                if (!chunk->hasBeenAbandoned) {
                    EV_INFO << "TSN " << chunk->tsn << " will be abandoned"
                            << " (expiryTime=" << chunk->expiryTime
                            << " sendTime=" << chunk->sendTime << ")" << endl;
                    chunk->hasBeenAbandoned = true;
                    chunk->sendForwardIfAbandoned = true;
                    sendIndicationToApp(SCTP_I_ABANDONED);
                }
            }
            break;

        case PR_RTX:
            if (chunk->numberOfRetransmissions >= chunk->allowedNoRetransmissions) {
                if (!chunk->hasBeenAbandoned) {
                    EV_INFO << "chunkMustBeAbandoned: TSN " << chunk->tsn << " will be abandoned"
                            << " (maxRetransmissions=" << chunk->allowedNoRetransmissions << ")" << endl;
                    chunk->hasBeenAbandoned = true;
                    chunk->sendForwardIfAbandoned = true;
                    decreaseOutstandingBytes(chunk);
                    chunk->countsAsOutstanding = false;
                    sendIndicationToApp(SCTP_I_ABANDONED);
                }
            }
            break;
    }

    if (chunk->hasBeenAbandoned) {
        return true;
    }
    return false;
}

SctpDataVariables *SctpAssociation::peekAbandonedChunk(const SctpPathVariables *path)
{
    SctpDataVariables *retChunk = nullptr;

    if (state->prMethod != 0 && !retransmissionQ->payloadQueue.empty()) {
        for (auto & elem : retransmissionQ->payloadQueue) {
            SctpDataVariables *chunk = elem.second;

            if (chunk->getLastDestinationPath() == path) {
                /* Apply policies if necessary */
                if (!chunk->hasBeenAbandoned && !chunk->hasBeenAcked &&
                    (chunk->hasBeenFastRetransmitted || chunk->hasBeenTimerBasedRtxed))
                {
                    switch (chunk->prMethod) {
                        case PR_TTL:
                            if (chunk->expiryTime > 0 && chunk->expiryTime <= simTime()) {
                                if (!chunk->hasBeenAbandoned) {
                                    EV_DETAIL << "TSN " << chunk->tsn << " will be abandoned"
                                              << " (expiryTime=" << chunk->expiryTime
                                              << " sendTime=" << chunk->sendTime << ")" << endl;
                                    chunk->hasBeenAbandoned = true;
                                    sendIndicationToApp(SCTP_I_ABANDONED);
                                }
                            }
                            break;

                        case PR_RTX:
                            if (chunk->hasBeenFastRetransmitted && chunk->numberOfRetransmissions >= chunk->allowedNoRetransmissions) {
                                if (!chunk->hasBeenAbandoned) {
                                    EV_DETAIL << "peekAbandonedChunk: TSN " << chunk->tsn << " will be abandoned"
                                              << " (maxRetransmissions=" << chunk->allowedNoRetransmissions << ")" << endl;
                                    chunk->hasBeenAbandoned = true;
                                    sendIndicationToApp(SCTP_I_ABANDONED);
                                }
                            }
                            break;
                    }
                }

                if (chunk->hasBeenAbandoned && chunk->sendForwardIfAbandoned) {
                    retChunk = chunk;
                }
            }
        }
    }
    return retChunk;
}

void SctpAssociation::fragmentOutboundDataMsgs() {
    cPacketQueue *streamQ = nullptr;
    for (auto iter = sendStreams.begin(); iter != sendStreams.end(); ++iter) {
        SctpSendStream *stream = iter->second;
        streamQ = nullptr;

        if (!stream->getUnorderedStreamQ()->isEmpty()) {
            streamQ = stream->getUnorderedStreamQ();
            EV_DETAIL << "fragmentOutboundDataMsgs() found chunks in stream " << iter->first << " unordered queue, queue size=" << stream->getUnorderedStreamQ()->getLength() << "\n";
        }
        else if (!stream->getStreamQ()->isEmpty()) {
            streamQ = stream->getStreamQ();
            EV_DETAIL << "fragmentOutboundDataMsgs() found chunks in stream " << iter->first << " ordered queue, queue size=" << stream->getStreamQ()->getLength() << "\n";
        }

        if (streamQ) {
            int32 b = ADD_PADDING(check_and_cast<SctpDataMsg *>(streamQ->front())->getEncapsulatedPacket()->getByteLength() + SCTP_DATA_CHUNK_LENGTH);

            /* check if chunk found in queue has to be fragmented */
            if (b > (int32)state->fragPoint + (int32)SCTP_DATA_CHUNK_LENGTH) {
                /* START FRAGMENTATION */
                SctpDataMsg *datMsgQueued = check_and_cast<SctpDataMsg *>(streamQ->pop());
                cPacket *datMsgQueuedEncMsg = datMsgQueued->getEncapsulatedPacket();
                SctpDataMsg *datMsgLastFragment = nullptr;
                uint32 offset = 0;
                uint32 msgbytes = state->fragPoint;
                const uint16 fullSizedPackets = (uint16)(datMsgQueued->getByteLength() / msgbytes);
                EV_DETAIL << "Fragmentation: chunk " << &datMsgQueued << " - size = " << datMsgQueued->getByteLength() << endl;
                EV_DETAIL << assocId << ": number of fullSizedPackets: " << fullSizedPackets << endl;
                uint16 pcounter = 0;

                while (datMsgQueued) {
                    /* detemine size of fragment, either max payload or what's left */
                    if (msgbytes > datMsgQueuedEncMsg->getByteLength() - offset)
                        msgbytes = datMsgQueuedEncMsg->getByteLength() - offset;

                    /* new DATA msg */
                    SctpDataMsg *datMsgFragment = new SctpDataMsg();
                    datMsgFragment->setSid(datMsgQueued->getSid());
                    datMsgFragment->setPpid(datMsgQueued->getPpid());
                    if (++pcounter == fullSizedPackets && sctpMain->sackNow)
                        datMsgFragment->setSackNow(true);
                    else
                        datMsgFragment->setSackNow(datMsgQueued->getSackNow());
                    datMsgFragment->setInitialDestination(datMsgQueued->getInitialDestination());
                    datMsgFragment->setEnqueuingTime(datMsgQueued->getEnqueuingTime());
                    datMsgFragment->setPrMethod(datMsgQueued->getPrMethod());
                    datMsgFragment->setPriority(datMsgQueued->getPriority());
                    //EV_DETAIL << "felix: " << datMsgQueued->getPriority() << endl;
                    datMsgFragment->setStrReset(datMsgQueued->getStrReset());
                    datMsgFragment->setMsgNum(datMsgQueued->getMsgNum());
                    datMsgFragment->setOrdered(datMsgQueued->getOrdered());
                    datMsgFragment->setExpiryTime(datMsgQueued->getExpiryTime());
                    datMsgFragment->setRtx(datMsgQueued->getRtx());
                    datMsgFragment->setFragment(true);

                    if (state->padding)
                        datMsgFragment->setBooksize(ADD_PADDING(msgbytes + state->header));
                    else
                        datMsgFragment->setBooksize(msgbytes + state->header);

                    /* is this the first fragment? */
                    if (offset == 0)
                        datMsgFragment->setBBit(true);

                    /* new msg */
                    cPacket *datMsgFragmentEncMsg = datMsgQueuedEncMsg->dup();

                    datMsgFragmentEncMsg->setByteLength(msgbytes);

                    SctpSimpleMessage *datMsgQueuedSimple = dynamic_cast<SctpSimpleMessage *>(datMsgQueuedEncMsg);
                    SctpSimpleMessage *datMsgFragmentSimple = dynamic_cast<SctpSimpleMessage *>(datMsgFragmentEncMsg);
                    if ((datMsgQueuedSimple != nullptr) &&
                        (datMsgFragmentSimple != nullptr) &&
                        (datMsgQueuedSimple->getDataArraySize() >= msgbytes + offset))
                    {
                        datMsgFragmentSimple->setDataArraySize(msgbytes);
                        datMsgFragmentSimple->setDataLen(msgbytes);
                        /* copy data */
                        for (uint32 i = offset; i < offset + msgbytes; i++) {
                            datMsgFragmentSimple->setData(i - offset, datMsgQueuedSimple->getData(i));
                        }
                    }

                    offset += msgbytes;
                    //datMsgFragment->insertAtBack(datMsgFragmentEncMsg);
                    datMsgFragment->encapsulate(datMsgFragmentEncMsg);

                    /* insert fragment into queue */
                    if (!streamQ->isEmpty()) {
                        if (!datMsgLastFragment) {
                            /* insert first fragment at the begining of the queue*/
                            streamQ->insertBefore(check_and_cast<SctpDataMsg *>(streamQ->front()), datMsgFragment);
                        }
                        else {
                            /* insert fragment after last inserted   */
                            streamQ->insertAfter(datMsgLastFragment, datMsgFragment);
                        }
                    }
                    else
                        streamQ->insert(datMsgFragment);

                    state->queuedMessages++;
                    qCounter.roomSumSendStreams += ADD_PADDING(datMsgFragment->getByteLength() + SCTP_DATA_CHUNK_LENGTH);
                    qCounter.bookedSumSendStreams += datMsgFragment->getBooksize();
                    EV_DETAIL << "Fragmentation: fragment " << &datMsgFragment << " created, length = " << datMsgFragmentEncMsg->getByteLength() << ", queue size = " << streamQ->getLength() << endl;

                    datMsgLastFragment = datMsgFragment;

                    /* all fragments done? */
                    if (datMsgQueuedEncMsg->getByteLength() == offset) {
                        datMsgFragment->setEBit(true);
                        if (sctpMain->sackNow)
                            datMsgFragment->setSackNow(true);
                        /* remove original element */
                        EV_DETAIL << "Fragmentation: delete " << &datMsgQueued << endl;
                        //streamQ->pop();
                        qCounter.roomSumSendStreams -= ADD_PADDING(datMsgQueued->getByteLength() + SCTP_DATA_CHUNK_LENGTH);
                        qCounter.bookedSumSendStreams -= datMsgQueued->getBooksize();
                        delete datMsgQueued;
                        datMsgQueued = nullptr;
                        state->queuedMessages--;
                    }
                } // while
            }
        }
    }

}

SctpDataMsg *SctpAssociation::dequeueOutboundDataMsg(SctpPathVariables *path,
        int32 availableSpace,
        int32 availableCwnd)
{
    SctpDataMsg *datMsg = nullptr;
    cPacketQueue *streamQ = nullptr;
    int32 nextStream = -1;

    EV_INFO << "dequeueOutboundDataMsg: " << availableSpace << " bytes left to be sent" << endl;

    fragmentOutboundDataMsgs();

    /* Only change stream if we don't have to finish a fragmented message */
    if (state->lastMsgWasFragment)
        nextStream = state->lastStreamScheduled;
    else
        nextStream = (this->*ssFunctions.ssGetNextSid)(path, false);

    if (nextStream == -1)
        return nullptr;

    EV_INFO << "dequeueOutboundDataMsg: now stream " << nextStream << endl;

    SctpSendStream *stream = sendStreams.find(nextStream)->second;
    streamQ = nullptr;

    if (!stream->getUnorderedStreamQ()->isEmpty()) {
        streamQ = stream->getUnorderedStreamQ();
        EV_DETAIL << "DequeueOutboundDataMsg() found chunks in stream " << nextStream << " unordered queue, queue size=" << stream->getUnorderedStreamQ()->getLength() << "\n";
    }
    else if (!stream->getStreamQ()->isEmpty()) {
        streamQ = stream->getStreamQ();
        EV_DETAIL << "DequeueOutboundDataMsg() found chunks in stream " << nextStream << " ordered queue, queue size=" << stream->getStreamQ()->getLength() << "\n";
    }

    if (streamQ) {
        int32 b = ADD_PADDING(check_and_cast<SctpDataMsg *>(streamQ->front())->getEncapsulatedPacket()->getByteLength() + SCTP_DATA_CHUNK_LENGTH);

        if ((b <= availableSpace) &&
            ((int32)check_and_cast<SctpDataMsg *>(streamQ->front())->getBooksize() <= availableCwnd))
        {
            datMsg = check_and_cast<SctpDataMsg *>(streamQ->pop());
            sendQueue->record(streamQ->getLength());

            if (!datMsg->getFragment()) {
                datMsg->setBBit(true);
                datMsg->setEBit(true);
                state->lastMsgWasFragment = false;
            }
            else {
                if (datMsg->getEBit())
                    state->lastMsgWasFragment = false;
                else
                    state->lastMsgWasFragment = true;
            }

            EV_DETAIL << "DequeueOutboundDataMsg() found chunk (" << datMsg->str() << ") in the stream queue " << nextStream << "(" << streamQ << ") queue size=" << streamQ->getLength() << endl;
        }
    }


    if (datMsg != nullptr) {
        qCounter.roomSumSendStreams -= ADD_PADDING(datMsg->getEncapsulatedPacket()->getByteLength() + SCTP_DATA_CHUNK_LENGTH);
        qCounter.bookedSumSendStreams -= datMsg->getBooksize();
    }
    return datMsg;
}

bool SctpAssociation::nextChunkFitsIntoPacket(SctpPathVariables *path, int32 bytes)
{
    int32 nextStream = -1;
    SctpSendStream *stream;

    /* Only change stream if we don't have to finish a fragmented message */
    if (state->lastMsgWasFragment)
        nextStream = state->lastStreamScheduled;
    else
        nextStream = (this->*ssFunctions.ssGetNextSid)(path, true);

    if (nextStream == -1)
        return false;

    stream = sendStreams.find(nextStream)->second;

    if (stream) {
        cPacketQueue *streamQ = nullptr;

        if (!stream->getUnorderedStreamQ()->isEmpty())
            streamQ = stream->getUnorderedStreamQ();
        else if (!stream->getStreamQ()->isEmpty())
            streamQ = stream->getStreamQ();

        if (streamQ) {
            int32 b = ADD_PADDING(check_and_cast<SctpDataMsg *>(streamQ->front())->getEncapsulatedPacket()->getByteLength() + SCTP_DATA_CHUNK_LENGTH);

            /* Check if next message would be fragmented */
            if (b > (int32)state->fragPoint + (int32)SCTP_DATA_CHUNK_LENGTH) {
                /* Test if fragment fits */
                if (bytes >= (int32)state->fragPoint)
                    return true;
                else
                    return false;
            }

            /* Message doesn't need to be fragmented, just try if it fits */
            if (b <= bytes)
                return true;
            else
                return false;
        }
    }

    return false;
}

SctpPathVariables *SctpAssociation::getNextPath(const SctpPathVariables *oldPath) const
{
    int32 hit = 0;
    if (sctpPathMap.size() > 1) {
        for (const auto & elem : sctpPathMap) {
            SctpPathVariables *newPath = elem.second;
            if (newPath == oldPath) {
                if (++hit == 1) {
                    continue;
                }
                else {
                    break;
                }
            }
            if ((newPath->activePath) &&
                ((state->allowCMT == false) || (newPath->blockingTimeout <= 0.0) ||
                 (simTime() > newPath->blockingTimeout))) {
                return newPath;
            }
        }
    }
    return nullptr;
}

SctpPathVariables *SctpAssociation::getNextDestination(SctpDataVariables *chunk) const
{
    SctpPathVariables *next;

    EV_DEBUG << "Running getNextDestination()" << endl;
    if (chunk->numberOfTransmissions == 0) {
        if (chunk->getInitialDestinationPath() == nullptr) {
            next = state->getPrimaryPath();
        }
        else {
            next = chunk->getInitialDestinationPath();
        }
    }
    else {
        if (chunk->hasBeenFastRetransmitted) {
            EV_DETAIL << "Chunk " << chunk->tsn << " is scheduled for FastRetransmission. Next destination = "
                      << chunk->getLastDestination() << endl;
            return chunk->getLastDestinationPath();
        }
        // If this is a retransmission, we should choose another, active path.
        SctpPathVariables *last = chunk->getLastDestinationPath();
        next = getNextPath(last);
        if ((next == nullptr) || (next->confirmed == false)) {
            next = last;
        }
    }

    EV_INFO << "getNextDestination(): chunk was last sent to " << chunk->getLastDestination()
            << ", will next be sent to path " << next->remoteAddress << endl;
    return next;
}

void SctpAssociation::pmDataIsSentOn(SctpPathVariables *path)
{
    if ((!state->sendHeartbeatsOnActivePaths) || (!sctpMain->getEnableHeartbeats())) {
        /* restart hb_timer on this path */
        stopTimer(path->HeartbeatTimer);
        if (sctpMain->getEnableHeartbeats()) {
            path->heartbeatTimeout = path->pathRto + sctpMain->getHbInterval();
            startTimer(path->HeartbeatTimer, path->heartbeatTimeout);
            EV_DETAIL << "Restarting HB timer on path " << path->remoteAddress
                      << " to expire at time " << path->heartbeatTimeout << endl;
        }
    }

    path->cwndTimeout = path->pathRto;
    stopTimer(path->CwndTimer);
    startTimer(path->CwndTimer, path->cwndTimeout);

    EV_INFO << "Restarting CWND timer on path " << path->remoteAddress
            << " to expire at time " << path->cwndTimeout << endl;
}

void SctpAssociation::pmStartPathManagement()
{
    SctpPathVariables *path;
    const InterfaceEntry *rtie;
    int32 i = 0;
    /* populate path structures !!! */
    /* set a high start value...this is appropriately decreased later (below) */
    state->assocPmtu = state->localRwnd;
    for (auto & elem : sctpPathMap) {
        path = elem.second;
        path->pathErrorCount = 0;
        rtie = rt->getOutputInterfaceForDestination(path->remoteAddress);
        path->pmtu = rtie->getMtu();
        EV_DETAIL << "Path MTU of Interface " << i << " = " << path->pmtu << "\n";
        if (path->pmtu < state->assocPmtu) {
            state->assocPmtu = path->pmtu;
        }
        if (state->fragPoint > path->pmtu - IP_HEADER_LENGTH - SCTP_COMMON_HEADER - SCTP_DATA_CHUNK_LENGTH) {
            state->fragPoint = path->pmtu - IP_HEADER_LENGTH - SCTP_COMMON_HEADER - SCTP_DATA_CHUNK_LENGTH;
        }
        initCcParameters(path);
        path->pathRto = sctpMain->getRtoInitial();
        path->srtt = path->pathRto;
        path->rttvar = SIMTIME_ZERO;
        /* from now on we may have one update per RTO/SRTT */
        path->rttUpdateTime = SIMTIME_ZERO;

        path->partialBytesAcked = 0;
        path->outstandingBytes = 0;
        path->activePath = true;
        // Timer probably not running, but stop it anyway I.R.
        stopTimer(path->T3_RtxTimer);

        if (path->remoteAddress == state->initialPrimaryPath && !path->confirmed) {
            path->confirmed = true;
        }
        EV_DETAIL << getFullPath() << " numberOfLocalAddresses=" << state->localAddresses.size() << "\n";
        if (sctpMain->getEnableHeartbeats()) {
            path->heartbeatTimeout = sctpMain->getHbInterval() + i * path->pathRto;
            stopTimer(path->HeartbeatTimer);
            if (!path->confirmed)
                sendHeartbeat(path);
            startTimer(path->HeartbeatTimer, path->heartbeatTimeout);
            startTimer(path->HeartbeatIntervalTimer, path->heartbeatIntervalTimeout);
        }
        path->statisticsPathRTO->record(path->pathRto);
        i++;
    }
}

int32 SctpAssociation::getOutstandingBytes() const
{
    int32 osb = 0;
    for (const auto & elem : sctpPathMap) {
        osb += elem.second->outstandingBytes;
    }
    return osb;
}

void SctpAssociation::pmClearPathCounter(SctpPathVariables *path)
{
    state->errorCount = 0;
    path->pathErrorCount = 0;
    if (path->activePath == false) {
        /* notify the application */
        pathStatusIndication(path, true);
        EV_INFO << "Path " << path->remoteAddress
                << " state changes from INACTIVE to ACTIVE !!!" << endl;
        path->activePath = true;    // Mark path as active!
    }
}

void SctpAssociation::pathStatusIndication(const SctpPathVariables *path,
        const bool status)
{
    Indication *msg = new Indication("StatusInfo", SCTP_I_STATUS);
    SctpStatusReq *cmd = msg->addTag<SctpStatusReq>();
    cmd->setPathId(path->remoteAddress);
    cmd->setSocketId(assocId);
    cmd->setActive(status);
    if (!status) {
        auto iter = sctpMain->assocStatMap.find(assocId);
        iter->second.numPathFailures++;
    }
    sendToApp(msg);
}

void SctpAssociation::pmRttMeasurement(SctpPathVariables *path,
        const simtime_t& rttEstimation)
{
    if (rttEstimation < SIMTIME_MAX) {
        if (simTime() > path->rttUpdateTime) {
            if (path->rttUpdateTime == SIMTIME_ZERO) {
                path->rttvar = rttEstimation.dbl() / 2;
                path->srtt = rttEstimation;
                path->pathRto = 3.0 * rttEstimation.dbl();
                path->pathRto = max(min(path->pathRto.dbl(), sctpMain->getRtoMax()), sctpMain->getRtoMin());
            }
            else {
                path->rttvar = (1.0 - sctpMain->par("rtoBeta").doubleValue()) * path->rttvar.dbl()
                    + sctpMain->par("rtoBeta").doubleValue() * fabs(path->srtt.dbl() - rttEstimation.dbl());
                path->srtt = (1.0 - sctpMain->par("rtoAlpha").doubleValue()) * path->srtt.dbl()
                    + sctpMain->par("rtoAlpha").doubleValue() * rttEstimation.dbl();
                path->pathRto = path->srtt.dbl() + 4.0 * path->rttvar.dbl();
                path->pathRto = max(min(path->pathRto.dbl(), sctpMain->getRtoMax()), sctpMain->getRtoMin());
            }
            // RFC 2960, sect. 6.3.1: new RTT measurements SHOULD be made no more
            //                                than once per round-trip.
            path->rttUpdateTime = simTime() + path->srtt;
            path->statisticsPathRTO->record(path->pathRto);
            path->statisticsPathRTT->record(rttEstimation);
        }
    }
}

bool SctpAssociation::allPathsInactive() const
{
    for (const auto & elem : sctpPathMap) {
        if (elem.second->activePath) {
            return false;
        }
    }
    return true;
}

void SctpAssociation::removeFirstChunk(SctpHeader *sctpmsg)
{
    SctpChunk *chunk = sctpmsg->removeFirstChunk();
    delete chunk;
}

void SctpAssociation::disposeOf(SctpHeader *sctpmsg)
{
    SctpChunk *chunk;
    uint32 numberOfChunks = sctpmsg->getSctpChunksArraySize();
    if (numberOfChunks > 0)
        for (uint32 i = 0; i < numberOfChunks; i++) {
            chunk = sctpmsg->removeFirstChunk();
         /*   if (chunk->getSctpChunkType() == DATA) {
                delete chunk->Chunk::peek<SctpSimpleMessage>(Chunk::BackwardIterator(B(0)));
            }*/
            delete chunk;
        }
    delete sctpmsg;
}

int SctpAssociation::getAddressLevel(const L3Address& addr)
{
    if (addr.getType() == L3Address::IPv6) {
        switch (addr.toIpv6().getScope()) {
            case Ipv6Address::UNSPECIFIED:
            case Ipv6Address::MULTICAST:
                return 0;

            case Ipv6Address::LOOPBACK:
                return 1;

            case Ipv6Address::LINK:
                return 2;

            case Ipv6Address::SITE:
                return 3;

            case Ipv6Address::GLOBAL:
                return 4;

            default:
                throw cRuntimeError("Unknown IPv6 scope: %d", (int)(addr.toIpv6().getScope()));
        }
    }
    else if (addr.getType() == L3Address::IPv4) {
        switch (addr.toIpv4().getAddressCategory()) {
            case Ipv4Address::UNSPECIFIED:
            case Ipv4Address::THIS_NETWORK:
            case Ipv4Address::MULTICAST:
            case Ipv4Address::BROADCAST:
            case Ipv4Address::BENCHMARK:
            case Ipv4Address::IPv6_TO_IPv4_RELAY:
            case Ipv4Address::IETF:
            case Ipv4Address::TEST_NET:
            case Ipv4Address::RESERVED:
                return 0;

            case Ipv4Address::LOOPBACK:
                return 1;

            case Ipv4Address::LINKLOCAL:
                return 2;

            case Ipv4Address::PRIVATE_NETWORK:
                return 3;

            case Ipv4Address::GLOBAL:
                return 4;

            default:
                throw cRuntimeError("Unknown Ipv4 address category: %d", (int)(addr.toIpv4().getAddressCategory()));
        }
    }
    throw cRuntimeError("Unknown address type: %d", (int)(addr.getType()));
}

void SctpAssociation::putInTransmissionQ(const uint32 tsn, SctpDataVariables *chunk)
{
    if (chunk->countsAsOutstanding) {
        decreaseOutstandingBytes(chunk);
    }
    auto it = transmissionQ->payloadQueue.find(tsn);
    if (it == transmissionQ->payloadQueue.end()) {
        EV_DETAIL << "putInTransmissionQ: insert tsn=" << tsn << endl;
        chunk->wasDropped = true;
        chunk->wasPktDropped = true;
        chunk->hasBeenFastRetransmitted = true;
        chunk->setNextDestination(chunk->getLastDestinationPath());
        if (!transmissionQ->checkAndInsertChunk(chunk->tsn, chunk)) {
            EV_DETAIL << "putInTransmissionQ: cannot add message/chunk (TSN="
                      << tsn << ") to the transmissionQ" << endl;
        }
        else {
            chunk->enqueuedInTransmissionQ = true;
            auto q = qCounter.roomTransQ.find(chunk->getNextDestination());
            q->second += ADD_PADDING(chunk->len / 8 + SCTP_DATA_CHUNK_LENGTH);
            auto qb = qCounter.bookedTransQ.find(chunk->getNextDestination());
            qb->second += chunk->booksize;
            EV_DETAIL << "putInTransmissionQ: " << transmissionQ->getQueueSize() << " chunks="
                      << q->second << " bytes" << endl;
        }
    }
}

} // namespace sctp

} // namespace inet

