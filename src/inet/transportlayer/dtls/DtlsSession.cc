// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include <algorithm>
#include <limits>

#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"
#include "inet/transportlayer/dtls/DtlsRecordProtection.h"

namespace inet::dtls {

DtlsPostHandshakeExchange *DtlsAssociation::findPostHandshakeExchange(const std::shared_ptr<DtlsHandshakeFlight>& value)
{
    for (auto& entry : postHandshakeExchanges)
        if (entry.second->flight == value)
            return entry.second.get();
    return nullptr;
}

bool DtlsAssociation::allocateRecord(uint64_t epoch, uint64_t& number)
{
    if (epoch == sendEpoch)
        return allocateApplicationRecord(number);
    auto it = retainedSendCounters.find(epoch);
    if (it == retainedSendCounters.end() || it->second >= MAX_PROTECTED_RECORDS)
        return false;
    number = it->second++;
    return true;
}

void DtlsAssociation::issueTicket()
{
    // RFC 9846 Section 4.7.1: one stateful ticket issued after successful establishment.
    auto& owner = engine;
    if (!serverSide || closing || !owner.par("resumptionEnabled").boolValue() || owner.operationalState != Dtls::OPERATING ||
        localMessagesExhausted || owner.nextTicketSerial == 0 || postHandshakeExchanges.count(4))
        return;
    auto serial = owner.nextTicketSerial++;
    DtlsSessionStore::Entry entry;
    for (auto value : {uint64_t(owner.getId()), serial})
        for (int shift = 56; shift >= 0; shift -= 8)
            entry.ticket.push_back(value >> shift);
    entry.serverSide = true;
    entry.peerAddress = peerAddress;
    entry.peerPort = peerPort;
    entry.peerIdentity = owner.par("identity").stdstringValue();
    entry.issuedAt = simTime();
    entry.expiresAt = simTime() + SimTime(owner.par("ticketLifetime").doubleValue());
    entry.ageAdd = (uint32_t(owner.intuniform(0, 65535)) << 16) | owner.intuniform(0, 65535);
    entry.earlyDataBytes = owner.par("earlyDataEnabled").boolValue() ? owner.par("earlyDataBytes").intValue() : 0;
    if (!owner.sessions.insert(entry, simTime()))
        return;
    auto body = makeShared<tls::TlsNewSessionTicket>();
    body->setTicketLifetime(owner.par("ticketLifetime").doubleValue());
    body->setTicketAgeAdd(entry.ageAdd);
    body->setTicketNonceArraySize(8);
    for (size_t i = 0; i < 8; ++i)
        body->setTicketNonce(i, serial >> (56 - i * 8));
    body->setTicketArraySize(entry.ticket.size());
    for (size_t i = 0; i < entry.ticket.size(); ++i)
        body->setTicket(i, entry.ticket[i]);
    if (entry.earlyDataBytes != 0) {
        body->setExtensionsArraySize(8);
        const std::vector<uint8_t> early = {0,42,0,4,uint8_t(entry.earlyDataBytes >> 24),uint8_t(entry.earlyDataBytes >> 16),uint8_t(entry.earlyDataBytes >> 8),uint8_t(entry.earlyDataBytes)};
        for (size_t i = 0; i < early.size(); ++i)
            body->setExtensions(i, early[i]);
    }
    body->setChunkLength(B(13 + 8 + entry.ticket.size() + body->getExtensionsArraySize()));
    body->markImmutable();
    auto exchange = std::make_unique<DtlsPostHandshakeExchange>(owner, "Ticket");
    auto flight = std::make_shared<DtlsHandshakeFlight>();
    exchange->flight = flight;
    auto size = body->getChunkLength().get<B>();
    auto maximum = std::min<int64_t>(owner.par("udpPayloadLimit").intValue() - 34, DtlsRecordProtection::MAX_CONTENT_BYTES - 12);
    if (maximum <= 0)
        return;
    auto sequence = nextLocalMessage;
    if (nextLocalMessage == std::numeric_limits<uint16_t>::max())
        localMessagesExhausted = true;
    else
        ++nextLocalMessage;
    if (size <= maximum)
        flight->add(4, sequence, size, 0, sendEpoch, body);
    else {
        Packet packet("DTLS ticket body", body);
        auto bytes = packet.peekDataAsBytes()->getBytes();
        for (size_t offset = 0; offset < bytes.size(); offset += maximum) {
            auto length = std::min<size_t>(maximum, bytes.size() - offset);
            auto part = makeShared<BytesChunk>(std::vector<uint8_t>(bytes.begin() + offset, bytes.begin() + offset + length));
            part->markImmutable();
            flight->add(4, sequence, size, offset, sendEpoch, part);
        }
    }
    owner.scheduleAt(simTime() + SimTime(owner.par("handshakeTimeout").doubleValue()), exchange->deadline.get());
    postHandshakeExchanges.emplace(4, std::move(exchange));
    auto handle = id;
    auto delay = SimTime(owner.par("handshakeOutDelay").doubleValue());
    if (!owner.enqueueProcessing(owner.outputProcessing.get(), handle, size + 12, delay, [&owner, handle, flight]() {
        auto it = owner.associations.find(handle);
        if (it == owner.associations.end() || !it->second->findPostHandshakeExchange(flight))
            return;
        for (size_t i = 0; i < flight->size(); ++i)
            flight->markReady(i);
        it->second->pumpTicket();
    }, [&owner, handle, flight]() {
        auto it = owner.associations.find(handle);
        return it != owner.associations.end() && !it->second->closing && it->second->findPostHandshakeExchange(flight);
    })) {
        owner.failAssociation(handle, "DTLS ticket generation queue is full");
    }
}

void DtlsAssociation::pumpTicket()
{
    auto it = postHandshakeExchanges.find(4);
    if (closing || it == postHandshakeExchanges.end() || !it->second->flight)
        return;
    auto value = it->second->flight;
    auto handle = id;
    auto& owner = engine;
    size_t queued = 0;
    for (size_t i = 0; i < value->size(); ++i) {
        if (value->reserve(i)) {
            owner.transmitFlightFragment(handle, value, i);
            if (++queued == 10)
                return;
        }
        if (!owner.containsAssociation(handle))
            return;
    }
}

void DtlsAssociation::ticketRecordHandedOff(const std::shared_ptr<DtlsHandshakeFlight>& value, size_t index, uint64_t number)
{
    auto exchange = findPostHandshakeExchange(value);
    if (!exchange)
        return;
    auto retransmission = value->get(index).transmissions != 0;
    value->handedOff(index, number, simTime());
    if (!exchange->retransmissionTimer->isScheduled())
        engine.scheduleAt(simTime() + exchange->timeout, exchange->retransmissionTimer.get());
    if (retransmission)
        engine.emit(Dtls::handshakeRetransmissionSignal, (long)id);
}

void DtlsAssociation::receiveTicket(uint64_t epoch, uint64_t number, Packet *body)
{
    if (serverSide || body->getDataLength() < B(12) || !engine.hasConsistentEncoding(body))
        return;
    auto header = body->popAtFront<DtlsHandshakeHeader>(B(12));
    if (header->getMessageType() != 4 || header->getFragmentLength() != body->getDataLength().get<B>())
        return;
    auto sequence = header->getMessageSequence();
    if (sequence > nextPeerMessage)
        return;
    auto replay = epoch == receiveEpoch ? receiveEpoch3.get() : previousReceiveEpoch.get();
    if (!replay)
        return;
    if (sequence < nextPeerMessage) {
        if (replay->accept(number))
            noteHandshakeRecord(epoch, number, true);
        return;
    }
    auto pending = postHandshakeExchanges.find(4);
    if (pending != postHandshakeExchanges.end() && pending->second->receivingSequence == sequence && !reassembly.hasEntry(sequence)) {
        // The complete body is already queued for logical handshake processing.
        if (replay->accept(number))
            noteHandshakeRecord(epoch, number, true);
        return;
    }
    auto result = reassembly.insert(4, sequence, header->getMessageLength(), header->getFragmentOffset(), body->peekDataAsBytes()->getBytes());
    if (result == DtlsHandshakeReassembly::Result::CONFLICT) {
        engine.failAssociation(id, "conflicting DTLS ticket fragments", 47);
        return;
    }
    if (result != DtlsHandshakeReassembly::Result::INCOMPLETE && result != DtlsHandshakeReassembly::Result::COMPLETE)
        return;
    if (!replay->accept(number))
        return;
    auto handle = id;
    auto& owner = engine;
    auto ticketExchange = postHandshakeExchanges.find(4);
    if (ticketExchange == postHandshakeExchanges.end())
        ticketExchange = postHandshakeExchanges.emplace(4, std::make_unique<DtlsPostHandshakeExchange>(owner, "TicketReceive")).first;
    if (!ticketExchange->second->receivingSequence) {
        ticketExchange->second->receivingSequence = sequence;
        owner.scheduleAt(simTime() + SimTime(owner.par("handshakeTimeout").doubleValue()), ticketExchange->second->deadline.get());
    }
    if (result == DtlsHandshakeReassembly::Result::COMPLETE) {
        auto bytes = reassembly.take(sequence).body;
        auto message = std::make_shared<Packet>("DTLS reassembled ticket", makeShared<BytesChunk>(bytes));
        auto delay = SimTime(owner.par("handshakeInDelay").doubleValue());
        if (!owner.inputProcessing->enqueueContinuation(handle, bytes.size() + 12, delay, [&owner, handle, message, sequence]() {
            auto it = owner.associations.find(handle);
            if (it == owner.associations.end() || it->second->nextPeerMessage != sequence)
                return;
            try {
                auto ticket = message->popAtFront<tls::TlsNewSessionTicket>(message->getDataLength());
                if (message->getDataLength() != b(0))
                    throw cRuntimeError("Trailing ticket bytes");
                std::set<uint16_t> extensions;
                uint32_t earlyLimit = 0;
                for (size_t offset = 0; offset < ticket->getExtensionsArraySize();) {
                    auto end = ticket->getExtensionsArraySize();
                    if (end - offset < 4)
                        throw cRuntimeError("Truncated ticket extension");
                    uint16_t type = (uint16_t(ticket->getExtensions(offset)) << 8) | ticket->getExtensions(offset + 1);
                    size_t length = (uint16_t(ticket->getExtensions(offset + 2)) << 8) | ticket->getExtensions(offset + 3);
                    offset += 4;
                    if (length > end - offset || !extensions.insert(type).second)
                        throw cRuntimeError("Invalid ticket extension vector");
                    if (type == 42) {
                        if (length != 4)
                            throw cRuntimeError("Invalid ticket early-data allowance");
                        for (size_t i = offset; i < offset + 4; ++i)
                            earlyLimit = (earlyLimit << 8) | ticket->getExtensions(i);
                    }
                    offset += length;
                }
                ++it->second->nextPeerMessage;
                auto& exchange = *it->second->postHandshakeExchanges.at(4);
                owner.cancelEvent(exchange.deadline.get());
                exchange.receivingSequence.reset();
                if (owner.par("resumptionEnabled").boolValue() && ticket->getTicketLifetime() != 0) {
                    DtlsSessionStore::Entry entry;
                    entry.peerAddress = it->second->peerAddress;
                    entry.peerPort = it->second->peerPort;
                    entry.peerIdentity = owner.par("identity").stdstringValue();
                    entry.issuedAt = simTime();
                    entry.expiresAt = simTime() + SimTime(std::min<uint32_t>(604800, ticket->getTicketLifetime()));
                    entry.ageAdd = ticket->getTicketAgeAdd();
                    entry.earlyDataBytes = earlyLimit;
                    for (size_t i = 0; i < ticket->getTicketArraySize(); ++i)
                        entry.ticket.push_back(ticket->getTicket(i));
                    owner.sessions.insert(entry, simTime());
                }
            }
            catch (const cRuntimeError&) {
                owner.failAssociation(handle, "malformed DTLS session ticket", 50);
            }
        }, {}, {}, [&owner, delay]() { owner.emit(Dtls::processingDelaySignal, delay.dbl()); })) {
            ++nextPeerMessage; // a client may ignore a ticket when local admission is exhausted
            owner.cancelEvent(ticketExchange->second->deadline.get());
            ticketExchange->second->receivingSequence.reset();
        }
    }
    auto current = owner.associations.find(handle);
    if (current != owner.associations.end())
        current->second->noteHandshakeRecord(epoch, number, true);
}

} // namespace inet::dtls
