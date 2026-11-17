// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include <algorithm>
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"

namespace inet::dtls {

std::vector<uint8_t> Dtls::allocateConnectionId(AssociationId id)
{
    if (nextConnectionId == 0)
        return {};
    auto value = nextConnectionId++;
    std::vector<uint8_t> cid;
    for (int shift = 56; shift >= 0; shift -= 8)
        cid.push_back(value >> shift);
    return cidIndex.emplace(cid, id).second ? cid : std::vector<uint8_t>{};
}

void Dtls::removeConnectionIds(AssociationId id)
{
    for (auto it = cidIndex.begin(); it != cidIndex.end();)
        if (it->second == id)
            it = cidIndex.erase(it);
        else
            ++it;
}

size_t Dtls::countConnectionIds(AssociationId id) const
{
    return std::count_if(cidIndex.begin(), cidIndex.end(), [id](const auto& entry) { return entry.second == id; });
}

bool Dtls::requestConnectionIds(AssociationId id, int count)
{
    Enter_Method("requestConnectionIds");
    auto it = associations.find(id);
    return operationalState == OPERATING && it != associations.end() && it->second->requestConnectionIds(count);
}

bool Dtls::provideConnectionIds(AssociationId id, int count, bool immediate)
{
    Enter_Method("provideConnectionIds");
    auto it = associations.find(id);
    return operationalState == OPERATING && it != associations.end() && it->second->provideConnectionIds(count, immediate);
}

bool Dtls::activateSpareConnectionId(AssociationId id)
{
    Enter_Method("activateSpareConnectionId");
    auto it = associations.find(id);
    return operationalState == OPERATING && it != associations.end() && it->second->activateSpareConnectionId();
}

bool DtlsAssociation::requestConnectionIds(int count)
{
    if (isDtls12())
        return false;
    auto existing = postHandshakeExchanges.find(9);
    if (state != HandshakeState::ESTABLISHED || closing || !cidNegotiated || peerCid.empty() ||
        count < 1 || count > 255 || cidRequestUnfulfilled ||
        (existing != postHandshakeExchanges.end() && existing->second->flight))
        return false;
    auto body = makeShared<DtlsRequestConnectionId>();
    body->setNumCids(count);
    body->markImmutable();
    cidRequestUnfulfilled = true;
    if (localMessagesExhausted) {
        cidRequestUnfulfilled = false;
        return false;
    }
    auto& owner = engine;
    auto handle = id;
    if (startPostHandshake(9, body, "ConnectionIdRequest"))
        return true;
    auto it = owner.associations.find(handle);
    if (it != owner.associations.end())
        it->second->cidRequestUnfulfilled = false;
    return false;
}

bool DtlsAssociation::provideConnectionIds(int count, bool immediate)
{
    if (isDtls12())
        return false;
    auto existing = postHandshakeExchanges.find(10);
    if (state != HandshakeState::ESTABLISHED || closing || !cidNegotiated || localCid.empty() ||
        localMessagesExhausted || count < 0 || count > 255 || (immediate && count == 0) ||
        (existing != postHandshakeExchanges.end() && existing->second->flight))
        return false;
    // Leave one receive-index slot available for an immediate replacement.
    auto capacity = engine.par("maxReceiveConnectionIds").intValue();
    auto available = std::max<int64_t>(0, capacity - int64_t(engine.countConnectionIds(id)) - (immediate ? 0 : 1));
    if (count > available)
        return false;
    std::vector<std::vector<uint8_t>> allocated;
    for (int i = 0; i < count; ++i) {
        auto cid = engine.allocateConnectionId(id);
        if (cid.empty()) {
            for (const auto& value : allocated)
                engine.cidIndex.erase(value);
            return false;
        }
        allocated.push_back(cid);
    }
    std::vector<uint8_t> bytes;
    for (const auto& cid : allocated) {
        bytes.push_back(cid.size());
        bytes.insert(bytes.end(), cid.begin(), cid.end());
    }
    auto body = makeShared<DtlsNewConnectionId>();
    body->setConnectionIdsArraySize(bytes.size());
    for (size_t i = 0; i < bytes.size(); ++i)
        body->setConnectionIds(i, bytes[i]);
    body->setUsage(immediate ? 0 : 1);
    body->setChunkLength(B(3 + bytes.size()));
    body->markImmutable();
    pendingLocalCids = std::move(allocated);
    pendingCidsImmediate = immediate;
    auto& owner = engine;
    auto handle = id;
    if (startPostHandshake(10, body, "ConnectionIdUpdate"))
        return true;
    auto it = owner.associations.find(handle);
    if (it != owner.associations.end()) {
        for (const auto& cid : it->second->pendingLocalCids)
            owner.cidIndex.erase(cid);
        it->second->pendingLocalCids.clear();
        it->second->pendingCidsImmediate = false;
    }
    return false;
}

bool DtlsAssociation::activateSpareConnectionId()
{
    if (isDtls12())
        return false;
    if (state != HandshakeState::ESTABLISHED || closing || !cidNegotiated || sparePeerCids.empty())
        return false;
    auto& owner = engine;
    auto handle = id;
    auto maximum = owner.getMaximumDatagramSize(handle);
    peerCid = std::move(sparePeerCids.front());
    sparePeerCids.pop_front();
    owner.emit(cComponent::registerSignal("connectionIdChanged"), (long)handle);
    auto it = owner.associations.find(handle);
    if (it != owner.associations.end())
        it->second->notifyPathMtu(maximum);
    return true;
}

void DtlsAssociation::finishConnectionIdUpdate()
{
    if (pendingCidsImmediate && !pendingLocalCids.empty()) {
        for (auto it = engine.cidIndex.begin(); it != engine.cidIndex.end();)
            if (it->second == id && std::find(pendingLocalCids.begin(), pendingLocalCids.end(), it->first) == pendingLocalCids.end())
                it = engine.cidIndex.erase(it);
            else
                ++it;
        localCid = pendingLocalCids.front();
    }
    pendingLocalCids.clear();
    pendingCidsImmediate = false;
    if (deferredPeerCidRequest) {
        auto count = *deferredPeerCidRequest;
        deferredPeerCidRequest.reset();
        serveConnectionIdRequest(count);
    }
}

void DtlsAssociation::serveConnectionIdRequest(uint8_t count)
{
    auto existing = postHandshakeExchanges.find(10);
    if (existing != postHandshakeExchanges.end() && existing->second->flight) {
        if (deferredPeerCidRequest)
            engine.failAssociation(id, "excessive DTLS connection ID requests", 52);
        else
            deferredPeerCidRequest = count;
        return;
    }
    auto available = std::max<int64_t>(0, engine.par("maxReceiveConnectionIds").intValue() - int64_t(engine.countConnectionIds(id)) - 1);
    auto& owner = engine;
    auto handle = id;
    if (!provideConnectionIds(std::min<int64_t>(count, available), false) && owner.containsAssociation(handle))
        owner.failAssociation(handle, "DTLS connection ID response could not start");
}

void DtlsAssociation::receiveConnectionIdMessage(uint64_t epoch, uint64_t number, Packet *body)
{
    if (body->getDataLength() < B(12) || !engine.hasConsistentEncoding(body))
        return;
    auto header = body->popAtFront<DtlsHandshakeHeader>(B(12));
    auto type = header->getMessageType();
    if (type != 9 && type != 10)
        return;
    if (!cidNegotiated || (type == 9 ? localCid.empty() : peerCid.empty())) {
        engine.failAssociation(id, "DTLS connection ID update was not negotiated", 10);
        return;
    }
    auto sequence = header->getMessageSequence();
    auto bytes = body->peekDataAsBytes()->getBytes();
    if (header->getFragmentLength() != bytes.size() || sequence > nextPeerMessage)
        return;
    auto replay = epoch == receiveEpoch ? receiveEpoch3.get() : previousReceiveEpoch.get();
    if (!replay)
        return;
    if (sequence < nextPeerMessage) {
        auto old = lastPeerCidMessages.find(type);
        if (old == lastPeerCidMessages.end() || old->second.first != sequence)
            return;
        auto offset = header->getFragmentOffset();
        const auto& original = old->second.second;
        if (header->getMessageLength() != original.size() || offset > original.size() || bytes.size() > original.size() - offset ||
            !std::equal(bytes.begin(), bytes.end(), original.begin() + offset)) {
            engine.failAssociation(id, "conflicting DTLS connection ID duplicate", 47);
            return;
        }
        if (replay->accept(number))
            noteHandshakeRecord(epoch, number, true);
        return;
    }
    auto result = reassembly.insert(type, sequence, header->getMessageLength(), header->getFragmentOffset(), bytes);
    if (result == DtlsHandshakeReassembly::Result::CONFLICT) {
        engine.failAssociation(id, "conflicting DTLS connection ID fragments", 47);
        return;
    }
    if (result != DtlsHandshakeReassembly::Result::INCOMPLETE && result != DtlsHandshakeReassembly::Result::COMPLETE)
        return;
    if (!replay->accept(number))
        return;
    auto key = uint16_t(256 | type);
    auto incoming = postHandshakeExchanges.find(key);
    if (incoming == postHandshakeExchanges.end())
        incoming = postHandshakeExchanges.emplace(key, std::make_unique<DtlsPostHandshakeExchange>(engine, "ConnectionIdReceive")).first;
    auto& exchange = *incoming->second;
    if (!exchange.receivingSequence) {
        exchange.receivingSequence = sequence;
        engine.scheduleAt(simTime() + SimTime(engine.par("handshakeTimeout").doubleValue()), exchange.deadline.get());
    }
    auto handle = id;
    auto& owner = engine;
    if (result == DtlsHandshakeReassembly::Result::COMPLETE && !exchange.receivingQueued) {
        exchange.receivingQueued = true;
        auto delay = owner.processingDelay(handle, "handshakeInDelay");
        if (!owner.inputProcessing->enqueueContinuation(handle, header->getMessageLength() + 12, delay, [&owner, handle, type, sequence]() {
            auto it = owner.associations.find(handle);
            if (it != owner.associations.end())
                it->second->applyConnectionIdMessage(type, sequence);
        }, {}, {}, [&owner, delay]() { owner.emit(Dtls::processingDelaySignal, delay.dbl()); })) {
            exchange.receivingQueued = false;
            return; // retain the complete body, but leave its last record unacknowledged
        }
    }
    auto current = owner.associations.find(handle);
    if (current != owner.associations.end())
        current->second->noteHandshakeRecord(epoch, number, true);
}

void DtlsAssociation::applyConnectionIdMessage(uint8_t type, uint16_t sequence)
{
    if (nextPeerMessage != sequence || !reassembly.isComplete(sequence))
        return;
    auto bytes = reassembly.take(sequence).body;
    auto& incoming = *postHandshakeExchanges.at(256 | type);
    engine.cancelEvent(incoming.deadline.get());
    incoming.receivingSequence.reset();
    incoming.receivingQueued = false;
    if (type == 9) {
        if (bytes.size() != 1) {
            engine.failAssociation(id, "malformed DTLS connection ID request", 50);
            return;
        }
        lastPeerCidMessages[type] = {sequence, bytes};
        ++nextPeerMessage;
        serveConnectionIdRequest(bytes[0]);
        return;
    }
    if (bytes.size() < 3 || size_t((uint16_t(bytes[0]) << 8) | bytes[1]) != bytes.size() - 3) {
        engine.failAssociation(id, "malformed DTLS connection ID vector", 50);
        return;
    }
    auto usage = bytes.back();
    std::vector<std::vector<uint8_t>> ids;
    for (size_t offset = 2; offset < bytes.size() - 1;) {
        auto length = bytes[offset++];
        if (length > bytes.size() - 1 - offset) {
            engine.failAssociation(id, "truncated DTLS connection ID", 50);
            return;
        }
        std::vector<uint8_t> cid(bytes.begin() + offset, bytes.begin() + offset + length);
        offset += length;
        if (std::find(ids.begin(), ids.end(), cid) == ids.end())
            ids.push_back(std::move(cid));
    }
    if (usage > 1 || (usage == 0 && ids.empty())) {
        engine.failAssociation(id, "invalid DTLS connection ID usage", 47);
        return;
    }
    lastPeerCidMessages[type] = {sequence, bytes};
    ++nextPeerMessage;
    bool changed = usage == 0;
    auto maximum = engine.getMaximumDatagramSize(id);
    if (changed) {
        peerCid = ids.front();
        sparePeerCids.clear();
        if (candidatePath)
            candidatePath->peerCid = peerCid;
    }
    auto capacity = engine.par("maxReceiveConnectionIds").intValue();
    for (const auto& cid : ids)
        if (cid != peerCid && (!candidatePath || cid != candidatePath->peerCid) &&
            std::find(sparePeerCids.begin(), sparePeerCids.end(), cid) == sparePeerCids.end() &&
            sparePeerCids.size() + 1 + (candidatePath && candidatePath->peerCid != peerCid ? 1 : 0) < size_t(capacity))
            sparePeerCids.push_back(cid);
    cidRequestUnfulfilled = false;
    auto request = postHandshakeExchanges.find(9);
    if (request != postHandshakeExchanges.end() && !request->second->flight)
        engine.cancelEvent(request->second->deadline.get());
    auto handle = id;
    auto& owner = engine;
    owner.emit(cComponent::registerSignal("connectionIdsReceived"), (long)ids.size());
    if (changed && owner.containsAssociation(handle))
        owner.emit(cComponent::registerSignal("connectionIdChanged"), (long)handle);
    auto it = owner.associations.find(handle);
    if (changed && it != owner.associations.end())
        it->second->notifyPathMtu(maximum);
}

Ptr<DtlsCiphertextHeader> DtlsAssociation::makeRecordHeader(uint64_t epoch, uint64_t number, uint16_t length, const std::vector<uint8_t> *cidOverride) const
{
    auto header = makeShared<DtlsCiphertextHeader>();
    header->setEpochBits(epoch & 3);
    header->setSequenceNumber(number & 0xffff);
    header->setLength(length);
    if (cidNegotiated) {
        const auto& cid = cidOverride ? *cidOverride : peerCid;
        header->setConnectionIdArraySize(cid.size());
        for (size_t i = 0; i < cid.size(); ++i)
            header->setConnectionId(i, cid[i]);
    }
    header->setChunkLength(B(5 + header->getConnectionIdArraySize()));
    return header;
}

} // namespace inet::dtls
