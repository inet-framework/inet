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
}

void CoapEndpointBase::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        parseAndValidateParameters();
        initializeCountersOnce();
        if (deadlineTimer == nullptr)
            deadlineTimer = new cMessage("deadlineTimer");
    }
}

void CoapEndpointBase::parseAndValidateParameters()
{
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
    for (const auto& kv : pendingRequests)
        onRequestAborted(kv.second.requestId);

    pendingRequests.clear();
    outboundMidToToken.clear();
    activeExchanges.clear();
    peerCongestion.clear();
    outboundQueue.clear();

    if (clearDuplicates) {
        conDuplicateOrder.clear();
        nonDuplicateOrder.clear();
        duplicateEntries.clear();
    }
}

void CoapEndpointBase::handleMessageWhenUp(cMessage *msg)
{
    if (msg == deadlineTimer) {
        onDeadlineTimer();
    }
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
    onEndpointStop();
    clearTransientState(false);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void CoapEndpointBase::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(deadlineTimer);
    onEndpointCrash();
    clearTransientState(true);
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

void CoapEndpointBase::sendEmptyMessage(CoapMessageType type, uint16_t messageId, const L3Address& destAddr, int destPort)
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
    sendCoapPacket(packet, destAddr, destPort);
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
    sendCoapPacket(packet, peer.address, peer.port);
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
        const Ptr<CoapHeader>& header, const std::vector<uint8_t>& payload)
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
        sendCoapPacket(packet, peer.address, peer.port);
        rescheduleDeadlineTimer();
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
    item.requestId = requestId;
    item.token = token;
    item.packetName = packetName;
    item.header = header;
    item.payload = payload;
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
            ex.peer = dispatched.peer;
            ex.messageId = mid;
            ex.isRequest = dispatched.isRequest;
            ex.requestId = dispatched.requestId;
            ex.token = dispatched.token;
            ex.packetName = dispatched.packetName;
            ex.wireBytes = wireBytes;
            ex.initialTimeout = initTimeout;
            ex.currentTimeout = initTimeout;
            ex.nextRetryAt = now + initTimeout;
            ex.retransmitCount = 0;

            activeExchanges[std::make_pair(dispatched.peer, mid)] = std::move(ex);
            peerState.activeConfirmables += 1;
        }

        sendCoapPacket(pkt, dispatched.peer.address, dispatched.peer.port);
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

            Packet *retry = new Packet(ex.packetName.c_str());
            retry->insertAtBack(makeShared<BytesChunk>(ex.wireBytes));
            emit(retransmissionSignal, 1L);
            sendCoapPacket(retry, ex.peer.address, ex.peer.port);
        }
        else {
            CoapOutboundExchange timedOut = std::move(it->second);
            activeExchanges.erase(it);

            auto& peerState = peerCongestion[timedOut.peer];
            peerState.activeConfirmables = std::max(0, peerState.activeConfirmables - 1);
            peerState.inProbingBackoff = true;
            simtime_t probeWait = SimTime(static_cast<double>(timedOut.wireBytes.size()) / probingRateBytesPerSec);
            peerState.probingEarliestAt = std::max(peerState.probingEarliestAt, now + probeWait);

            if (timedOut.isRequest) {
                bool stillPending = (pendingRequests.find(timedOut.token) != pendingRequests.end());
                removePendingRequest(timedOut.token);
                if (stillPending)
                    onRequestFailed(timedOut.requestId, "CoAP confirmable exchange timed out");
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
        sendCoapPacket(replay, peer.address, peer.port);
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

void CoapEndpointBase::removePendingRequest(const std::vector<uint8_t>& token)
{
    auto it = pendingRequests.find(token);
    if (it == pendingRequests.end())
        return;

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
        onRequestFailed(item.first, "CoAP request timed out");
    }
}

bool CoapEndpointBase::isRecognizedRequestOption(uint16_t optionNumber) const
{
    switch (optionNumber) {
        case COAP_OPT_IF_MATCH:
        case COAP_OPT_URI_HOST:
        case COAP_OPT_ETAG:
        case COAP_OPT_IF_NONE_MATCH:
        case COAP_OPT_URI_PORT:
        case COAP_OPT_URI_PATH:
        case COAP_OPT_CONTENT_FORMAT:
        case COAP_OPT_MAX_AGE:
        case COAP_OPT_URI_QUERY:
        case COAP_OPT_ACCEPT:
        case COAP_OPT_PROXY_URI:
        case COAP_OPT_PROXY_SCHEME:
            return true;
        default:
            return false;
    }
}

bool CoapEndpointBase::isRecognizedResponseOption(uint16_t optionNumber) const
{
    switch (optionNumber) {
        case COAP_OPT_ETAG:
        case COAP_OPT_LOCATION_PATH:
        case COAP_OPT_LOCATION_QUERY:
        case COAP_OPT_CONTENT_FORMAT:
        case COAP_OPT_MAX_AGE:
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

    CoapPeerEndpoint peer{l3Ind->getSrcAddress(), l4Ind->getSrcPort()};
    L3Address localDest = l3Ind->getDestAddress();
    bool destIsMulticast = !localDest.isUnspecified() && localDest.isMulticast();

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
            sendEmptyMessage(COAP_TYPE_RST, header->getMessageId(), peer.address, peer.port);
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
                sendCoapPacket(errPkt, peer.address, peer.port);
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
        activeExchanges.erase(exIt);
        pumpOutboundQueue(simTime());

        uint16_t badOpt = 0;
        if (findUnrecognizedCriticalOption(*header, false, badOpt)) {
            uint64_t reqId = reqIt->second.requestId;
            removePendingRequest(token);
            onRequestFailed(reqId, "Unrecognized critical option in CoAP response", COAP_CODE_BAD_OPTION);
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
            onRequestFailed(reqId, "Unrecognized critical option in CoAP response", COAP_CODE_BAD_OPTION);
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
        sendCoapPacket(respPkt, peer.address, peer.port);
    }
}

void CoapEndpointBase::handleMatchedResponse(const CoapPendingRequest& request, const CoapPeerEndpoint& peer,
        const Ptr<const CoapHeader>& header, const std::vector<uint8_t>& payload)
{
    removePendingRequest(request.token);
    onRequestCompleted(request.requestId, peer, header, payload);
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
    activeExchanges.erase(exIt);
    pumpOutboundQueue(simTime());
    rescheduleDeadlineTimer();
}

void CoapEndpointBase::handleReset(const CoapPeerEndpoint& peer, uint16_t messageId)
{
    auto exIt = activeExchanges.find(std::make_pair(peer, messageId));
    if (exIt != activeExchanges.end()) {
        CoapOutboundExchange ex = std::move(exIt->second);
        activeExchanges.erase(exIt);

        markPeerReachable(peer);
        auto peerIt = peerCongestion.find(peer);
        if (peerIt != peerCongestion.end())
            peerIt->second.activeConfirmables = std::max(0, peerIt->second.activeConfirmables - 1);

        if (ex.isRequest) {
            bool stillPending = (pendingRequests.find(ex.token) != pendingRequests.end());
            removePendingRequest(ex.token);
            if (stillPending)
                onRequestFailed(ex.requestId, "CoAP exchange reset by peer");
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
    onRequestFailed(reqId, "CoAP exchange reset by peer");
}

} // namespace coap
} // namespace inet
