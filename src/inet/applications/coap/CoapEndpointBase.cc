//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/coap/CoapEndpointBase.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "inet/applications/coap/CoapHeaderSerializer.h"
#include "inet/applications/coap/CoapProtocol.h"
#include "inet/common/MemoryOutputStream.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/packet/chunk/SequenceChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

namespace inet {
namespace coap {

Define_Module(CoapEndpointBase);

simsignal_t CoapEndpointBase::retransmissionSignal = cComponent::registerSignal("retransmission");
simsignal_t CoapEndpointBase::duplicateReceivedSignal = cComponent::registerSignal("duplicateReceived");
simsignal_t CoapEndpointBase::malformedReceivedSignal = cComponent::registerSignal("malformedReceived");
simsignal_t CoapEndpointBase::capacityRejectedSignal = cComponent::registerSignal("capacityRejected");

CoapEndpointBase::~CoapEndpointBase()
{
    cancelAndDelete(deadlineTimer);
    deadlineTimer = nullptr;
#ifdef INET_WITH_DTLS
    cancelAndDelete(dtlsSendErrorTimer);
    dtlsSendErrorTimer = nullptr;
#endif
}

void CoapEndpointBase::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        parseAndValidateParameters();
        initializeCountersOnce();
        if (deadlineTimer == nullptr)
            deadlineTimer = new cMessage("deadlineTimer");
#ifdef INET_WITH_DTLS
        if (dtlsSendErrorTimer == nullptr)
            dtlsSendErrorTimer = new cMessage("dtlsSendErrorTimer");
#endif
    }
}

void CoapEndpointBase::parseAndValidateParameters()
{
    dtlsModulePath = par("dtlsModule").stdstringValue();
    localPort = par("localPort");
    timeToLive = par("timeToLive");
    dscp = par("dscp");
    tos = par("tos");
    rngStream = par("rngStream");

    ackTimeout = par("ackTimeout");
    ackRandomFactor = par("ackRandomFactor");
    maxRetransmit = par("maxRetransmit");
    nstart = par("nstart");
    double probingRateBps = par("probingRate");
    probingRateBytesPerSec = probingRateBps / 8.0;
    maxLatency = par("maxLatency");
    processingDelay = par("processingDelay");

    maxPendingRequests = par("maxPendingRequests");
    maxPendingOutboundPerEndpoint = par("maxPendingOutboundPerEndpoint");
    maxIncomingBlockSize = par("maxIncomingBlockSize");
    preferredBlockSize = par("preferredBlockSize");
    maxIncomingBlockTransfers = par("maxIncomingBlockTransfers");
    maxObservers = par("maxObservers");

    if (rngStream < 0)
        throw cRuntimeError("rngStream must be non-negative");
    if (ackTimeout <= SIMTIME_ZERO)
        throw cRuntimeError("ackTimeout must be positive");
    if (ackRandomFactor < 1.0)
        throw cRuntimeError("ackRandomFactor must be >= 1.0");
    if (maxRetransmit < 0 || maxRetransmit > 16)
        throw cRuntimeError("maxRetransmit must be in [0, 16]");
    if (nstart < 1)
        throw cRuntimeError("nstart must be >= 1");
    if (probingRateBytesPerSec <= 0.0)
        throw cRuntimeError("probingRate must be positive");
    if (maxLatency <= SIMTIME_ZERO)
        throw cRuntimeError("maxLatency must be positive");
    if (processingDelay < SIMTIME_ZERO)
        throw cRuntimeError("processingDelay must be non-negative");
    if (maxPendingRequests < 1 || maxPendingOutboundPerEndpoint < 1)
        throw cRuntimeError("Queue and pending capacities must be >= 1");
    if (maxIncomingBlockSize < 16 || maxIncomingBlockTransfers < 1 || maxObservers < 1)
        throw cRuntimeError("Block and observer capacities must be valid positive values");
    bool validPrefBlock = (preferredBlockSize == 16 || preferredBlockSize == 32 || preferredBlockSize == 64 ||
            preferredBlockSize == 128 || preferredBlockSize == 256 || preferredBlockSize == 512 || preferredBlockSize == 1024);
    if (!validPrefBlock)
        throw cRuntimeError("preferredBlockSize must be a power of two in [16, 1024], got %d", preferredBlockSize);

    double spanFactor = (maxRetransmit == 0) ? 0.0 : static_cast<double>((1U << maxRetransmit) - 1U) * ackRandomFactor;
    double waitFactor = static_cast<double>((1U << (maxRetransmit + 1)) - 1U) * ackRandomFactor;
    maxTransmitSpan = ackTimeout * spanFactor;
    maxTransmitWait = ackTimeout * waitFactor;
    exchangeLifetime = maxTransmitSpan + 2.0 * maxLatency + processingDelay;
    nonLifetime = maxTransmitSpan + maxLatency;
}

void CoapEndpointBase::initializeCountersOnce()
{
    if (generatorsInitialized)
        return;

    cRNG *rng = getRNG(rngStream);
    uint64_t seedToken = 0;
    for (int i = 0; i < 4; ++i)
        seedToken = (seedToken << 16U) | static_cast<uint64_t>(rng->intRand(65536));

    tokenCounter = seedToken;
    tokenRemaining = std::numeric_limits<uint64_t>::max();
    nextMessageId = static_cast<uint16_t>(rng->intRand(65536));
    generatorsInitialized = true;
}

void CoapEndpointBase::bindSocket()
{
    if (isDtlsMode()) {
#ifdef INET_WITH_DTLS
        auto *engine = getModuleFromPar<dtls::IDtls>(par("dtlsModule"), this);
        dtlsSocket.setEngine(engine);
        dtlsSocket.setCallback(this);
        dtlsBound = true;
        return;
#else
        throw cRuntimeError("CoapEndpointBase dtlsModule='%s' requires the Dtls feature", dtlsModulePath.c_str());
#endif
    }

    const char *localAddrStr = par("localAddress");
    localAddress = (localAddrStr != nullptr && localAddrStr[0] != '\0')
            ? L3AddressResolver().resolve(localAddrStr)
            : L3Address();

    socket.setOutputGate(gate("socketOut"));
    socket.setCallback(this);
    socket.bind(localAddress, localPort);

    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);
    if (dscp != -1)
        socket.setDscp(dscp);
    if (tos != -1)
        socket.setTos(tos);
}

void CoapEndpointBase::clearTransientState(bool clearDuplicates)
{
    auto pendingSnapshot = std::move(pendingRequests);
    pendingRequests.clear();
    outboundMidToToken.clear();
    activeExchanges.clear();
    peerCongestion.clear();
    outboundQueue.clear();
#ifdef INET_WITH_DTLS
    dtlsInFlightPackets.clear();
#endif

    if (clearDuplicates) {
        conDuplicateOrder.clear();
        nonDuplicateOrder.clear();
        duplicateEntries.clear();
    }

    for (const auto& kv : pendingSnapshot)
        dispatchRequestAborted(kv.second.requestId);
}

void CoapEndpointBase::handleMessageWhenUp(cMessage *msg)
{
    if (msg == deadlineTimer) {
        onDeadlineTimer();
    }
#ifdef INET_WITH_DTLS
    else if (msg == dtlsSendErrorTimer) {
        processPendingDtlsSendErrors();
    }
#endif
    else if (msg->isSelfMessage()) {
        handleSubclassSelfMessage(msg);
    }
    else if (socket.belongsToSocket(msg)) {
        socket.processMessage(msg);
    }
    else {
        throw cRuntimeError("Unknown message arrived at CoapEndpointBase: %s", msg->getName());
    }
}

void CoapEndpointBase::handleSubclassSelfMessage(cMessage *msg)
{
    throw cRuntimeError("Unhandled self-message: %s", msg->getName());
}

void CoapEndpointBase::handleStartOperation(LifecycleOperation *operation)
{
    bindSocket();
    onEndpointStart();
    rescheduleDeadlineTimer();
}

void CoapEndpointBase::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(deadlineTimer);
#ifdef INET_WITH_DTLS
    cancelEvent(dtlsSendErrorTimer);
    pendingDtlsSendErrors.clear();
#endif
    clearTransientState(false);
    onEndpointStop();
#ifdef INET_WITH_DTLS
    if (isDtlsMode()) {
        std::vector<dtls::IDtls::AssociationId> sessionIds;
        sessionIds.reserve(dtlsSessions.size());
        for (const auto& kv : dtlsSessions)
            sessionIds.push_back(kv.first);
        dtlsSessions.clear();
        activeClientDtlsByAddrPort.clear();
        dtlsInFlightPackets.clear();
        conDuplicateOrder.clear();
        nonDuplicateOrder.clear();
        duplicateEntries.clear();
        if (dtlsBound) {
            for (auto id : sessionIds)
                dtlsSocket.close(id);
            dtlsBound = false;
        }
        return;
    }
#endif
    if (socket.isOpen()) {
        socket.close();
        delayActiveOperationFinish(par("stopOperationTimeout"));
    }
}

void CoapEndpointBase::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(deadlineTimer);
#ifdef INET_WITH_DTLS
    cancelEvent(dtlsSendErrorTimer);
    pendingDtlsSendErrors.clear();
#endif
    clearTransientState(true);
    onEndpointCrash();
#ifdef INET_WITH_DTLS
    if (isDtlsMode()) {
        dtlsSessions.clear();
        activeClientDtlsByAddrPort.clear();
        dtlsInFlightPackets.clear();
        dtlsBound = false;
        return;
    }
#endif
    if (operation->getRootModule() != getContainingNode(this))
        socket.destroy();
    else
        socket.setCallback(nullptr);
}

void CoapEndpointBase::socketClosed(UdpSocket *closedSocket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void CoapEndpointBase::socketErrorArrived(UdpSocket *errSocket, Indication *indication)
{
    EV_WARN << "UDP error indication received: " << indication->getName() << endl;
    delete indication;
}

void CoapEndpointBase::finish()
{
    ApplicationBase::finish();
}

void CoapEndpointBase::rescheduleDeadlineTimer()
{
    if (deadlineTimer == nullptr || operationalState != State::OPERATING)
        return;

    cancelEvent(deadlineTimer);

    simtime_t now = simTime();
    simtime_t nextDeadline = SIMTIME_ZERO;
    auto considerDeadline = [&](simtime_t t) {
        if (t > SIMTIME_ZERO && (nextDeadline == SIMTIME_ZERO || t < nextDeadline))
            nextDeadline = t;
    };

    for (const auto& kv : pendingRequests)
        considerDeadline(kv.second.expiresAt);

    for (const auto& kv : activeExchanges)
        considerDeadline(kv.second.nextRetryAt);

    for (const auto& kv : peerCongestion) {
        if (kv.second.probingEarliestAt > now)
            considerDeadline(kv.second.probingEarliestAt);
    }

    if (midPoolExhausted && !midReservationOrder.empty())
        considerDeadline(midReservationOrder.front().second);

    if (!conDuplicateOrder.empty()) {
        auto it = duplicateEntries.find(conDuplicateOrder.front());
        if (it != duplicateEntries.end())
            considerDeadline(it->second.expiry);
    }

    if (!nonDuplicateOrder.empty()) {
        auto it = duplicateEntries.find(nonDuplicateOrder.front());
        if (it != duplicateEntries.end())
            considerDeadline(it->second.expiry);
    }

    considerDeadline(getNextSubclassDeadline());

    if (nextDeadline > SIMTIME_ZERO)
        scheduleAt(std::max(nextDeadline, now), deadlineTimer);
}

void CoapEndpointBase::onDeadlineTimer()
{
    simtime_t now = simTime();
    purgeExpiredMidReservations(now);
    purgeExpiredDuplicates(now);
    processConfirmableRetransmissions(now);
    pumpOutboundQueue(now);
    expirePendingRequests(now);
    onSubclassTimer(now);
    rescheduleDeadlineTimer();
}

void CoapEndpointBase::configureTokenGeneratorForTest(uint64_t initialValue, uint64_t remainingCount)
{
    generatorsInitialized = true;
    tokenCounter = initialValue;
    tokenRemaining = remainingCount;
}

void CoapEndpointBase::configureMidGeneratorForTest(uint16_t initialMid, bool reserveAll, simtime_t reserveDuration)
{
    generatorsInitialized = true;
    nextMessageId = initialMid;
    if (reserveAll) {
        simtime_t expiry = simTime() + (reserveDuration > SIMTIME_ZERO ? reserveDuration : exchangeLifetime);
        midReservationOrder.clear();
        reservedOutgoingMids.clear();
        for (uint32_t i = 0; i < 65536U; ++i) {
            uint16_t mid = static_cast<uint16_t>(i);
            reservedOutgoingMids[mid] = expiry;
            midReservationOrder.emplace_back(mid, expiry);
        }
    }
}

void CoapEndpointBase::purgeExpiredMidReservations(simtime_t now)
{
    while (!midReservationOrder.empty()) {
        const auto& front = midReservationOrder.front();
        if (front.second > now)
            break;
        auto it = reservedOutgoingMids.find(front.first);
        if (it != reservedOutgoingMids.end() && it->second <= now) {
            reservedOutgoingMids.erase(it);
            midPoolExhausted = false;
        }
        midReservationOrder.pop_front();
    }
}

bool CoapEndpointBase::allocateToken(std::vector<uint8_t>& tokenOut)
{
    if (tokenRemaining == 0) {
        emit(capacityRejectedSignal, 1L);
        return false;
    }

    uint64_t value = tokenCounter;
    std::vector<uint8_t> candidate(8);
    for (int i = 7; i >= 0; --i) {
        candidate[i] = static_cast<uint8_t>(value & 0xFFU);
        value >>= 8U;
    }

    if (pendingRequests.find(candidate) != pendingRequests.end()) {
        emit(capacityRejectedSignal, 1L);
        return false;
    }

    ++tokenCounter;
    --tokenRemaining;
    tokenOut = std::move(candidate);
    return true;
}

bool CoapEndpointBase::allocateMessageId(uint16_t& midOut, bool emitOnFailure)
{
    simtime_t now = simTime();
    purgeExpiredMidReservations(now);

    for (uint32_t offset = 0; offset < 65536U; ++offset) {
        uint16_t candidate = static_cast<uint16_t>(nextMessageId + offset);
        auto it = reservedOutgoingMids.find(candidate);
        if (it == reservedOutgoingMids.end() || it->second <= now) {
            simtime_t expiry = now + exchangeLifetime;
            reservedOutgoingMids[candidate] = expiry;
            midReservationOrder.emplace_back(candidate, expiry);
            nextMessageId = static_cast<uint16_t>(candidate + 1U);
            midOut = candidate;
            midPoolExhausted = false;
            return true;
        }
    }

    if (emitOnFailure)
        emit(capacityRejectedSignal, 1L);
    return false;
}

Packet *CoapEndpointBase::buildCoapPacket(const char *name, const Ptr<CoapHeader>& header, const std::vector<uint8_t>& payload) const
{
    finalizeCoapHeader(header, !payload.empty());
    Packet *packet = new Packet(name);
    packet->insertAtBack(header);
    if (!payload.empty()) {
        auto payloadChunk = makeShared<BytesChunk>(payload);
        packet->insertAtBack(payloadChunk);
    }
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&getCoapProtocol());
    return packet;
}

bool CoapEndpointBase::parseInboundDatagram(Packet *packet, Ptr<CoapHeader>& headerOut, std::vector<uint8_t>& payloadOut) const
{
    headerOut = nullptr;
    payloadOut.clear();

    if (packet == nullptr || packet->getDataLength() == B(0))
        return false;

    const auto& rootChunk = packet->peekData();
    if (auto typedHeader = dynamicPtrCast<const CoapHeader>(rootChunk)) {
        headerOut = makeShared<CoapHeader>(*typedHeader);
        return validateCoapHeaderStructure(*headerOut, 0);
    }

    if (auto seqChunk = dynamicPtrCast<const SequenceChunk>(rootChunk)) {
        const auto& subchunks = seqChunk->getChunks();
        if (!subchunks.empty()) {
            if (auto typedHeader = dynamicPtrCast<const CoapHeader>(subchunks.front())) {
                headerOut = makeShared<CoapHeader>(*typedHeader);
                MemoryOutputStream payloadStream;
                for (size_t i = 1; i < subchunks.size(); ++i)
                    Chunk::serialize(payloadStream, subchunks[i]);
                payloadOut = payloadStream.getData();
                return validateCoapHeaderStructure(*headerOut, payloadOut.size());
            }
        }
    }

    const auto& bytesChunk = packet->peekDataAsBytes();
    const auto& rawBytes = bytesChunk->getBytes();
    return parseCoapDatagramBytes(rawBytes, headerOut, payloadOut);
}

void CoapEndpointBase::sendCoapPacket(Packet *packet, const L3Address& destAddr, int destPort)
{
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&getCoapProtocol());
    emit(packetSentSignal, packet);
    emit(packetSentToLowerSignal, packet);
    socket.sendTo(packet, destAddr, destPort);
}

void CoapEndpointBase::sendCoapPacketToPeer(const CoapPeerEndpoint& peer, Packet *packet,
        uint64_t outboundId, uint64_t requestId, const std::vector<uint8_t>& token,
        uint16_t messageId, bool isConfirmable, bool isRequest)
{
#ifdef INET_WITH_DTLS
    if (peer.sessionId != 0) {
        if (!dtlsBound || dtlsSessions.find(peer.sessionId) == dtlsSessions.end()) {
            delete packet;
            return;
        }
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&getCoapProtocol());
        emit(packetSentSignal, packet);
        emit(packetSentToLowerSignal, packet);
        // Remove CoAP PacketProtocolTag so Dtls::sendApplication tags the outer record as dtlsProtocol.
        packet->removeTagIfPresent<PacketProtocolTag>();
        if (outboundId != 0 || requestId != 0) {
            CoapDtlsInFlightPacket info;
            info.sessionId = peer.sessionId;
            info.peer = peer;
            info.outboundId = outboundId;
            info.requestId = requestId;
            info.token = token;
            info.messageId = messageId;
            info.isConfirmable = isConfirmable;
            info.isRequest = isRequest;
            dtlsInFlightPackets[packet->getId()] = std::move(info);
        }
        struct ScopedFlagReset {
            bool& flag;
            ~ScopedFlagReset() { flag = false; }
        };
        insideDtlsSend = true;
        ScopedFlagReset resetFlag{insideDtlsSend};
        dtlsSocket.send(peer.sessionId, packet);
        return;
    }
#endif
    sendCoapPacket(packet, peer.address, peer.port);
}

void CoapEndpointBase::sendEmptyMessage(CoapMessageType type, uint16_t messageId, const L3Address& destAddr, int destPort)
{
    sendEmptyMessageToPeer(type, messageId, CoapPeerEndpoint{destAddr, destPort, 0});
}

void CoapEndpointBase::sendEmptyMessageToPeer(CoapMessageType type, uint16_t messageId, const CoapPeerEndpoint& peer)
{
    auto header = makeShared<CoapHeader>();
    header->setVersion(1);
    header->setMessageType(type);
    header->setCode(COAP_CODE_EMPTY);
    header->setMessageId(messageId);
    const char *pktName = (type == COAP_TYPE_RST) ? "CoapReset"
            : (type == COAP_TYPE_ACK ? "CoapAck" : "CoapEmpty");
    std::vector<uint8_t> emptyPayload;
    Packet *packet = buildCoapPacket(pktName, header, emptyPayload);
    sendCoapPacketToPeer(peer, packet);
}

void CoapEndpointBase::sendEmptyMessageAndCacheDuplicate(CoapMessageType type, uint16_t messageId, const CoapPeerEndpoint& peer)
{
    auto header = makeShared<CoapHeader>();
    header->setVersion(1);
    header->setMessageType(type);
    header->setCode(COAP_CODE_EMPTY);
    header->setMessageId(messageId);
    const char *pktName = (type == COAP_TYPE_RST) ? "CoapReset"
            : (type == COAP_TYPE_ACK ? "CoapAck" : "CoapEmpty");
    std::vector<uint8_t> emptyPayload;
    Packet *packet = buildCoapPacket(pktName, header, emptyPayload);
    cacheDuplicateResponse(peer, messageId, packet);
    sendCoapPacketToPeer(peer, packet);
}

void CoapEndpointBase::markPeerReachable(const CoapPeerEndpoint& peer)
{
    auto it = peerCongestion.find(peer);
    if (it != peerCongestion.end()) {
        it->second.inProbingBackoff = false;
        it->second.probingEarliestAt = SIMTIME_ZERO;
    }
}

bool CoapEndpointBase::submitOutboundMessage(const CoapPeerEndpoint& peer, bool peerIsMulticast, bool confirmable,
        bool isRequest, uint64_t requestId, const std::vector<uint8_t>& token, const char *packetName,
        const Ptr<CoapHeader>& header, const std::vector<uint8_t>& payload,
        bool refreshObserveOnRetry, uint64_t *outboundIdOut)
{
    if (peerIsMulticast) {
        uint16_t mid = 0;
        if (!allocateMessageId(mid, true))
            return false;
        header->setMessageId(mid);
        if (isRequest) {
            auto reqIt = pendingRequests.find(token);
            if (reqIt != pendingRequests.end()) {
                if (reqIt->second.sentMids.empty())
                    reqIt->second.initialMid = mid;
                reqIt->second.sentMids.insert(mid);
            }
        }
        Packet *packet = buildCoapPacket(packetName, header, payload);
        sendCoapPacketToPeer(peer, packet);
        rescheduleDeadlineTimer();
        if (outboundIdOut != nullptr)
            *outboundIdOut = 0;
        return true;
    }

    if (static_cast<int>(outboundQueue.size()) >= maxPendingOutboundPerEndpoint) {
        emit(capacityRejectedSignal, 1L);
        return false;
    }

    CoapOutboundItem item;
    item.outboundId = nextOutboundId++;
    item.peer = peer;
    item.peerIsMulticast = false;
    item.confirmable = confirmable;
    item.isRequest = isRequest;
    item.refreshObserveOnRetry = refreshObserveOnRetry;
    item.requestId = requestId;
    item.token = token;
    item.packetName = packetName;
    item.header = header;
    item.payload = payload;
    if (outboundIdOut != nullptr)
        *outboundIdOut = item.outboundId;
    outboundQueue.push_back(std::move(item));

    pumpOutboundQueue(simTime());
    rescheduleDeadlineTimer();
    return true;
}

bool CoapEndpointBase::sendConfirmablePing(const CoapPeerEndpoint& peer, uint64_t& exchangeIdOut)
{
    auto header = makeShared<CoapHeader>();
    header->setVersion(1);
    header->setMessageType(COAP_TYPE_CON);
    header->setCode(COAP_CODE_EMPTY);
    std::vector<uint8_t> emptyToken;
    std::vector<uint8_t> emptyPayload;
    uint64_t expectedExchange = nextExchangeId;
    if (!submitOutboundMessage(peer, false, true, false, 0, emptyToken, "CoapPing", header, emptyPayload))
        return false;
    exchangeIdOut = expectedExchange;
    return true;
}

void CoapEndpointBase::pumpOutboundQueue(simtime_t now)
{
    if (operationalState != State::OPERATING || outboundQueue.empty())
        return;

    std::set<CoapPeerEndpoint> blockedPeers;
    for (size_t idx = 0; idx < outboundQueue.size();) {
        const auto& item = outboundQueue[idx];
        if (blockedPeers.count(item.peer) > 0) {
            ++idx;
            continue;
        }
#ifdef INET_WITH_DTLS
        if (item.peer.sessionId != 0) {
            auto sessIt = dtlsSessions.find(item.peer.sessionId);
            if (sessIt == dtlsSessions.end() || !sessIt->second.established) {
                blockedPeers.insert(item.peer);
                ++idx;
                continue;
            }
        }
#endif

        auto& peerState = peerCongestion[item.peer];
        if (peerState.inProbingBackoff && peerState.probingEarliestAt > now) {
            blockedPeers.insert(item.peer);
            ++idx;
            continue;
        }
        if (item.confirmable && peerState.activeConfirmables >= nstart) {
            blockedPeers.insert(item.peer);
            ++idx;
            continue;
        }

        uint16_t mid = 0;
        if (!allocateMessageId(mid, false)) {
            midPoolExhausted = true;
            break;
        }

        CoapOutboundItem dispatched = std::move(outboundQueue[idx]);
        outboundQueue.erase(outboundQueue.begin() + static_cast<std::ptrdiff_t>(idx));

        dispatched.header->setMessageId(mid);
        if (dispatched.isRequest) {
            auto reqIt = pendingRequests.find(dispatched.token);
            if (reqIt != pendingRequests.end()) {
                if (reqIt->second.sentMids.empty())
                    reqIt->second.initialMid = mid;
                reqIt->second.sentMids.insert(mid);
                outboundMidToToken[std::make_pair(dispatched.peer, mid)] = dispatched.token;
            }
        }

        Packet *pkt = buildCoapPacket(dispatched.packetName.c_str(), dispatched.header, dispatched.payload);
        auto wireBytesChunk = pkt->peekDataAsBytes();
        std::vector<uint8_t> wireBytes = wireBytesChunk->getBytes();

        if (peerState.inProbingBackoff) {
            simtime_t pacing = SimTime(static_cast<double>(wireBytes.size()) / probingRateBytesPerSec);
            peerState.probingEarliestAt = now + pacing;
        }

        if (dispatched.confirmable) {
            cRNG *rng = getRNG(rngStream);
            double randFactor = 1.0 + rng->doubleRand() * (ackRandomFactor - 1.0);
            simtime_t initTimeout = ackTimeout * randFactor;

            CoapOutboundExchange ex;
            ex.exchangeId = nextExchangeId++;
            ex.outboundId = dispatched.outboundId;
            ex.peer = dispatched.peer;
            ex.messageId = mid;
            ex.isRequest = dispatched.isRequest;
            ex.refreshObserveOnRetry = dispatched.refreshObserveOnRetry;
            ex.requestId = dispatched.requestId;
            ex.token = dispatched.token;
            ex.packetName = dispatched.packetName;
            if (dispatched.refreshObserveOnRetry) {
                ex.header = dispatched.header;
                ex.payload = dispatched.payload;
            }
            ex.wireBytes = wireBytes;
            ex.initialTimeout = initTimeout;
            ex.currentTimeout = initTimeout;
            ex.nextRetryAt = now + initTimeout;
            ex.retransmitCount = 0;

            activeExchanges[std::make_pair(dispatched.peer, mid)] = std::move(ex);
            peerState.activeConfirmables += 1;
        }

        sendCoapPacketToPeer(dispatched.peer, pkt, dispatched.outboundId, dispatched.requestId,
                dispatched.token, mid, dispatched.confirmable, dispatched.isRequest);
    }
}

void CoapEndpointBase::processConfirmableRetransmissions(simtime_t now)
{
    std::vector<std::pair<CoapPeerEndpoint, uint16_t>> dueKeys;
    for (const auto& kv : activeExchanges) {
        if (kv.second.nextRetryAt <= now)
            dueKeys.push_back(kv.first);
    }

    for (const auto& key : dueKeys) {
        auto it = activeExchanges.find(key);
        if (it == activeExchanges.end())
            continue;

        CoapOutboundExchange& ex = it->second;
        if (ex.retransmitCount < maxRetransmit) {
            ex.retransmitCount += 1;
            ex.currentTimeout *= 2.0;
            ex.nextRetryAt = now + ex.currentTimeout;

            Packet *retry = nullptr;
            if (ex.refreshObserveOnRetry && ex.header != nullptr) {
                uint32_t tick24 = static_cast<uint32_t>(
                        (now > SIMTIME_ZERO ? static_cast<uint64_t>(now.inUnit(SIMTIME_MS)) : 0ULL) & 0xFFFFFFULL);
                auto refreshedHdr = makeShared<CoapHeader>(*ex.header);
                setCoapOptionUint(refreshedHdr, COAP_OPT_OBSERVE, tick24);
                ex.header = refreshedHdr;
                retry = buildCoapPacket(ex.packetName.c_str(), refreshedHdr, ex.payload);
                auto chunk = retry->peekDataAsBytes();
                ex.wireBytes = chunk->getBytes();
            }
            else {
                retry = new Packet(ex.packetName.c_str());
                retry->insertAtBack(makeShared<BytesChunk>(ex.wireBytes));
            }
            emit(retransmissionSignal, 1L);
            sendCoapPacketToPeer(ex.peer, retry, ex.outboundId, ex.requestId,
                    ex.token, ex.messageId, true, ex.isRequest);
        }
        else {
            CoapOutboundExchange timedOut = std::move(it->second);
            activeExchanges.erase(it);
            eraseDtlsInFlight(timedOut.outboundId, timedOut.isRequest ? timedOut.requestId : 0);

            auto& peerState = peerCongestion[timedOut.peer];
            peerState.activeConfirmables = std::max(0, peerState.activeConfirmables - 1);
            peerState.inProbingBackoff = true;
            simtime_t probeWait = SimTime(static_cast<double>(timedOut.wireBytes.size()) / probingRateBytesPerSec);
            peerState.probingEarliestAt = std::max(peerState.probingEarliestAt, now + probeWait);

            onOutboundItemFailed(timedOut.outboundId, "CoAP confirmable exchange timed out");
            if (timedOut.isRequest) {
                bool stillPending = (pendingRequests.find(timedOut.token) != pendingRequests.end());
                removePendingRequest(timedOut.token);
                if (stillPending)
                    dispatchRequestFailed(timedOut.requestId, "CoAP confirmable exchange timed out");
            }
            else {
                onOutboundExchangeFailed(timedOut.exchangeId, timedOut.peer, timedOut.messageId,
                        "CoAP confirmable exchange timed out");
            }
        }
    }
}

void CoapEndpointBase::purgeExpiredDuplicates(simtime_t now)
{
    auto purgeDeque = [&](std::deque<CoapDuplicateKey>& order) {
        while (!order.empty()) {
            const auto& key = order.front();
            auto it = duplicateEntries.find(key);
            if (it == duplicateEntries.end()) {
                order.pop_front();
                continue;
            }
            if (it->second.expiry > now)
                break;
            duplicateEntries.erase(it);
            order.pop_front();
        }
    };
    purgeDeque(conDuplicateOrder);
    purgeDeque(nonDuplicateOrder);
}

bool CoapEndpointBase::checkAndHandleDuplicate(const CoapPeerEndpoint& peer, uint16_t messageId, CoapMessageType type)
{
    if (type != COAP_TYPE_CON && type != COAP_TYPE_NON)
        return false;

    simtime_t now = simTime();
    purgeExpiredDuplicates(now);

    CoapDuplicateKey key{peer, messageId};
    auto it = duplicateEntries.find(key);
    if (it == duplicateEntries.end() || it->second.expiry <= now)
        return false;

    emit(duplicateReceivedSignal, 1L);
    if (type == COAP_TYPE_CON && it->second.hasCachedResponse && !it->second.cachedWireBytes.empty()) {
        Packet *replay = new Packet("CoapCachedAck");
        replay->insertAtBack(makeShared<BytesChunk>(it->second.cachedWireBytes));
        sendCoapPacketToPeer(peer, replay);
    }
    return true;
}

void CoapEndpointBase::recordDuplicateEntry(const CoapPeerEndpoint& peer, uint16_t messageId, bool isConfirmable, simtime_t lifetime)
{
    simtime_t now = simTime();
    purgeExpiredDuplicates(now);

    CoapDuplicateKey key{peer, messageId};
    auto existingIt = duplicateEntries.find(key);
    if (existingIt != duplicateEntries.end()) {
        existingIt->second.expiry = now + lifetime;
        existingIt->second.isConfirmable = isConfirmable;
        return;
    }

    CoapDuplicateEntry entry;
    entry.key = key;
    entry.expiry = now + lifetime;
    entry.isConfirmable = isConfirmable;
    duplicateEntries[key] = entry;
    if (isConfirmable)
        conDuplicateOrder.push_back(key);
    else
        nonDuplicateOrder.push_back(key);
}

void CoapEndpointBase::cacheDuplicateResponse(const CoapPeerEndpoint& peer, uint16_t messageId, Packet *responsePacket)
{
    if (responsePacket == nullptr)
        return;
    auto wireBytesChunk = responsePacket->peekDataAsBytes();
    std::vector<uint8_t> wireBytes = wireBytesChunk->getBytes();
    CoapDuplicateKey key{peer, messageId};
    auto it = duplicateEntries.find(key);
    if (it != duplicateEntries.end()) {
        it->second.hasCachedResponse = true;
        it->second.cachedWireBytes = std::move(wireBytes);
    }
}

bool CoapEndpointBase::registerPendingRequest(const CoapPeerEndpoint& peer, bool peerIsMulticast, bool confirmable,
        uint8_t requestCode, const std::vector<uint8_t>& token, uint64_t& requestIdOut)
{
    if (static_cast<int>(pendingRequests.size()) >= maxPendingRequests) {
        emit(capacityRejectedSignal, 1L);
        return false;
    }

    if (pendingRequests.find(token) != pendingRequests.end()) {
        emit(capacityRejectedSignal, 1L);
        return false;
    }

    CoapPendingRequest req;
    req.requestId = nextRequestId++;
    req.peer = peer;
    req.peerIsMulticast = peerIsMulticast;
    req.confirmable = confirmable;
    req.requestCode = requestCode;
    req.token = token;
    req.createdAt = simTime();
    req.expiresAt = simTime() + exchangeLifetime;

    pendingRequests[token] = req;
    requestIdOut = req.requestId;
    rescheduleDeadlineTimer();
    return true;
}

bool CoapEndpointBase::hasPendingRequest(uint64_t requestId) const
{
    for (const auto& kv : pendingRequests) {
        if (kv.second.requestId == requestId)
            return true;
    }
    return false;
}

void CoapEndpointBase::removePendingRequest(const std::vector<uint8_t>& token)
{
    auto it = pendingRequests.find(token);
    if (it == pendingRequests.end())
        return;

    eraseDtlsInFlight(0, it->second.requestId);
    for (uint16_t mid : it->second.sentMids)
        outboundMidToToken.erase(std::make_pair(it->second.peer, mid));

    pendingRequests.erase(it);
    rescheduleDeadlineTimer();
}

void CoapEndpointBase::cancelConfirmableExchangesForRequest(uint64_t requestId)
{
    if (requestId == 0)
        return;
    bool releasedAny = false;
    for (auto it = activeExchanges.begin(); it != activeExchanges.end();) {
        if (it->second.isRequest && it->second.requestId == requestId) {
            auto peerIt = peerCongestion.find(it->first.first);
            if (peerIt != peerCongestion.end())
                peerIt->second.activeConfirmables = std::max(0, peerIt->second.activeConfirmables - 1);
            eraseDtlsInFlight(it->second.outboundId, requestId);
            it = activeExchanges.erase(it);
            releasedAny = true;
        }
        else {
            ++it;
        }
    }
    if (releasedAny)
        pumpOutboundQueue(simTime());
}

void CoapEndpointBase::expirePendingRequests(simtime_t now)
{
    std::vector<std::pair<uint64_t, std::vector<uint8_t>>> expired;
    for (const auto& kv : pendingRequests) {
        if (kv.second.expiresAt <= now)
            expired.emplace_back(kv.second.requestId, kv.first);
    }

    for (const auto& item : expired) {
        cancelConfirmableExchangesForRequest(item.first);
        removePendingRequest(item.second);
        dispatchRequestFailed(item.first, "CoAP request timed out");
    }
}

bool CoapEndpointBase::isRecognizedRequestOption(uint16_t optionNumber) const
{
    switch (optionNumber) {
        case COAP_OPT_IF_MATCH:
        case COAP_OPT_URI_HOST:
        case COAP_OPT_ETAG:
        case COAP_OPT_IF_NONE_MATCH:
        case COAP_OPT_OBSERVE:
        case COAP_OPT_URI_PORT:
        case COAP_OPT_URI_PATH:
        case COAP_OPT_CONTENT_FORMAT:
        case COAP_OPT_MAX_AGE:
        case COAP_OPT_URI_QUERY:
        case COAP_OPT_ACCEPT:
        case COAP_OPT_BLOCK2:
        case COAP_OPT_BLOCK1:
        case COAP_OPT_SIZE2:
        case COAP_OPT_PROXY_URI:
        case COAP_OPT_PROXY_SCHEME:
        case COAP_OPT_SIZE1:
        case COAP_OPT_REQUEST_TAG:
            return true;
        default:
            return false;
    }
}

bool CoapEndpointBase::isRecognizedResponseOption(uint16_t optionNumber) const
{
    switch (optionNumber) {
        case COAP_OPT_ETAG:
        case COAP_OPT_OBSERVE:
        case COAP_OPT_LOCATION_PATH:
        case COAP_OPT_LOCATION_QUERY:
        case COAP_OPT_CONTENT_FORMAT:
        case COAP_OPT_MAX_AGE:
        case COAP_OPT_BLOCK2:
        case COAP_OPT_BLOCK1:
        case COAP_OPT_SIZE2:
        case COAP_OPT_SIZE1:
            return true;
        default:
            return false;
    }
}

bool CoapEndpointBase::findUnrecognizedCriticalOption(const CoapHeader& header, bool isRequest, uint16_t& badOptionOut) const
{
    for (size_t i = 0; i < header.getOptionsArraySize(); ++i) {
        uint16_t num = header.getOptions(i).getNumber();
        if (!isCoapOptionCritical(num))
            continue;
        bool known = isRequest ? isRecognizedRequestOption(num) : isRecognizedResponseOption(num);
        if (!known) {
            badOptionOut = num;
            return true;
        }
    }
    return false;
}

void CoapEndpointBase::socketDataArrived(UdpSocket *arrivedSocket, Packet *packet)
{
    if (operationalState != State::OPERATING) {
        delete packet;
        return;
    }

    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&getCoapProtocol());
    emit(packetReceivedSignal, packet);
    emit(packetReceivedFromLowerSignal, packet);

    const auto& l3Ind = packet->findTag<L3AddressInd>();
    const auto& l4Ind = packet->findTag<L4PortInd>();
    if (l3Ind == nullptr || l4Ind == nullptr) {
        emit(malformedReceivedSignal, 1L);
        delete packet;
        return;
    }

    CoapPeerEndpoint peer{l3Ind->getSrcAddress(), l4Ind->getSrcPort(), 0};
    L3Address localDest = l3Ind->getDestAddress();
    bool destIsMulticast = !localDest.isUnspecified() && localDest.isMulticast();
    processInboundCoapPacket(peer, destIsMulticast, packet);
}

void CoapEndpointBase::processInboundCoapPacket(const CoapPeerEndpoint& peer, bool destIsMulticast, Packet *packet)
{
    if (packet->hasBitError()) {
        emit(malformedReceivedSignal, 1L);
        delete packet;
        return;
    }

    Ptr<CoapHeader> header;
    std::vector<uint8_t> payload;
    if (!parseInboundDatagram(packet, header, payload)) {
        emit(malformedReceivedSignal, 1L);
        if (header != nullptr && header->getVersion() == 1 && header->getMessageType() == COAP_TYPE_CON && !destIsMulticast)
            sendEmptyMessageToPeer(COAP_TYPE_RST, header->getMessageId(), peer);
        delete packet;
        return;
    }

    CoapMessageType type = header->getMessageType();
    uint8_t code = header->getCode();
    uint16_t mid = header->getMessageId();
    uint8_t codeClass = getCoapCodeClass(code);

    // Reject impossible request-in-ACK/RST and non-empty RST combinations before dispatching.
    if ((codeClass == 0 && code != COAP_CODE_EMPTY && (type == COAP_TYPE_ACK || type == COAP_TYPE_RST))
            || (type == COAP_TYPE_RST && code != COAP_CODE_EMPTY)) {
        emit(malformedReceivedSignal, 1L);
        delete packet;
        return;
    }

    if (type == COAP_TYPE_CON || type == COAP_TYPE_NON) {
        if (checkAndHandleDuplicate(peer, mid, type)) {
            delete packet;
            return;
        }
        recordDuplicateEntry(peer, mid, type == COAP_TYPE_CON,
                type == COAP_TYPE_CON ? exchangeLifetime : nonLifetime);
    }

    if (code == COAP_CODE_EMPTY) {
        if (type == COAP_TYPE_CON) {
            if (!destIsMulticast)
                sendEmptyMessageAndCacheDuplicate(COAP_TYPE_RST, mid, peer);
        }
        else if (type == COAP_TYPE_ACK) {
            handleEmptyAck(peer, mid);
        }
        else if (type == COAP_TYPE_RST) {
            handleReset(peer, mid);
        }
        delete packet;
        return;
    }

    if (codeClass == 0) {
        uint16_t badOpt = 0;
        if (findUnrecognizedCriticalOption(*header, true, badOpt)) {
            if (type == COAP_TYPE_CON && !destIsMulticast) {
                auto errHeader = makeShared<CoapHeader>();
                errHeader->setVersion(1);
                errHeader->setMessageType(COAP_TYPE_ACK);
                errHeader->setCode(COAP_CODE_BAD_OPTION);
                errHeader->setMessageId(mid);
                errHeader->setTokenArraySize(header->getTokenArraySize());
                for (size_t i = 0; i < header->getTokenArraySize(); ++i)
                    errHeader->setToken(i, header->getToken(i));
                std::vector<uint8_t> emptyPayload;
                Packet *errPkt = buildCoapPacket("CoapBadOption", errHeader, emptyPayload);
                cacheDuplicateResponse(peer, mid, errPkt);
                sendCoapPacketToPeer(peer, errPkt);
            }
            delete packet;
            return;
        }
        handleInboundRequest(peer, destIsMulticast, header, payload);
        delete packet;
        return;
    }

    // Response class (2, 4, or 5)
    std::vector<uint8_t> token(header->getTokenArraySize());
    for (size_t i = 0; i < header->getTokenArraySize(); ++i)
        token[i] = header->getToken(i);

    if (type == COAP_TYPE_ACK) {
        auto exIt = activeExchanges.find(std::make_pair(peer, mid));
        if (exIt == activeExchanges.end() || exIt->second.token != token) {
            delete packet;
            return;
        }

        auto reqIt = pendingRequests.find(token);
        if (reqIt == pendingRequests.end() || (!reqIt->second.peerIsMulticast && reqIt->second.peer != peer)) {
            delete packet;
            return;
        }

        // Valid piggybacked ACK completes message-layer CON retransmission.
        markPeerReachable(peer);
        auto peerIt = peerCongestion.find(peer);
        if (peerIt != peerCongestion.end())
            peerIt->second.activeConfirmables = std::max(0, peerIt->second.activeConfirmables - 1);
        uint64_t obId = exIt->second.outboundId;
        uint64_t reqIdForInFlight = exIt->second.isRequest ? exIt->second.requestId : 0;
        activeExchanges.erase(exIt);
        eraseDtlsInFlight(obId, reqIdForInFlight);
        onOutboundItemAcknowledged(obId);
        pumpOutboundQueue(simTime());

        uint16_t badOpt = 0;
        if (findUnrecognizedCriticalOption(*header, false, badOpt)) {
            uint64_t reqId = reqIt->second.requestId;
            removePendingRequest(token);
            dispatchRequestFailed(reqId, "Unrecognized critical option in CoAP response", COAP_CODE_BAD_OPTION);
            delete packet;
            return;
        }

        CoapPendingRequest matchedReq = reqIt->second;
        handleMatchedResponse(matchedReq, peer, header, payload);
        delete packet;
        return;
    }

    auto reqIt = pendingRequests.find(token);
    bool peerMatches = false;
    if (reqIt != pendingRequests.end()) {
        if (reqIt->second.peerIsMulticast || reqIt->second.peer == peer)
            peerMatches = true;
    }

    if (reqIt == pendingRequests.end() || !peerMatches) {
        if (handleUnsolicitedResponse(peer, destIsMulticast, header, payload)) {
            delete packet;
            return;
        }
        if ((type == COAP_TYPE_CON || type == COAP_TYPE_NON) && !destIsMulticast)
            sendEmptyMessageAndCacheDuplicate(COAP_TYPE_RST, mid, peer);
        delete packet;
        return;
    }

    markPeerReachable(peer);

    uint16_t badOpt = 0;
    if (findUnrecognizedCriticalOption(*header, false, badOpt)) {
        if (type == COAP_TYPE_CON && !destIsMulticast) {
            sendEmptyMessageAndCacheDuplicate(COAP_TYPE_RST, mid, peer);
            uint64_t reqId = reqIt->second.requestId;
            cancelConfirmableExchangesForRequest(reqId);
            removePendingRequest(token);
            dispatchRequestFailed(reqId, "Unrecognized critical option in CoAP response", COAP_CODE_BAD_OPTION);
        }
        delete packet;
        return;
    }

    CoapPendingRequest matchedReq = reqIt->second;
    if (type == COAP_TYPE_CON && !destIsMulticast)
        sendEmptyMessageAndCacheDuplicate(COAP_TYPE_ACK, mid, peer);
    cancelConfirmableExchangesForRequest(matchedReq.requestId);
    handleMatchedResponse(matchedReq, peer, header, payload);
    delete packet;
}

void CoapEndpointBase::handleInboundRequest(const CoapPeerEndpoint& peer, bool destIsMulticast,
        const Ptr<const CoapHeader>& header, const std::vector<uint8_t>& payload)
{
    if (header->getMessageType() == COAP_TYPE_CON && !destIsMulticast) {
        auto respHeader = makeShared<CoapHeader>();
        respHeader->setVersion(1);
        respHeader->setMessageType(COAP_TYPE_ACK);
        respHeader->setCode(COAP_CODE_METHOD_NOT_ALLOWED);
        respHeader->setMessageId(header->getMessageId());
        respHeader->setTokenArraySize(header->getTokenArraySize());
        for (size_t i = 0; i < header->getTokenArraySize(); ++i)
            respHeader->setToken(i, header->getToken(i));
        std::vector<uint8_t> emptyPayload;
        Packet *respPkt = buildCoapPacket("CoapMethodNotAllowed", respHeader, emptyPayload);
        cacheDuplicateResponse(peer, header->getMessageId(), respPkt);
        sendCoapPacketToPeer(peer, respPkt);
    }
}

void CoapEndpointBase::handleMatchedResponse(const CoapPendingRequest& request, const CoapPeerEndpoint& peer,
        const Ptr<const CoapHeader>& header, const std::vector<uint8_t>& payload)
{
    removePendingRequest(request.token);
    dispatchMatchedResponse(request.requestId, peer, header, payload);
}

void CoapEndpointBase::handleEmptyAck(const CoapPeerEndpoint& peer, uint16_t messageId)
{
    auto exIt = activeExchanges.find(std::make_pair(peer, messageId));
    if (exIt == activeExchanges.end())
        return;

    markPeerReachable(peer);
    auto peerIt = peerCongestion.find(peer);
    if (peerIt != peerCongestion.end())
        peerIt->second.activeConfirmables = std::max(0, peerIt->second.activeConfirmables - 1);
    uint64_t obId = exIt->second.outboundId;
    uint64_t reqId = exIt->second.isRequest ? exIt->second.requestId : 0;
    activeExchanges.erase(exIt);
    eraseDtlsInFlight(obId, reqId);
    onOutboundItemAcknowledged(obId);
    pumpOutboundQueue(simTime());
    rescheduleDeadlineTimer();
}

void CoapEndpointBase::handleReset(const CoapPeerEndpoint& peer, uint16_t messageId)
{
    auto exIt = activeExchanges.find(std::make_pair(peer, messageId));
    if (exIt != activeExchanges.end()) {
        CoapOutboundExchange ex = std::move(exIt->second);
        activeExchanges.erase(exIt);
        eraseDtlsInFlight(ex.outboundId, ex.isRequest ? ex.requestId : 0);

        markPeerReachable(peer);
        auto peerIt = peerCongestion.find(peer);
        if (peerIt != peerCongestion.end())
            peerIt->second.activeConfirmables = std::max(0, peerIt->second.activeConfirmables - 1);

        onOutboundItemFailed(ex.outboundId, "CoAP exchange reset by peer");
        if (ex.isRequest) {
            bool stillPending = (pendingRequests.find(ex.token) != pendingRequests.end());
            removePendingRequest(ex.token);
            if (stillPending)
                dispatchRequestFailed(ex.requestId, "CoAP exchange reset by peer");
        }
        else {
            onOutboundExchangeFailed(ex.exchangeId, ex.peer, ex.messageId, "CoAP exchange reset by peer");
        }
        pumpOutboundQueue(simTime());
        rescheduleDeadlineTimer();
        return;
    }

    auto mapIt = outboundMidToToken.find(std::make_pair(peer, messageId));
    if (mapIt == outboundMidToToken.end())
        return;

    std::vector<uint8_t> token = mapIt->second;
    auto reqIt = pendingRequests.find(token);
    if (reqIt == pendingRequests.end()) {
        outboundMidToToken.erase(mapIt);
        return;
    }

    markPeerReachable(peer);
    uint64_t reqId = reqIt->second.requestId;
    removePendingRequest(token);
    dispatchRequestFailed(reqId, "CoAP exchange reset by peer");
}

void CoapEndpointBase::eraseDtlsInFlight(uint64_t outboundId, uint64_t requestId)
{
#ifdef INET_WITH_DTLS
    if (dtlsInFlightPackets.empty())
        return;
    for (auto it = dtlsInFlightPackets.begin(); it != dtlsInFlightPackets.end();) {
        bool matchOutbound = (outboundId != 0 && it->second.outboundId == outboundId);
        bool matchRequest = (requestId != 0 && it->second.requestId == requestId);
        if (matchOutbound || matchRequest)
            it = dtlsInFlightPackets.erase(it);
        else
            ++it;
    }
#endif
}

bool CoapEndpointBase::resolveDtlsPeerSession(const L3Address& destAddr, int destPort, CoapPeerEndpoint& peerOut)
{
    if (!isDtlsMode()) {
        peerOut = CoapPeerEndpoint{destAddr, destPort, 0};
        return true;
    }
#ifdef INET_WITH_DTLS
    if (!dtlsBound || destAddr.isUnspecified() || destAddr.isMulticast() || destPort <= 0 || destPort > 65535)
        return false;

    auto key = std::make_pair(destAddr, destPort);
    auto activeIt = activeClientDtlsByAddrPort.find(key);
    if (activeIt != activeClientDtlsByAddrPort.end()) {
        auto sessIt = dtlsSessions.find(activeIt->second);
        if (sessIt != dtlsSessions.end() && !sessIt->second.closing) {
            peerOut = sessIt->second.peer;
            return true;
        }
        activeClientDtlsByAddrPort.erase(activeIt);
    }

    dtls::IDtls::AssociationId id = dtlsSocket.open(destAddr, destPort);
    if (id == 0)
        return false;

    CoapDtlsSession session;
    session.id = id;
    session.peer = CoapPeerEndpoint{destAddr, destPort, id};
    session.outgoing = true;
    session.established = false;
    session.closing = false;
    dtlsSessions[id] = session;
    activeClientDtlsByAddrPort[key] = id;
    peerOut = session.peer;
    return true;
#else
    return false;
#endif
}

void CoapEndpointBase::listenOnDtls()
{
#ifdef INET_WITH_DTLS
    if (isDtlsMode() && dtlsBound)
        dtlsSocket.listen();
#endif
}

void CoapEndpointBase::onDtlsSessionTerminated(uint64_t sessionId, const char *reason)
{
#ifdef INET_WITH_DTLS
    auto sessIt = dtlsSessions.find(sessionId);
    if (sessIt == dtlsSessions.end())
        return;

    CoapDtlsSession session = std::move(sessIt->second);
    dtlsSessions.erase(sessIt);

    if (session.outgoing) {
        auto key = std::make_pair(session.peer.address, session.peer.port);
        auto mapIt = activeClientDtlsByAddrPort.find(key);
        if (mapIt != activeClientDtlsByAddrPort.end() && mapIt->second == sessionId)
            activeClientDtlsByAddrPort.erase(mapIt);
    }

    for (auto it = dtlsInFlightPackets.begin(); it != dtlsInFlightPackets.end();) {
        if (it->second.sessionId == sessionId)
            it = dtlsInFlightPackets.erase(it);
        else
            ++it;
    }

    for (auto it = duplicateEntries.begin(); it != duplicateEntries.end();) {
        if (it->first.peer.sessionId == sessionId)
            it = duplicateEntries.erase(it);
        else
            ++it;
    }

    if (operationalState != State::OPERATING)
        return;

    const char *effectiveReason = (reason != nullptr && reason[0] != '\0') ? reason : "DTLS session terminated";
    onDtlsSessionClosing(sessionId, effectiveReason);

    std::vector<uint64_t> droppedOutboundIds;
    for (auto it = outboundQueue.begin(); it != outboundQueue.end();) {
        if (it->peer.sessionId == sessionId) {
            if (!it->isRequest && it->outboundId != 0)
                droppedOutboundIds.push_back(it->outboundId);
            it = outboundQueue.erase(it);
        }
        else {
            ++it;
        }
    }
    for (uint64_t obId : droppedOutboundIds)
        onOutboundItemFailed(obId, effectiveReason);

    std::vector<std::pair<CoapPeerEndpoint, uint16_t>> exchangeKeys;
    for (const auto& kv : activeExchanges) {
        if (kv.first.first.sessionId == sessionId)
            exchangeKeys.push_back(kv.first);
    }
    for (const auto& key : exchangeKeys) {
        auto exIt = activeExchanges.find(key);
        if (exIt == activeExchanges.end())
            continue;
        CoapOutboundExchange ex = std::move(exIt->second);
        activeExchanges.erase(exIt);
        auto peerIt = peerCongestion.find(ex.peer);
        if (peerIt != peerCongestion.end())
            peerIt->second.activeConfirmables = std::max(0, peerIt->second.activeConfirmables - 1);
        if (!ex.isRequest) {
            if (ex.outboundId != 0)
                onOutboundItemFailed(ex.outboundId, effectiveReason);
            onOutboundExchangeFailed(ex.exchangeId, ex.peer, ex.messageId, effectiveReason);
        }
    }

    for (auto it = outboundMidToToken.begin(); it != outboundMidToToken.end();) {
        if (it->first.first.sessionId == sessionId)
            it = outboundMidToToken.erase(it);
        else
            ++it;
    }

    std::vector<std::pair<uint64_t, std::vector<uint8_t>>> failedRequests;
    for (const auto& kv : pendingRequests) {
        if (kv.second.peer.sessionId == sessionId)
            failedRequests.emplace_back(kv.second.requestId, kv.first);
    }
    for (const auto& item : failedRequests) {
        removePendingRequest(item.second);
        dispatchRequestFailed(item.first, effectiveReason);
    }

    for (auto it = peerCongestion.begin(); it != peerCongestion.end();) {
        if (it->first.sessionId == sessionId)
            it = peerCongestion.erase(it);
        else
            ++it;
    }

    onDtlsSessionClosed(sessionId, effectiveReason);
    rescheduleDeadlineTimer();
#endif
}

#ifdef INET_WITH_DTLS
void CoapEndpointBase::accepted(dtls::IDtls::AssociationId id, const L3Address& localAddress, int localPort,
        const L3Address& peerAddress, int peerPort)
{
    Enter_Method("accepted");
    if (operationalState != State::OPERATING || !isDtlsMode())
        return;

    CoapDtlsSession session;
    session.id = id;
    session.peer = CoapPeerEndpoint{peerAddress, peerPort, id};
    session.outgoing = false;
    session.established = false;
    session.closing = false;
    dtlsSessions[id] = session;
}

void CoapEndpointBase::established(dtls::IDtls::AssociationId id)
{
    Enter_Method("established");
    auto it = dtlsSessions.find(id);
    if (it != dtlsSessions.end()) {
        it->second.established = true;
        if (operationalState == State::OPERATING) {
            pumpOutboundQueue(simTime());
            rescheduleDeadlineTimer();
        }
    }
}

void CoapEndpointBase::dataArrived(dtls::IDtls::AssociationId id, Packet *packet)
{
    Enter_Method("dataArrived");
    take(packet);
    if (operationalState != State::OPERATING || !isDtlsMode()) {
        delete packet;
        return;
    }

    auto sessIt = dtlsSessions.find(id);
    if (sessIt == dtlsSessions.end()) {
        delete packet;
        return;
    }

    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&getCoapProtocol());
    emit(packetReceivedSignal, packet);
    emit(packetReceivedFromLowerSignal, packet);

    processInboundCoapPacket(sessIt->second.peer, false, packet);
}

void CoapEndpointBase::sendError(dtls::IDtls::AssociationId id, long packetId, const char *reason)
{
    Enter_Method("sendError");
    if (insideDtlsSend && dtlsSendErrorTimer != nullptr) {
        pendingDtlsSendErrors.push_back({id, packetId, reason != nullptr ? reason : ""});
        if (!dtlsSendErrorTimer->isScheduled())
            scheduleAt(simTime(), dtlsSendErrorTimer);
        return;
    }
    handleDtlsSendError(id, packetId, reason);
}

void CoapEndpointBase::processPendingDtlsSendErrors()
{
    if (pendingDtlsSendErrors.empty())
        return;
    auto errors = std::move(pendingDtlsSendErrors);
    pendingDtlsSendErrors.clear();
    for (const auto& entry : errors)
        handleDtlsSendError(entry.id, entry.packetId, entry.reason.c_str());
}

void CoapEndpointBase::handleDtlsSendError(dtls::IDtls::AssociationId id, long packetId, const char *reason)
{
    auto it = dtlsInFlightPackets.find(packetId);
    if (it == dtlsInFlightPackets.end())
        return;

    CoapDtlsInFlightPacket info = std::move(it->second);
    dtlsInFlightPackets.erase(it);

    if (operationalState != State::OPERATING)
        return;

    const char *effectiveReason = (reason != nullptr && reason[0] != '\0') ? reason : "DTLS send error";
    if (info.isConfirmable) {
        auto exIt = activeExchanges.find(std::make_pair(info.peer, info.messageId));
        if (exIt != activeExchanges.end() && exIt->second.outboundId == info.outboundId) {
            CoapOutboundExchange ex = std::move(exIt->second);
            activeExchanges.erase(exIt);
            auto peerIt = peerCongestion.find(info.peer);
            if (peerIt != peerCongestion.end())
                peerIt->second.activeConfirmables = std::max(0, peerIt->second.activeConfirmables - 1);
            if (ex.outboundId != 0)
                onOutboundItemFailed(ex.outboundId, effectiveReason);
            if (ex.isRequest) {
                bool stillPending = (pendingRequests.find(ex.token) != pendingRequests.end());
                removePendingRequest(ex.token);
                if (stillPending)
                    dispatchRequestFailed(ex.requestId, effectiveReason);
            }
            else {
                onOutboundExchangeFailed(ex.exchangeId, ex.peer, ex.messageId, effectiveReason);
            }
        }
    }
    else {
        if (info.outboundId != 0)
            onOutboundItemFailed(info.outboundId, effectiveReason);
        if (info.isRequest) {
            auto reqIt = pendingRequests.find(info.token);
            if (reqIt != pendingRequests.end() && reqIt->second.requestId == info.requestId) {
                removePendingRequest(info.token);
                dispatchRequestFailed(info.requestId, effectiveReason);
            }
        }
    }

    pumpOutboundQueue(simTime());
    rescheduleDeadlineTimer();
}

void CoapEndpointBase::error(dtls::IDtls::AssociationId id, const char *reason)
{
    Enter_Method("error");
    onDtlsSessionTerminated(id, reason);
}

void CoapEndpointBase::closed(dtls::IDtls::AssociationId id)
{
    Enter_Method("closed");
    onDtlsSessionTerminated(id, "DTLS session closed");
}
#endif

} // namespace coap
} // namespace inet
