// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsAssociation.h"

#include <algorithm>
#include <limits>

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/transportlayer/dtls/Dtls.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"
#include "inet/transportlayer/dtls/DtlsRecordProtection.h"

namespace inet::dtls {

DtlsAssociation::DtlsAssociation(Dtls& engine, size_t reassemblyLimit) : engine(engine), reassembly(reassemblyLimit)
{
    retransmissionTimer = std::make_unique<cMessage>("dtlsRetransmission");
    delayedAckTimer = std::make_unique<cMessage>("dtlsDelayedAck");
    retentionTimer = std::make_unique<cMessage>("dtlsFinalAckRetention");
    updateRetransmissionTimer = std::make_unique<cMessage>("dtlsUpdateRetransmission");
    updateDeadline = std::make_unique<cMessage>("dtlsUpdateDeadline");
}

void DtlsAssociation::cancelTimers()
{
    engine.cancelEvent(retransmissionTimer.get());
    engine.cancelEvent(delayedAckTimer.get());
    engine.cancelEvent(retentionTimer.get());
    engine.cancelEvent(updateRetransmissionTimer.get());
    engine.cancelEvent(updateDeadline.get());
}

bool DtlsAssociation::handleTimer(cMessage *message)
{
    // RFC 9147 Section 5.8.2 recommends doubling the retransmission timer up to at least 60 seconds.
    if (message == updateRetransmissionTimer.get()) {
        updateTimeout = std::min(SimTime(60), updateTimeout * 2);
        if (updateFlight) {
            updateFlight->requestRetransmission();
            pumpUpdate();
        }
        return true;
    }
    if (message == updateDeadline.get()) {
        engine.failAssociation(id, "DTLS KeyUpdate timeout");
        return true;
    }
    if (message == retransmissionTimer.get()) {
        if (flight && !flight->isComplete()) {
            retransmissionTimeout = std::min(SimTime(60), retransmissionTimeout * 2);
            flight->requestRetransmission();
            burstLimit = burstQueued;
            pumpFlight();
        }
        return true;
    }
    if (message == delayedAckTimer.get()) {
        flushAcknowledgements(state == HandshakeState::ESTABLISHED ? sendEpoch : 2);
        return true;
    }
    if (message == retentionTimer.get()) {
        processedHandshakes.clear();
        receiveEpoch2.reset();
        return true;
    }
    return false;
}

void DtlsAssociation::noteHandshakeRecord(uint64_t epoch, uint64_t recordNumber, bool immediate)
{
    pendingAcks.emplace(epoch, recordNumber);
    if (immediate)
        flushAcknowledgements(state == HandshakeState::ESTABLISHED ? sendEpoch : 2);
    else if (!delayedAckTimer->isScheduled())
        engine.scheduleAt(simTime() + std::max(SimTime::fromRaw(1), retransmissionTimeout / 4), delayedAckTimer.get());
}

void DtlsAssociation::flushAcknowledgements(uint64_t sendEpoch)
{
    engine.cancelEvent(delayedAckTimer.get());
    if (pendingAcks.empty())
        return;
    std::vector<std::pair<uint64_t, uint64_t>> records(pendingAcks.begin(), pendingAcks.end());
    pendingAcks.clear();
    engine.sendAcknowledgements(id, sendEpoch, records);
}

void DtlsAssociation::clearAcknowledgements()
{
    engine.cancelEvent(delayedAckTimer.get());
    pendingAcks.clear();
}

bool DtlsAssociation::allocateApplicationRecord(uint64_t& number)
{
    if (nextApplicationRecord >= MAX_PROTECTED_RECORDS)
        return false;
    number = nextApplicationRecord++;
    return true;
}

bool DtlsAssociation::allocatePlaintextRecord(uint64_t& number)
{
    if (nextEpoch0Record >= MAX_PLAINTEXT_RECORDS)
        return false;
    number = nextEpoch0Record++;
    return true;
}

bool DtlsAssociation::advanceSendEpoch()
{
    if (sendEpoch >= MAX_SEND_EPOCH)
        return false;
    ++sendEpoch;
    nextApplicationRecord = 0;
    return true;
}

bool DtlsAssociation::requestUpdate(bool requestPeerUpdate)
{
    // RFC 9846 Section 4.7.3 forbids another requested update until a subsequent peer KeyUpdate.
    if (state != HandshakeState::ESTABLISHED || updateFlight ||
        (requestPeerUpdate && awaitingPeerUpdate) || sendEpoch >= MAX_SEND_EPOCH || localMessagesExhausted)
        return false;
    auto timeout = SimTime(engine.par("handshakeTimeout").doubleValue());
    if (timeout <= SIMTIME_ZERO)
        return false;
    auto body = makeShared<tls::TlsKeyUpdate>();
    body->setRequestUpdate(requestPeerUpdate ? 1 : 0);
    body->setChunkLength(B(1));
    body->markImmutable();
    auto update = std::make_shared<DtlsHandshakeFlight>();
    auto messageSequence = nextLocalMessage;
    if (nextLocalMessage == std::numeric_limits<uint16_t>::max())
        localMessagesExhausted = true;
    else
        ++nextLocalMessage;
    update->add(24, messageSequence, 1, 0, sendEpoch, body);
    updateFlight = update;
    updateRequested = requestPeerUpdate;
    updateTimeout = 1;
    auto& owner = engine;
    auto handle = id;
    owner.scheduleAt(simTime() + timeout, updateDeadline.get());
    auto delay = SimTime(owner.par("handshakeOutDelay").doubleValue());
    if (!owner.enqueueProcessing(owner.outputProcessing.get(), handle, 13, delay, [&owner, handle, update]() {
        auto it = owner.associations.find(handle);
        if (it == owner.associations.end() || it->second->updateFlight != update)
            return;
        update->markReady(0);
        it->second->pumpUpdate();
    }, [&owner, handle, update]() {
        auto it = owner.associations.find(handle);
        return it != owner.associations.end() && it->second->updateFlight == update;
    })) {
        owner.failAssociation(handle, "DTLS output processing queue is full");
        return true;
    }
    return true;
}

void DtlsAssociation::pumpUpdate()
{
    if (!updateFlight || updateFlight->isComplete())
        return;
    auto update = updateFlight;
    if (!update->reserve(0))
        return;
    engine.transmitFlightFragment(id, update, 0);
}

void DtlsAssociation::updateRecordHandedOff(const std::shared_ptr<DtlsHandshakeFlight>& sentFlight, size_t index, uint64_t recordNumber)
{
    if (updateFlight != sentFlight)
        return;
    bool retransmission = sentFlight->get(index).transmissions != 0;
    sentFlight->handedOff(index, recordNumber, simTime());
    if (!retransmission && updateRequested)
        awaitingPeerUpdate = true;
    auto& owner = engine;
    auto handle = id;
    if (retransmission)
        owner.emit(Dtls::handshakeRetransmissionSignal, (long)handle);
    auto it = owner.associations.find(handle);
    if (it != owner.associations.end() && it->second->updateFlight == sentFlight && !sentFlight->isComplete() &&
        !it->second->updateRetransmissionTimer->isScheduled())
        owner.scheduleAt(simTime() + it->second->updateTimeout, it->second->updateRetransmissionTimer.get());
}

void DtlsAssociation::updateRecordSkipped(const std::shared_ptr<DtlsHandshakeFlight>& sentFlight, size_t index)
{
    sentFlight->cancelReservation(index);
}

void DtlsAssociation::receiveKeyUpdate(uint64_t epoch, uint64_t recordSequence, Packet *body)
{
    if (body->getDataLength() < B(12))
        return;
    Ptr<const DtlsHandshakeHeader> header;
    try {
        header = body->popAtFront<DtlsHandshakeHeader>(B(12));
    }
    catch (const cRuntimeError&) {
        engine.emit(Dtls::recordDroppedSignal, (long)Dtls::MALFORMED_RECORD);
        return;
    }
    if (header->getMessageType() != 24 || header->getMessageLength() != 1 || header->getFragmentOffset() != 0 ||
        header->getFragmentLength() > 1 || body->getDataLength() != B(header->getFragmentLength()))
        return;
    if (header->getFragmentLength() == 0)
        return;
    Ptr<const tls::TlsKeyUpdate> update;
    try {
        update = body->popAtFront<tls::TlsKeyUpdate>(B(1));
    }
    catch (const cRuntimeError&) {
        return;
    }
    auto flag = update->getRequestUpdate();
    auto sequence = header->getMessageSequence();
    bool duplicate = hasPeerUpdate && sequence == lastPeerUpdateMessage && epoch == lastPeerUpdateEpoch;
    if (!duplicate && (epoch != receiveEpoch || sequence != nextPeerMessage))
        return;
    if (flag > 1) {
        engine.failAssociation(id, "invalid DTLS KeyUpdate request", 47);
        return;
    }
    if (duplicate) {
        if (flag != lastPeerUpdateFlag) {
            engine.failAssociation(id, "conflicting DTLS KeyUpdate", 47);
            return;
        }
        auto replay = epoch == receiveEpoch ? receiveEpoch3.get() : previousReceiveEpoch.get();
        if (!replay || !replay->accept(recordSequence))
            return;
        noteHandshakeRecord(epoch, recordSequence, true);
        return;
    }
    if (receiveEpoch == std::numeric_limits<uint64_t>::max()) {
        engine.failAssociation(id, "DTLS receiving epoch exhausted");
        return;
    }
    auto& owner = engine;
    auto handle = id;
    auto delay = SimTime(owner.par("handshakeInDelay").doubleValue());
    bool satisfiesRequest = awaitingPeerUpdate;
    bool previousResponseDue = peerResponseDue;
    if (flag == 1 && sendEpoch < MAX_SEND_EPOCH)
        peerResponseDue = true;
    if (!owner.inputProcessing->enqueueContinuation(handle, 13, delay, [&owner, handle, epoch, recordSequence, sequence, flag, satisfiesRequest]() {
        auto it = owner.associations.find(handle);
        if (it != owner.associations.end())
            it->second->applyKeyUpdate(epoch, recordSequence, sequence, flag, satisfiesRequest);
    }, {}, {}, [&owner, delay]() { owner.emit(Dtls::processingDelaySignal, delay.dbl()); })) {
        peerResponseDue = previousResponseDue;
        owner.emit(Dtls::recordDroppedSignal, (long)Dtls::RESOURCE_LIMIT);
    }
}

void DtlsAssociation::applyKeyUpdate(uint64_t epoch, uint64_t recordSequence, uint16_t sequence, uint8_t flag, bool satisfiesRequest)
{
    // RFC 9147 Section 8 advances the receiving epoch on a new KeyUpdate, not its retransmissions.
    if (epoch != receiveEpoch || sequence != nextPeerMessage)
        return;
    if (!receiveEpoch3->accept(recordSequence))
        return;
    previousReceiveEpoch = std::move(receiveEpoch3);
    previousReceiveEpochNumber = receiveEpoch++;
    receiveEpoch3 = std::make_unique<DtlsRecordState>(engine.par("replayWindowSize").intValue());
    lastPeerUpdateMessage = sequence;
    lastPeerUpdateEpoch = epoch;
    lastPeerUpdateFlag = flag;
    hasPeerUpdate = true;
    ++nextPeerMessage;
    if (satisfiesRequest)
        awaitingPeerUpdate = false;
    if (flag == 1 && sendEpoch < MAX_SEND_EPOCH)
        peerResponseDue = true;
    auto& owner = engine;
    auto handle = id;
    noteHandshakeRecord(epoch, recordSequence, true);
    auto current = owner.associations.find(handle);
    if (current == owner.associations.end())
        return;
    if (current->second->peerResponseDue && !current->second->updateFlight) {
        current->second->peerResponseDue = false;
        if (!current->second->requestUpdate(false) && owner.containsAssociation(handle))
            owner.failAssociation(handle, "DTLS KeyUpdate response could not start");
    }
}

void DtlsAssociation::beginFlight()
{
    engine.cancelEvent(retransmissionTimer.get());
    flight = std::make_shared<DtlsHandshakeFlight>();
    buildingFlight = true;
    generationPending = 0;
    burstQueued = 0;
    burstLimit = 0;
}

void DtlsAssociation::appendHandshake(uint8_t type, uint16_t messageSequence, uint64_t epoch, const Ptr<Chunk>& body, bool establishAfterSend, bool serverFinished)
{
    // RFC 9147 Sections 4.4 and 5.5 fragment handshake messages to fit the transport payload.
    if (!buildingFlight || !flight || !body)
        throw cRuntimeError("DTLS handshake message has no active flight");
    auto size = body->getChunkLength().get<B>();
    auto udpLimit = engine.par("udpPayloadLimit").intValue();
    int overhead = epoch == 0 ? 25 : 34;
    if (udpLimit <= overhead || size <= 0 || size > 0xffffff)
        throw cRuntimeError("DTLS handshake message or UDP payload limit is invalid");
    size_t maximum = std::min<size_t>(udpLimit - overhead, DtlsRecordProtection::MAX_CONTENT_BYTES - 12);
    auto currentFlight = flight;
    std::vector<size_t> indices;
    if ((size_t)size <= maximum) {
        body->markImmutable();
        indices.push_back(currentFlight->add(type, messageSequence, size, 0, epoch, body, establishAfterSend, serverFinished));
    }
    else {
        Packet source("DTLS handshake body", body);
        auto bytes = source.peekDataAsBytes()->getBytes();
        for (size_t offset = 0; offset < bytes.size(); offset += maximum) {
            auto length = std::min(maximum, bytes.size() - offset);
            std::vector<uint8_t> part(bytes.begin() + offset, bytes.begin() + offset + length);
            auto chunk = makeShared<BytesChunk>(part);
            chunk->markImmutable();
            bool last = offset + length == bytes.size();
            indices.push_back(currentFlight->add(type, messageSequence, size, offset, epoch, chunk, last && establishAfterSend, last && serverFinished));
        }
    }
    ++generationPending;
    auto& owner = engine;
    auto handle = id;
    auto delay = SimTime(owner.par("handshakeOutDelay").doubleValue());
    auto eligible = [&owner, handle, currentFlight]() {
        auto it = owner.associations.find(handle);
        return it != owner.associations.end() && it->second->flight == currentFlight;
    };
    if (!owner.enqueueProcessing(owner.outputProcessing.get(), handle, size + 12, delay, [&owner, handle, currentFlight, indices]() {
        auto it = owner.associations.find(handle);
        if (it == owner.associations.end() || it->second->flight != currentFlight)
            return;
        for (auto index : indices)
            currentFlight->markReady(index);
        --it->second->generationPending;
        if (!it->second->buildingFlight && it->second->generationPending == 0)
            it->second->pumpFlight();
    }, eligible))
        owner.failAssociation(handle, "DTLS output processing queue is full");
}

void DtlsAssociation::finishFlight()
{
    buildingFlight = false;
    if (generationPending == 0)
        pumpFlight();
}

void DtlsAssociation::pumpFlight()
{
    // RFC 9147 Section 5.8.3 limits each burst before ACK-driven progress.
    if (pumpingFlight || buildingFlight || generationPending || !flight || flight->isComplete())
        return;
    pumpingFlight = true;
    auto currentFlight = flight;
    auto& owner = engine;
    auto handle = id;
    for (size_t i = 0; i < currentFlight->size() && burstLimit < 10; ++i) {
        if (!currentFlight->reserve(i))
            continue;
        ++burstLimit;
        ++burstQueued;
        owner.transmitFlightFragment(handle, currentFlight, i);
        if (!owner.containsAssociation(handle))
            return;
        if (flight != currentFlight) {
            pumpingFlight = false;
            return;
        }
    }
    pumpingFlight = false;
    if (burstQueued == 0 && !currentFlight->isComplete() && burstLimit != 0 && !retransmissionTimer->isScheduled())
        owner.scheduleAt(simTime() + retransmissionTimeout, retransmissionTimer.get());
}

void DtlsAssociation::flightRecordHandedOff(const std::shared_ptr<DtlsHandshakeFlight>& sentFlight, size_t index, uint64_t recordNumber)
{
    if (flight != sentFlight)
        return;
    auto fragment = sentFlight->get(index);
    bool retransmission = fragment.transmissions != 0;
    sentFlight->handedOff(index, recordNumber, simTime());
    if (burstQueued > 0)
        --burstQueued;
    if (retransmission)
        engine.emit(Dtls::handshakeRetransmissionSignal, (long)id);
    if (!pumpingFlight && burstQueued == 0 && !sentFlight->isComplete() && !retransmissionTimer->isScheduled())
        engine.scheduleAt(simTime() + retransmissionTimeout, retransmissionTimer.get());
    auto& owner = engine;
    auto handle = id;
    if (!retransmission && fragment.establishAfterSend)
        owner.establish(handle);
    else if (!retransmission && fragment.serverFinished)
        serverFinishedHandedOff();
}

void DtlsAssociation::flightRecordSkipped(const std::shared_ptr<DtlsHandshakeFlight>& sentFlight, size_t index, size_t reservedCredit)
{
    sentFlight->cancelReservation(index);
    if (reservedCredit <= reservedBytes)
        reservedBytes -= reservedCredit;
    if (flight != sentFlight)
        return;
    if (burstQueued > 0)
        --burstQueued;
    if (burstLimit > 0)
        --burstLimit;
    if (!pumpingFlight)
        pumpFlight();
}

void DtlsAssociation::acceptAck(const DtlsAck& ack)
{
    if (ack.getEpochsArraySize() != ack.getSequenceNumbersArraySize())
        return;
    auto& owner = engine;
    auto handle = id;
    if (updateFlight) {
        auto update = updateFlight;
        bool updateProgress = false;
        for (size_t i = 0; i < ack.getEpochsArraySize(); ++i) {
            auto result = update->acknowledge(ack.getEpochs(i), ack.getSequenceNumbers(i), simTime());
            updateProgress |= result.matched;
            if (result.rtt)
                updateTimeout = std::max(SimTime::fromRaw(1), *result.rtt * 1.5);
        }
        if (updateProgress) {
            owner.cancelEvent(updateRetransmissionTimer.get());
            if (update->isComplete()) {
                owner.cancelEvent(updateDeadline.get());
                updateFlight.reset();
                if (!advanceSendEpoch()) {
                    owner.failAssociation(handle, "DTLS sending epoch exhausted");
                    return;
                }
                if (peerResponseDue) {
                    peerResponseDue = false;
                    if (!requestUpdate(false) && owner.containsAssociation(handle)) {
                        owner.failAssociation(handle, "DTLS KeyUpdate response could not start");
                        return;
                    }
                }
                else
                    owner.resumeApplications(handle);
                if (!owner.containsAssociation(handle))
                    return;
            }
            else {
                update->requestRetransmission();
                pumpUpdate();
                if (!owner.containsAssociation(handle))
                    return;
            }
        }
    }
    if (!flight)
        return;
    bool progress = false;
    for (size_t i = 0; i < ack.getEpochsArraySize(); ++i) {
        auto result = flight->acknowledge(ack.getEpochs(i), ack.getSequenceNumbers(i), simTime());
        progress |= result.matched;
        // RFC 9147 Section 5.8.2 recommends 1.5 times an eligible measured RTT.
        if (result.rtt)
            retransmissionTimeout = std::max(SimTime::fromRaw(1), *result.rtt * 1.5);
    }
    if (!progress)
        return;
    engine.cancelEvent(retransmissionTimer.get());
    if (flight->isComplete()) {
        if (!serverSide && state == HandshakeState::ESTABLISHED) {
            processedHandshakes.clear();
            receiveEpoch2.reset();
        }
    }
    else {
        flight->requestRetransmission();
        burstLimit = burstQueued;
        pumpFlight();
    }
}

void DtlsAssociation::acknowledgeFlightImplicitly()
{
    // RFC 9147 Section 7.2 treats a valid response as implicit acknowledgment of the prior flight.
    if (!flight)
        return;
    auto result = flight->acknowledgeSent(simTime());
    if (result.rtt)
        retransmissionTimeout = std::max(SimTime::fromRaw(1), *result.rtt * 1.5);
    if (result.flightComplete)
        engine.cancelEvent(retransmissionTimer.get());
}

void DtlsAssociation::receiveHandshakeFragment(uint64_t recordSequence, Packet *body)
{
    std::vector<std::unique_ptr<Packet>> fragments;
    while (body->getDataLength() > b(0)) {
        if (body->getDataLength() < B(12)) {
            engine.emit(Dtls::recordDroppedSignal, (long)Dtls::MALFORMED_RECORD);
            return;
        }
        auto header = body->popAtFront<DtlsHandshakeHeader>(B(12));
        auto total = header->getMessageLength();
        auto offset = header->getFragmentOffset();
        auto length = header->getFragmentLength();
        if (length == 0 || offset > total || length > total - offset || body->getDataLength() < B(length)) {
            engine.emit(Dtls::recordDroppedSignal, (long)Dtls::MALFORMED_RECORD);
            return;
        }
        auto fragment = std::make_unique<Packet>("DTLS handshake fragment", body->removeAtFront(B(length)));
        if (!engine.hasConsistentEncoding(fragment.get())) {
            engine.emit(Dtls::recordDroppedSignal, (long)Dtls::MALFORMED_RECORD);
            return;
        }
        fragment->insertAtFront(header);
        fragments.push_back(std::move(fragment));
    }
    if (fragments.empty())
        return;
    auto& owner = engine;
    auto handle = id;
    bool immediateAck = false, duplicate = false, implicitResponse = false;
    for (auto& fragment : fragments) {
        if (!retainHandshakeFragment(recordSequence, fragment.get(), immediateAck, duplicate, implicitResponse))
            return;
    }
    if (implicitResponse)
        acknowledgeFlightImplicitly();
    receiveEpoch2->accept(recordSequence);
    noteHandshakeRecord(2, recordSequence, immediateAck);
    if (!owner.containsAssociation(handle))
        return;
    if (duplicate && !serverSide && flight && !flight->isComplete()) {
        owner.cancelEvent(retransmissionTimer.get());
        flight->requestRetransmission();
        burstLimit = burstQueued;
        pumpFlight();
        if (!owner.containsAssociation(handle))
            return;
    }
    scheduleNextHandshake();
}

bool DtlsAssociation::retainHandshakeFragment(uint64_t recordSequence, Packet *body, bool& immediateAck, bool& duplicate, bool& implicitResponse)
{
    if (body->getDataLength() < B(12)) {
        engine.emit(Dtls::recordDroppedSignal, (long)Dtls::MALFORMED_RECORD);
        return false;
    }
    auto header = body->popAtFront<DtlsHandshakeHeader>(B(12));
    auto total = header->getMessageLength();
    auto offset = header->getFragmentOffset();
    auto length = header->getFragmentLength();
    auto messageSequence = header->getMessageSequence();
    if (offset > total || length > total - offset || body->getDataLength() != B(length)) {
        engine.emit(Dtls::recordDroppedSignal, (long)Dtls::MALFORMED_RECORD);
        return false;
    }
    if (state == HandshakeState::ESTABLISHED && messageSequence >= nextPeerMessage)
        return false;
    if (length == 0)
        return false;
    if (!engine.hasConsistentEncoding(body)) {
        engine.emit(Dtls::recordDroppedSignal, (long)Dtls::MALFORMED_RECORD);
        return false;
    }
    auto bytes = body->peekDataAsBytes()->getBytes();
    if (messageSequence < nextPeerMessage) {
        auto old = processedHandshakes.find(messageSequence);
        if (old == processedHandshakes.end())
            return false;
        if (old->second.type != header->getMessageType() || old->second.body.size() != total ||
            !std::equal(bytes.begin(), bytes.end(), old->second.body.begin() + offset)) {
            engine.failAssociation(id, "conflicting retransmitted handshake fragment", 47);
            return false;
        }
        immediateAck = true;
        duplicate = true;
        return true;
    }
    if (processingHandshake && messageSequence == nextPeerMessage) {
        if (header->getMessageType() != processingHandshakeType || total != processingHandshakeBody.size() ||
            !std::equal(bytes.begin(), bytes.end(), processingHandshakeBody.begin() + offset)) {
            engine.failAssociation(id, "conflicting handshake fragment", 47);
            return false;
        }
        immediateAck = true;
        return true;
    }
    bool outOfOrder = messageSequence != nextPeerMessage || offset != reassembly.firstMissing(messageSequence);
    auto oldBytes = reassembly.getReservedBytes();
    auto result = reassembly.insert(header->getMessageType(), messageSequence, total, offset, bytes);
    if (result == DtlsHandshakeReassembly::Result::OUT_OF_RANGE || result == DtlsHandshakeReassembly::Result::EMPTY)
        return false;
    if (result == DtlsHandshakeReassembly::Result::CONFLICT) {
        engine.failAssociation(id, "conflicting handshake fragment", 47);
        return false;
    }
    if (result == DtlsHandshakeReassembly::Result::LIMIT) {
        auto& owner = engine;
        auto handle = id;
        owner.emit(Dtls::recordDroppedSignal, (long)Dtls::RESOURCE_LIMIT);
        if (owner.containsAssociation(handle))
            owner.failAssociation(handle, "DTLS handshake reassembly limit reached");
        return false;
    }
    auto responseBase = serverMessageBase + 1;
    bool clientResponse = clientAuthRequested ?
        (header->getMessageType() == 11 && messageSequence == responseBase) ||
        (header->getMessageType() == 15 && messageSequence == responseBase + 1) ||
        (header->getMessageType() == 20 && messageSequence == responseBase + 2) :
        header->getMessageType() == 20 && messageSequence == responseBase;
    if (serverSide && clientResponse &&
        (state == HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE || state == HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE_VERIFY ||
         state == HandshakeState::SERVER_WAIT_CLIENT_FINISHED))
        implicitResponse = true;
    auto handle = id;
    auto& owner = engine;
    if (oldBytes != reassembly.getReservedBytes()) {
        owner.emitReassemblyBytes();
        if (!owner.containsAssociation(handle))
            return false;
    }
    immediateAck |= outOfOrder;
    if (result == DtlsHandshakeReassembly::Result::COMPLETE)
        completeRecordNumbers[messageSequence] = recordSequence;
    return true;
}

void DtlsAssociation::scheduleNextHandshake()
{
    if (processingHandshake || !reassembly.isComplete(nextPeerMessage))
        return;
    auto messageSequence = nextPeerMessage;
    auto recordSequence = completeRecordNumbers.at(messageSequence);
    completeRecordNumbers.erase(messageSequence);
    auto message = reassembly.take(messageSequence);
    auto& owner = engine;
    auto handle = id;
    owner.emitReassemblyBytes();
    if (!owner.containsAssociation(handle))
        return;
    processingHandshake = true;
    processingHandshakeType = message.type;
    processingHandshakeBody = message.body;
    auto processed = std::make_shared<DtlsHandshakeReassembly::Message>(message);
    auto packet = std::make_shared<std::unique_ptr<Packet>>(new Packet("reassembled DTLS handshake", makeShared<BytesChunk>(message.body)));
    auto header = makeShared<DtlsHandshakeHeader>();
    header->setMessageType(message.type);
    header->setMessageSequence(messageSequence);
    header->setMessageLength(message.body.size());
    header->setFragmentLength(message.body.size());
    (*packet)->insertAtFront(header);
    auto delay = SimTime(engine.par("handshakeInDelay").doubleValue());
    auto bytes = message.body.size() + 12;
    if (!owner.enqueueProcessing(owner.inputProcessing.get(), handle, bytes, delay, [&owner, handle, messageSequence, recordSequence, processed, packet]() {
        auto it = owner.associations.find(handle);
        if (it == owner.associations.end())
            return;
        it->second->processingHandshake = false;
        it->second->processingHandshakeBody.clear();
        try {
            it->second->processHandshake(recordSequence, packet->get());
        }
        catch (const cRuntimeError&) {
            owner.failAssociation(handle, "malformed handshake message", 50);
        }
        it = owner.associations.find(handle);
        if (it != owner.associations.end()) {
            if (it->second->nextPeerMessage > messageSequence)
                it->second->processedHandshakes[messageSequence] = *processed;
            it->second->scheduleNextHandshake();
        }
    }))
        owner.failAssociation(handle, "DTLS input processing queue is full");
}

int DtlsAssociation::validateClientHello(const Ptr<const DtlsClientHello>& hello, bool certificateProfile, const std::string& identity,
                                         tls::TlsHandshakeParameters::Offer& offer, tls::TlsHandshakeParameters::Selection& selection)
{
    using Parameters = tls::TlsHandshakeParameters;
    if (hello->getLegacyVersion() != 0xfefd || hello->getLegacyCookieArraySize() != 0 ||
        hello->getCompressionMethodsArraySize() != 1 || hello->getCompressionMethods(0) != 0)
        return 47;
    if (!Parameters::decodeOffer(hello, offer).empty())
        return 50;
    if (offer.hasSupportedGroups != offer.hasKeyShare)
        return 109;
    bool offersPskDhe = std::find(offer.pskModes.begin(), offer.pskModes.end(), Parameters::PSK_DHE_KE) != offer.pskModes.end();
    if ((certificateProfile && (!offer.hasSupportedGroups || !offer.hasKeyShare || !offer.hasSignatureAlgorithms)) ||
        (offer.hasPsk && !offer.hasPskModes) || (offer.hasPsk && offersPskDhe && (!offer.hasSupportedGroups || !offer.hasKeyShare)))
        return 109;
    for (auto group : offer.keyShares)
        if (std::find(offer.groups.begin(), offer.groups.end(), group) == offer.groups.end())
            return 47;
    auto error = Parameters::select(offer, certificateProfile ? Parameters::Profile::CERTIFICATE : Parameters::Profile::PSK, identity, selection);
    if ((!certificateProfile && offer.pskIdentity != identity) || (certificateProfile && offer.hasPsk) || !error.empty())
        return error == "no supported DTLS version" ? 70 : 40;
    return 0;
}

static void appendUint16(std::vector<uint8_t>& bytes, size_t value)
{
    if (value > 0xffff)
        throw cRuntimeError("TLS vector exceeds its 16-bit length");
    bytes.push_back(value >> 8);
    bytes.push_back(value & 0xff);
}

static void appendExtension(std::vector<uint8_t>& extensions, uint16_t type, const std::vector<uint8_t>& content)
{
    if (content.size() > 65535)
        throw cRuntimeError("TLS extension exceeds its 16-bit length");
    extensions.push_back(type >> 8);
    extensions.push_back(type & 0xff);
    extensions.push_back(content.size() >> 8);
    extensions.push_back(content.size() & 0xff);
    extensions.insert(extensions.end(), content.begin(), content.end());
}

void DtlsAssociation::sendClientHello(const std::vector<uint8_t>& cookie)
{
    using Parameters = tls::TlsHandshakeParameters;
    auto& owner = engine;
    auto handle = id;
    std::string profile = owner.par("profile").stringValue();
    if (profile != "psk" && profile != "certificate")
        throw cRuntimeError("Unknown DTLS profile '%s'", profile.c_str());
    auto hello = makeShared<DtlsClientHello>();
    for (size_t i = 0; i < 32; ++i) {
        if (!retried) {
            clientRandom[i] = owner.intuniform(0, 255);
            clientShare[i] = owner.intuniform(0, 255);
        }
        hello->setRandom(i, clientRandom[i]);
    }
    hello->setCipherSuitesArraySize(1);
    hello->setCipherSuites(0, Parameters::AES_128_GCM_SHA256);
    hello->setCompressionMethodsArraySize(1);
    hello->setCompressionMethods(0, 0);
    std::vector<uint8_t> extensions;
    appendExtension(extensions, 43, {2, 0xfe, 0xfc});
    appendExtension(extensions, 10, {0, 2, 0, 0x1d});
    appendExtension(extensions, 13, {0, 2, 4, 3});
    std::vector<uint8_t> share = {0, 36, 0, 0x1d, 0, 32};
    share.insert(share.end(), clientShare.begin(), clientShare.end());
    if (owner.par("offerInitialKeyShare").boolValue() || retried)
        appendExtension(extensions, 51, share);
    else
        appendExtension(extensions, 51, {0, 0});
    if (!cookie.empty()) {
        std::vector<uint8_t> cookieExtension;
        appendUint16(cookieExtension, cookie.size());
        cookieExtension.insert(cookieExtension.end(), cookie.begin(), cookie.end());
        appendExtension(extensions, 44, cookieExtension);
    }
    if (profile == "psk") {
        appendExtension(extensions, 45, {1, Parameters::PSK_DHE_KE});
        std::string identity = owner.par("identity").stringValue();
        if (identity.empty() || identity.size() > 65526)
            throw cRuntimeError("DTLS PSK identity has an invalid length");
        std::vector<uint8_t> psk;
        appendUint16(psk, identity.size() + 6);
        appendUint16(psk, identity.size());
        psk.insert(psk.end(), identity.begin(), identity.end());
        psk.insert(psk.end(), 4, 0);
        appendUint16(psk, 33);
        psk.push_back(32);
        psk.insert(psk.end(), 32, 0); // modeled binder, not a MAC
        appendExtension(extensions, 41, psk);
    }
    if (extensions.size() > 0xffff)
        throw cRuntimeError("DTLS ClientHello extensions are too long");
    hello->setExtensionsArraySize(extensions.size());
    for (size_t i = 0; i < extensions.size(); ++i)
        hello->setExtensions(i, extensions[i]);
    hello->setChunkLength(B(44 + extensions.size()));
    auto decodeError = Parameters::decodeOffer(hello, offer);
    if (!decodeError.empty())
        throw cRuntimeError("Invalid locally generated ClientHello: %s", decodeError.c_str());
    beginFlight();
    owner.sendPlainHandshake(handle, 1, retried ? 1 : 0, hello);
    if (owner.containsAssociation(handle))
        finishFlight();
}

void DtlsAssociation::sendServerFlight()
{
    if (serverFlightQueued)
        return;
    auto& owner = engine;
    auto handle = id;
    bool useCertificate = certificateProfile;
    bool clientAuth = clientAuthRequested;
    Ptr<Chunk> certificate;
    Ptr<Chunk> verify;
    auto udpLimit = owner.par("udpPayloadLimit").intValue();
    auto wireSize = [udpLimit](size_t length, int overhead) {
        if (udpLimit <= overhead)
            throw cRuntimeError("DTLS UDP payload limit cannot carry a handshake fragment");
        size_t maximum = std::min<size_t>(udpLimit - overhead, DtlsRecordProtection::MAX_CONTENT_BYTES - 12);
        return length + ((length + maximum - 1) / maximum) * overhead;
    };
    size_t flightBytes = wireSize(40 + 46 + (useCertificate ? 0 : 6), 25) + wireSize(2, 34) + wireSize(32, 34);
    if (useCertificate) {
        certificate = owner.makeCertificate();
        verify = owner.makeCertificateVerify();
        flightBytes += wireSize(certificate->getChunkLength().get<B>(), 34) + wireSize(verify->getChunkLength().get<B>(), 34);
        if (clientAuth)
            flightBytes += wireSize(11, 34);
    }
    if (!addressValidated) {
        auto credit = receivedBytes > std::numeric_limits<size_t>::max() / 3 ? std::numeric_limits<size_t>::max() : receivedBytes * 3;
        if (sentBytes > credit || reservedBytes > credit - sentBytes || flightBytes > credit - sentBytes - reservedBytes)
            return;
    }
    auto hello = makeShared<tls::TlsServerHello>();
    hello->setLegacyVersion(0xfefd);
    for (size_t i = 0; i < 32; ++i)
        hello->setRandom(i, owner.intuniform(0, 255));
    std::vector<uint8_t> extensions;
    appendExtension(extensions, 43, {0xfe, 0xfc});
    std::vector<uint8_t> share = {0, 0x1d, 0, 32};
    for (int i = 0; i < 32; ++i)
        share.push_back(owner.intuniform(0, 255));
    appendExtension(extensions, 51, share);
    if (!useCertificate)
        appendExtension(extensions, 41, {0, 0});
    hello->setExtensionsArraySize(extensions.size());
    for (size_t i = 0; i < extensions.size(); ++i)
        hello->setExtensions(i, extensions[i]);
    hello->setChunkLength(B(40 + extensions.size()));
    reservedBytes += flightBytes;
    serverFlightQueued = true;
    auto base = serverMessageBase;
    beginFlight();
    owner.sendPlainHandshake(handle, 2, base, hello);
    if (!owner.containsAssociation(handle))
        return;
    auto encryptedExtensions = makeShared<tls::TlsEncryptedExtensions>();
    encryptedExtensions->setChunkLength(B(2));
    uint16_t messageSequence = base + 1;
    owner.sendProtectedHandshake(handle, 8, messageSequence++, encryptedExtensions);
    if (!owner.containsAssociation(handle))
        return;
    if (useCertificate && clientAuth) {
        auto request = makeShared<tls::TlsCertificateRequest>();
        const std::vector<uint8_t> signatureExtension = {0, 13, 0, 4, 0, 2, 4, 3};
        request->setExtensionsArraySize(signatureExtension.size());
        for (size_t i = 0; i < signatureExtension.size(); ++i)
            request->setExtensions(i, signatureExtension[i]);
        request->setChunkLength(B(11));
        owner.sendProtectedHandshake(handle, 13, messageSequence++, request);
        if (!owner.containsAssociation(handle))
            return;
    }
    if (useCertificate) {
        owner.sendProtectedHandshake(handle, 11, messageSequence++, certificate);
        if (!owner.containsAssociation(handle))
            return;
        owner.sendProtectedHandshake(handle, 15, messageSequence++, verify);
        if (!owner.containsAssociation(handle))
            return;
    }
    auto finished = makeShared<tls::TlsFinished>();
    finished->setVerifyDataArraySize(32);
    finished->setChunkLength(B(32));
    nextLocalMessage = messageSequence + 1;
    owner.sendProtectedHandshake(handle, 20, messageSequence, finished, false, true);
    if (owner.containsAssociation(handle))
        finishFlight();
}

void DtlsAssociation::serverFinishedHandedOff()
{
    ownFinishedHandedOff = true;
    if (peerFinishedProcessed) {
        auto& owner = engine;
        auto handle = id;
        auto sequence = peerFinishedRecord;
        owner.sendFinalAck(handle, sequence);
        owner.establish(handle);
    }
}

void DtlsAssociation::processServerHello(uint16_t messageSequence, Packet *body, const L3Address& destination, int destinationPort)
{
    using Parameters = tls::TlsHandshakeParameters;
    if (state != HandshakeState::CLIENT_WAIT_SERVER_HELLO ||
        (messageSequence != serverMessageBase && !(retried && messageSequence == 0)))
        return;
    auto handle = id;
    auto& owner = engine;
    localAddress = destination;
    localPort = destinationPort;
    if (!owner.hasConsistentEncoding(body)) {
        owner.failAssociation(handle, "inconsistent ServerHello encoding", 50);
        return;
    }
    Ptr<const tls::TlsServerHello> hello;
    try {
        hello = body->popAtFront<tls::TlsServerHello>(body->getDataLength());
    }
    catch (const cRuntimeError&) {
        owner.failAssociation(handle, "malformed ServerHello", 50);
        return;
    }
    if (body->getDataLength() != b(0)) {
        owner.failAssociation(handle, "trailing ServerHello data", 50);
        return;
    }
    if (hello->getLegacyVersion() != 0xfefd || hello->getCompressionMethod() != 0 || hello->getSessionIdArraySize() != 0 ||
        hello->getCipherSuite() != Parameters::AES_128_GCM_SHA256) {
        owner.failAssociation(handle, "invalid or unoffered server selection", 47);
        return;
    }
    bool isRetry = true;
    for (size_t i = 0; i < 32; ++i)
        isRetry &= hello->getRandom(i) == HELLO_RETRY_RANDOM[i];
    bool version = false, share = false, psk = false, pskPresent = false, requestedShare = false;
    std::vector<uint8_t> cookie;
    std::set<uint16_t> seen;
    size_t pos = 0;
    while (pos < hello->getExtensionsArraySize()) {
        auto end = hello->getExtensionsArraySize();
        if (end - pos < 4) {
            owner.failAssociation(handle, "truncated ServerHello extension", 50);
            return;
        }
        uint16_t type = (uint16_t(hello->getExtensions(pos)) << 8) | hello->getExtensions(pos + 1);
        uint16_t length = (uint16_t(hello->getExtensions(pos + 2)) << 8) | hello->getExtensions(pos + 3);
        pos += 4;
        if (length > end - pos) {
            owner.failAssociation(handle, "truncated ServerHello extension", 50);
            return;
        }
        if (!seen.insert(type).second) {
            owner.failAssociation(handle, "duplicate ServerHello extension", 47);
            return;
        }
        if (type == 43) {
            if (length != 2) {
                owner.failAssociation(handle, "invalid selected version length", 50);
                return;
            }
            version = hello->getExtensions(pos) == 0xfe && hello->getExtensions(pos + 1) == 0xfc;
        }
        else if (type == 51) {
            if (isRetry) {
                if (length != 2) {
                    owner.failAssociation(handle, "invalid retry key share length", 50);
                    return;
                }
                requestedShare = hello->getExtensions(pos) == 0 && hello->getExtensions(pos + 1) == 0x1d;
                if (!requestedShare) {
                    owner.failAssociation(handle, "unsupported retry key share", 47);
                    return;
                }
            }
            else {
                if (length < 4 || ((uint16_t(hello->getExtensions(pos + 2)) << 8) | hello->getExtensions(pos + 3)) != length - 4) {
                    owner.failAssociation(handle, "malformed server key share", 50);
                    return;
                }
                share = length == 36 && hello->getExtensions(pos) == 0 && hello->getExtensions(pos + 1) == 0x1d;
            }
        }
        else if (type == 41 && !isRetry) {
            pskPresent = true;
            if (length != 2) {
                owner.failAssociation(handle, "invalid selected PSK length", 50);
                return;
            }
            psk = hello->getExtensions(pos) == 0 && hello->getExtensions(pos + 1) == 0;
        }
        else if (type == 44 && isRetry) {
            if (length < 3 || ((uint16_t(hello->getExtensions(pos)) << 8) | hello->getExtensions(pos + 1)) != length - 2) {
                owner.failAssociation(handle, "malformed retry cookie", 50);
                return;
            }
            for (size_t i = pos + 2; i < pos + length; ++i)
                cookie.push_back(hello->getExtensions(i));
        }
        else {
            owner.failAssociation(handle, "unsupported ServerHello extension", 110);
            return;
        }
        pos += length;
    }
    if (isRetry) {
        if (retried) {
            if (messageSequence != 0 || !version || psk || requestedShare != retryRequestedShare || cookie != retryCookie)
                owner.failAssociation(handle, "second HelloRetryRequest", 10);
            return;
        }
        bool alreadyShared = std::find(offer.keyShares.begin(), offer.keyShares.end(), Parameters::X25519) != offer.keyShares.end();
        if (!version || psk || (cookie.empty() && !requestedShare) || (requestedShare && alreadyShared) ||
            (!requestedShare && !alreadyShared)) {
            owner.failAssociation(handle, "invalid HelloRetryRequest", 47);
            return;
        }
        retried = true;
        retryRequestedShare = requestedShare;
        retryCookie = cookie;
        serverMessageBase = 1;
        acknowledgeFlightImplicitly();
        owner.sendClientHello(handle, cookie);
        return;
    }
    if (messageSequence != serverMessageBase)
        return;
    Parameters::Selection selection;
    selection.usePsk = !certificateProfile;
    selection.signature = certificateProfile ? Parameters::ECDSA_P256_SHA256 : 0;
    if (!version || !share || pskPresent != selection.usePsk || (pskPresent && !psk) || !Parameters::wasOffered(offer, selection)) {
        owner.failAssociation(handle, "invalid or unoffered server selection", 47);
        return;
    }
    state = HandshakeState::CLIENT_WAIT_SERVER_EXTENSIONS;
    nextPeerMessage = serverMessageBase + 1;
    acknowledgeFlightImplicitly();
}

void DtlsAssociation::processHandshake(uint64_t sequence, Packet *body)
{
    if (body->getDataLength() < B(12)) {
        engine.emit(Dtls::recordDroppedSignal, (long)Dtls::MALFORMED_RECORD);
        return;
    }
    auto handshake = body->popAtFront<DtlsHandshakeHeader>(B(12));
    if (handshake->getFragmentOffset() != 0 || handshake->getFragmentLength() != handshake->getMessageLength() ||
        body->getDataLength() != B(handshake->getMessageLength()) || handshake->getMessageSequence() != nextPeerMessage)
        return;
    if (!engine.hasConsistentEncoding(body)) {
        engine.failAssociation(id, "inconsistent handshake body encoding", 50);
        return;
    }
    if (state == HandshakeState::CLIENT_WAIT_SERVER_EXTENSIONS && handshake->getMessageType() == 8) {
        Ptr<const tls::TlsEncryptedExtensions> extensions;
        try {
            extensions = body->popAtFront<tls::TlsEncryptedExtensions>(body->getDataLength());
        }
        catch (const cRuntimeError&) {
            engine.failAssociation(id, "malformed encrypted extensions", 50);
            return;
        }
        if (body->getDataLength() != b(0)) {
            engine.failAssociation(id, "trailing encrypted extensions data", 50);
            return;
        }
        if (extensions->getExtensionsArraySize() != 0) {
            size_t pos = 0, end = extensions->getExtensionsArraySize();
            std::set<uint16_t> seen;
            while (pos < end) {
                if (end - pos < 4) {
                    engine.failAssociation(id, "truncated encrypted extension", 50);
                    return;
                }
                auto type = (uint16_t(extensions->getExtensions(pos)) << 8) | extensions->getExtensions(pos + 1);
                auto length = (uint16_t(extensions->getExtensions(pos + 2)) << 8) | extensions->getExtensions(pos + 3);
                pos += 4;
                if (length > end - pos) {
                    engine.failAssociation(id, "truncated encrypted extension", 50);
                    return;
                }
                if (!seen.insert(type).second) {
                    engine.failAssociation(id, "duplicate encrypted extension", 47);
                    return;
                }
                pos += length;
            }
            engine.failAssociation(id, "unsupported encrypted extension", 110);
            return;
        }
        ++nextPeerMessage;
        state = certificateProfile ? HandshakeState::CLIENT_WAIT_SERVER_CERTIFICATE : HandshakeState::CLIENT_WAIT_SERVER_FINISHED;
    }
    else if (state == HandshakeState::CLIENT_WAIT_SERVER_CERTIFICATE && handshake->getMessageType() == 13 && !clientAuthRequested) {
        Ptr<const tls::TlsCertificateRequest> request;
        try {
            request = body->popAtFront<tls::TlsCertificateRequest>(body->getDataLength());
        }
        catch (const cRuntimeError&) {
            engine.failAssociation(id, "malformed CertificateRequest", 50);
            return;
        }
        if (body->getDataLength() != b(0)) {
            engine.failAssociation(id, "trailing CertificateRequest data", 50);
            return;
        }
        if (request->getRequestContextArraySize() != 0) {
            engine.failAssociation(id, "invalid handshake CertificateRequest context", 47);
            return;
        }
        bool compatible = false;
        size_t pos = 0, end = request->getExtensionsArraySize();
        std::set<uint16_t> seen;
        while (pos < end) {
            if (end - pos < 4) {
                engine.failAssociation(id, "truncated CertificateRequest extension", 50);
                return;
            }
            auto type = (uint16_t(request->getExtensions(pos)) << 8) | request->getExtensions(pos + 1);
            auto length = (uint16_t(request->getExtensions(pos + 2)) << 8) | request->getExtensions(pos + 3);
            pos += 4;
            if (length > end - pos) {
                engine.failAssociation(id, "truncated CertificateRequest extension", 50);
                return;
            }
            if (!seen.insert(type).second) {
                engine.failAssociation(id, "duplicate CertificateRequest extension", 47);
                return;
            }
            if (type == 13) {
                if (length < 4) {
                    engine.failAssociation(id, "invalid signature algorithms vector", 50);
                    return;
                }
                auto vectorLength = (uint16_t(request->getExtensions(pos)) << 8) | request->getExtensions(pos + 1);
                if (vectorLength != length - 2 || vectorLength % 2) {
                    engine.failAssociation(id, "invalid signature algorithms vector", 50);
                    return;
                }
                for (size_t i = pos + 2; i < pos + length; i += 2)
                    compatible |= request->getExtensions(i) == 4 && request->getExtensions(i + 1) == 3;
            }
            pos += length;
        }
        if (!compatible) {
            engine.failAssociation(id, "no compatible requested signature algorithm", seen.count(13) ? 40 : 109);
            return;
        }
        ++nextPeerMessage;
        clientAuthRequested = true;
    }
    else if (state == HandshakeState::CLIENT_WAIT_SERVER_CERTIFICATE && handshake->getMessageType() == 11) {
        auto alert = engine.checkCertificate(body);
        if (alert != 0) {
            engine.failAssociation(id, "invalid peer certificate", alert);
            return;
        }
        ++nextPeerMessage;
        state = HandshakeState::CLIENT_WAIT_SERVER_CERTIFICATE_VERIFY;
    }
    else if (state == HandshakeState::CLIENT_WAIT_SERVER_CERTIFICATE_VERIFY && handshake->getMessageType() == 15) {
        auto alert = engine.checkCertificateVerify(body);
        if (alert != 0) {
            engine.failAssociation(id, "invalid certificate signature", alert);
            return;
        }
        ++nextPeerMessage;
        state = HandshakeState::CLIENT_WAIT_SERVER_FINISHED;
    }
    else if (state == HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE && handshake->getMessageType() == 11) {
        auto alert = engine.checkCertificate(body);
        if (alert != 0) {
            engine.failAssociation(id, "invalid peer certificate", alert);
            return;
        }
        ++nextPeerMessage;
        state = HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE_VERIFY;
    }
    else if (state == HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE_VERIFY && handshake->getMessageType() == 15) {
        auto alert = engine.checkCertificateVerify(body);
        if (alert != 0) {
            engine.failAssociation(id, "invalid certificate signature", alert);
            return;
        }
        ++nextPeerMessage;
        state = HandshakeState::SERVER_WAIT_CLIENT_FINISHED;
    }
    else if (state == HandshakeState::CLIENT_WAIT_SERVER_FINISHED && handshake->getMessageType() == 20) {
        if (body->getDataLength() != B(32)) {
            engine.failAssociation(id, "invalid Finished length", 50);
            return;
        }
        ++nextPeerMessage;
        state = HandshakeState::CLIENT_WAIT_FINISHED_HANDOFF;
        clearAcknowledgements();
        auto handle = id;
        auto& owner = engine;
        auto nextMessage = serverMessageBase + 1;
        bool sendClientCertificate = clientAuthRequested;
        beginFlight();
        if (sendClientCertificate) {
            owner.sendProtectedHandshake(handle, 11, nextMessage++, owner.makeCertificate());
            if (!owner.containsAssociation(handle))
                return;
            owner.sendProtectedHandshake(handle, 15, nextMessage++, owner.makeCertificateVerify());
            if (!owner.containsAssociation(handle))
                return;
        }
        auto finished = makeShared<tls::TlsFinished>();
        finished->setVerifyDataArraySize(32);
        finished->setChunkLength(B(32));
        nextLocalMessage = nextMessage + 1;
        owner.sendProtectedHandshake(handle, 20, nextMessage, finished, true);
        if (owner.containsAssociation(handle))
            finishFlight();
    }
    else if (state == HandshakeState::SERVER_WAIT_CLIENT_FINISHED && handshake->getMessageType() == 20) {
        if (peerFinishedProcessed)
            return;
        if (body->getDataLength() != B(32)) {
            engine.failAssociation(id, "invalid Finished length", 50);
            return;
        }
        ++nextPeerMessage;
        peerFinishedProcessed = true;
        addressValidated = true;
        peerFinishedRecord = sequence;
        acknowledgeFlightImplicitly();
        if (ownFinishedHandedOff) {
            auto handle = id;
            auto& owner = engine;
            owner.sendFinalAck(handle, sequence);
            owner.establish(handle);
        }
    }
    else
        engine.failAssociation(id, "unexpected handshake message", 10);
}

} // namespace inet::dtls
