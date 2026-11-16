// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include <algorithm>
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"

namespace inet::dtls {

bool Dtls::requestClientAuthentication(AssociationId id)
{
    Enter_Method("requestClientAuthentication");
    auto it = associations.find(id);
    return operationalState == OPERATING && it != associations.end() && it->second->requestClientAuthentication();
}

bool Dtls::getPeerAuthentication(AssociationId id, DtlsPeerAuthentication& result) const
{
    auto it = associations.find(id);
    if (it == associations.end() || it->second->closing || it->second->state != HandshakeState::ESTABLISHED)
        return false;
    result.identity = it->second->authenticatedPeerIdentity;
    result.certificateAuthenticated = it->second->peerCertificateAuthenticated;
    result.resumed = it->second->selection.authenticationMode == tls::TlsHandshakeParameters::AuthenticationMode::TICKET_PSK;
    return true;
}

Ptr<Chunk> Dtls::makeCertificateRequest(const std::vector<uint8_t>& context) const
{
    auto request = makeShared<tls::TlsCertificateRequest>();
    request->setRequestContextArraySize(context.size());
    for (size_t i = 0; i < context.size(); ++i)
        request->setRequestContext(i, context[i]);
    std::vector<uint8_t> bytes;
    auto append = [&bytes](uint16_t value) { bytes.push_back(value >> 8); bytes.push_back(value); };
    append(13);
    append(2 + preferences.signatures.size() * 2);
    append(preferences.signatures.size() * 2);
    for (auto signature : preferences.signatures)
        append(signature);
    request->setExtensionsArraySize(bytes.size());
    for (size_t i = 0; i < bytes.size(); ++i)
        request->setExtensions(i, bytes[i]);
    request->setChunkLength(B(3 + context.size() + bytes.size()));
    return request;
}

int Dtls::checkCertificateRequest(Packet *body, bool postHandshake, std::vector<uint8_t>& context, uint16_t& signature) const
{
    Ptr<const tls::TlsCertificateRequest> request;
    try { request = body->popAtFront<tls::TlsCertificateRequest>(body->getDataLength()); }
    catch (const cRuntimeError&) { return 50; }
    if (body->getDataLength() != b(0))
        return 50;
    if ((request->getRequestContextArraySize() != 0) != postHandshake)
        return 47;
    context.clear();
    for (size_t i = 0; i < request->getRequestContextArraySize(); ++i)
        context.push_back(request->getRequestContext(i));
    std::set<uint16_t> seen;
    bool compatible = false;
    for (size_t pos = 0, end = request->getExtensionsArraySize(); pos < end;) {
        if (end - pos < 4)
            return 50;
        uint16_t type = (uint16_t(request->getExtensions(pos)) << 8) | request->getExtensions(pos + 1);
        size_t length = (uint16_t(request->getExtensions(pos + 2)) << 8) | request->getExtensions(pos + 3);
        pos += 4;
        if (length > end - pos)
            return 50;
        if (!seen.insert(type).second)
            return 47;
        if (type == 13) {
            if (length < 4)
                return 50;
            size_t vectorLength = (uint16_t(request->getExtensions(pos)) << 8) | request->getExtensions(pos + 1);
            if (vectorLength != length - 2 || vectorLength % 2)
                return 50;
            for (size_t i = pos + 2; i < pos + length; i += 2) {
                uint16_t scheme = (uint16_t(request->getExtensions(i)) << 8) | request->getExtensions(i + 1);
                if (!compatible && std::find(preferences.signatures.begin(), preferences.signatures.end(), scheme) != preferences.signatures.end()) {
                    signature = scheme;
                    compatible = true;
                }
            }
        }
        pos += length;
    }
    return compatible ? 0 : seen.count(13) ? 40 : 109;
}

size_t DtlsAssociation::retainedPostHandshakeBytes() const
{
    size_t bytes = 0;
    for (const auto& entry : postHandshakeExchanges)
        if (entry.second->flight)
            for (size_t i = 0; i < entry.second->flight->size(); ++i)
                bytes += entry.second->flight->get(i).data->getChunkLength().get<B>() + 12;
    return bytes;
}

bool DtlsAssociation::authenticationActive() const
{
    if (authenticationRequest)
        return true;
    auto incoming = postHandshakeExchanges.find(256 | 13);
    if (incoming != postHandshakeExchanges.end() && incoming->second->receivingSequence)
        return true;
    for (const auto& entry : postHandshakeExchanges)
        if (entry.first >= AUTHENTICATION_RESPONSE_BASE && !entry.second->firstTransmissionComplete)
            return true;
    return false;
}

bool DtlsAssociation::permitsOutput(const std::shared_ptr<DtlsHandshakeFlight>& value) const
{
    return !authenticationOutput || postHandshakeExchanges.at(*authenticationOutput)->flight == value;
}

bool DtlsAssociation::requestClientAuthentication()
{
    if (!engine.par("postHandshakeAuthEnabled").boolValue() || !serverSide || state != HandshakeState::ESTABLISHED || closing ||
        !offer.hasPostHandshakeAuth || authenticationRequest || candidatePath || engine.nextAuthenticationContext == 0 ||
        selection.authenticationMode == tls::TlsHandshakeParameters::AuthenticationMode::EXTERNAL_PSK)
        return false;
    std::vector<uint8_t> context;
    auto serial = engine.nextAuthenticationContext++;
    for (int shift = 56; shift >= 0; shift -= 8)
        context.push_back(serial >> shift);
    for (size_t i = 0; i < 8; ++i)
        context.push_back(engine.intuniform(0, 255));
    auto& owner = engine;
    auto handle = id;
    auto body = owner.makeCertificateRequest(context);
    auto capacity = owner.par("maxProcessingBytes").intValue();
    auto occupied = owner.outputProcessing->getOccupiedBytes();
    if (capacity <= 0 || occupied > size_t(capacity) || body->getChunkLength().get<B>() + 12 > size_t(capacity) - occupied)
        return false;
    authenticationRequest = AuthenticationRequest{context};
    suspendHeartbeatForAuthentication();
    if (startPostHandshake(13, body, "AuthenticationRequest"))
        return true;
    auto it = owner.associations.find(handle);
    if (it != owner.associations.end()) {
        it->second->authenticationRequest.reset();
        it->second->resumeHeartbeatAfterAuthentication();
    }
    return false;
}

void DtlsAssociation::receiveAuthentication(uint64_t epoch, uint64_t number, Packet *body)
{
    if (body->getDataLength() < B(12) || !engine.hasConsistentEncoding(body))
        return;
    auto header = body->popAtFront<DtlsHandshakeHeader>(B(12));
    auto type = header->getMessageType();
    if ((!serverSide && (type != 13 || !offer.hasPostHandshakeAuth)) ||
        (serverSide && type != 11 && type != 15 && type != 20) ||
        selection.authenticationMode == tls::TlsHandshakeParameters::AuthenticationMode::EXTERNAL_PSK) {
        engine.failAssociation(id, "unnegotiated DTLS post-handshake authentication", 10);
        return;
    }
    auto sequence = header->getMessageSequence();
    if (header->getFragmentLength() != body->getDataLength().get<B>() || sequence > nextPeerMessage)
        return;
    auto replay = epoch == receiveEpoch ? receiveEpoch3.get() : previousReceiveEpoch.get();
    if (!replay)
        return;
    if (sequence < nextPeerMessage) {
        if (replay->accept(number))
            noteHandshakeRecord(epoch, number, true);
        return;
    }
    if (serverSide && (!authenticationRequest || authenticationRequest->expectedType != type)) {
        engine.failAssociation(id, "unexpected DTLS authentication response", 10);
        return;
    }
    auto result = reassembly.insert(type, sequence, header->getMessageLength(), header->getFragmentOffset(), body->peekDataAsBytes()->getBytes());
    if (result == DtlsHandshakeReassembly::Result::CONFLICT) {
        engine.failAssociation(id, "conflicting DTLS authentication fragments", 47);
        return;
    }
    if (result != DtlsHandshakeReassembly::Result::INCOMPLETE && result != DtlsHandshakeReassembly::Result::COMPLETE)
        return;
    if (!replay->accept(number))
        return;
    auto key = uint32_t(256 | type);
    auto incoming = postHandshakeExchanges.find(key);
    if (incoming == postHandshakeExchanges.end())
        incoming = postHandshakeExchanges.emplace(key, std::make_unique<DtlsPostHandshakeExchange>(engine, "AuthenticationReceive")).first;
    auto& exchange = *incoming->second;
    if (!exchange.receivingSequence) {
        exchange.receivingSequence = sequence;
        engine.scheduleAt(simTime() + SimTime(engine.par("handshakeTimeout").doubleValue()), exchange.deadline.get());
        suspendHeartbeatForAuthentication();
    }
    auto& owner = engine;
    auto handle = id;
    if (result == DtlsHandshakeReassembly::Result::COMPLETE && !exchange.receivingQueued) {
        exchange.receivingQueued = true;
        auto delay = owner.processingDelay(handle, "handshakeInDelay");
        if (!owner.inputProcessing->enqueueContinuation(handle, header->getMessageLength() + 12, delay, [&owner, handle, type, sequence]() {
            auto it = owner.associations.find(handle);
            if (it != owner.associations.end())
                it->second->applyAuthentication(type, sequence);
        }, {}, {}, [&owner, delay]() { owner.emit(Dtls::processingDelaySignal, delay.dbl()); })) {
            exchange.receivingQueued = false;
            return;
        }
    }
    auto it = owner.associations.find(handle);
    if (it != owner.associations.end())
        it->second->noteHandshakeRecord(epoch, number, true);
}

void DtlsAssociation::applyAuthentication(uint8_t type, uint16_t sequence)
{
    if (nextPeerMessage != sequence || !reassembly.isComplete(sequence))
        return;
    auto bytes = reassembly.take(sequence).body;
    auto& incoming = *postHandshakeExchanges.at(256 | type);
    engine.cancelEvent(incoming.deadline.get());
    incoming.receivingSequence.reset();
    incoming.receivingQueued = false;
    Packet body("DTLS reassembled authentication", makeShared<BytesChunk>(bytes));
    auto& owner = engine;
    auto handle = id;
    if (!serverSide) {
        std::vector<uint8_t> context;
        uint16_t signature = 0;
        auto alert = owner.checkCertificateRequest(&body, true, context, signature);
        if (alert) {
            owner.failAssociation(handle, "invalid post-handshake CertificateRequest", alert);
            return;
        }
        for (const auto& entry : postHandshakeExchanges)
            if (entry.first >= AUTHENTICATION_RESPONSE_BASE && entry.second->authenticationContext == context) {
                owner.failAssociation(handle, "reused active authentication context", 47);
                return;
            }
        std::vector<std::pair<uint8_t, Ptr<Chunk>>> response;
        if (owner.par("postHandshakeCertificateAvailable").boolValue()) {
            response.emplace_back(11, owner.makeCertificate(context));
            response.emplace_back(15, owner.makeCertificateVerify(signature));
        }
        else {
            auto empty = makeShared<tls::TlsCertificate>();
            empty->setRequestContextArraySize(context.size());
            for (size_t i = 0; i < context.size(); ++i)
                empty->setRequestContext(i, context[i]);
            empty->setChunkLength(B(4 + context.size()));
            response.emplace_back(11, empty);
        }
        auto finished = makeShared<tls::TlsFinished>();
        auto hashBytes = tls::TlsHandshakeParameters::findCipherSuite(selection.cipherSuite)->hashBytes;
        finished->setVerifyDataArraySize(hashBytes);
        finished->setChunkLength(B(hashBytes));
        response.emplace_back(20, finished);
        ++nextPeerMessage;
        if (!startPostHandshakeFlight(AUTHENTICATION_RESPONSE_BASE | sequence, response, "AuthenticationResponse", context) && owner.containsAssociation(handle))
            owner.failAssociation(handle, "DTLS authentication response admission failed");
        return;
    }
    if (!authenticationRequest || authenticationRequest->expectedType != type) {
        owner.failAssociation(handle, "unexpected authentication response", 10);
        return;
    }
    if (type == 11) {
        auto alert = owner.checkCertificate(&body, authenticationRequest->context);
        if (alert) {
            owner.failAssociation(handle, "invalid post-handshake client certificate", alert);
            return;
        }
        authenticationRequest->identity = owner.par("identity").stdstringValue();
        authenticationRequest->expectedType = 15;
        auto& request = *postHandshakeExchanges.at(13);
        owner.cancelEvent(request.retransmissionTimer.get());
        request.flight.reset(); // The context-bound response implicitly acknowledges the request.
        pruneRetainedSendCounters();
    }
    else if (type == 15) {
        uint16_t scheme = 0;
        auto alert = owner.checkCertificateVerify(&body, scheme);
        if (alert) {
            owner.failAssociation(handle, "invalid post-handshake client signature", alert);
            return;
        }
        authenticationRequest->expectedType = 20;
    }
    else {
        if (body.getDataLength() != B(tls::TlsHandshakeParameters::findCipherSuite(selection.cipherSuite)->hashBytes)) {
            owner.failAssociation(handle, "invalid post-handshake Finished length", 50);
            return;
        }
        authenticatedPeerIdentity = authenticationRequest->identity;
        peerCertificateAuthenticated = true;
        authenticationRequest.reset();
        auto& request = *postHandshakeExchanges.at(13);
        request.cancelTimers();
        request.flight.reset();
        ++nextPeerMessage;
        auto identity = authenticatedPeerIdentity;
        auto callback = this->callback;
        callback->authenticationResult(handle, true, identity.c_str());
        auto it = owner.associations.find(handle);
        if (it != owner.associations.end())
            it->second->resumeHeartbeatAfterAuthentication();
        return;
    }
    ++nextPeerMessage;
}

void DtlsAssociation::resumeDeferredOutput()
{
    auto& owner = engine;
    auto handle = id;
    flushAcknowledgements(sendEpoch);
    auto it = owner.associations.find(handle);
    if (it == owner.associations.end())
        return;
    it->second->pumpUpdate();
    it = owner.associations.find(handle);
    if (it == owner.associations.end())
        return;
    for (const auto& entry : it->second->postHandshakeExchanges)
        if (entry.first != 24 && isOutgoingExchange(entry.first) && entry.second->flight) {
            it->second->pumpPostHandshake(entry.first);
            it = owner.associations.find(handle);
            if (it == owner.associations.end())
                return;
        }
    it->second->resumeHeartbeatAfterAuthentication();
    owner.resumeApplications(handle);
}

} // namespace inet::dtls
