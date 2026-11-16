// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include <algorithm>
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/transportlayer/dtls/Dtls12HelloVerifyRequest_m.h"
#include "inet/transportlayer/dtls/Dtls12RecordProtection.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"
#include "inet/transportlayer/dtls/DtlsRecordProtection.h"

namespace inet::dtls {

static void append16(std::vector<uint8_t>& bytes, uint16_t value)
{
    bytes.push_back(value >> 8);
    bytes.push_back(value);
}

static void extension12(std::vector<uint8_t>& bytes, uint16_t type, const std::vector<uint8_t>& value)
{
    append16(bytes, type);
    append16(bytes, value.size());
    bytes.insert(bytes.end(), value.begin(), value.end());
}

static Ptr<Chunk> certificate12(Dtls& engine)
{
    auto identity = engine.par("identity").stdstringValue();
    auto count = engine.par("certificateBytes").intValue();
    if (identity.empty() || count < int64_t(identity.size()) || count > 0xfffffc)
        throw cRuntimeError("Invalid modeled DTLS 1.2 certificate size");
    auto body = makeShared<Dtls12Certificate>();
    body->setCertificateArraySize(count);
    for (size_t i = 0; i < size_t(count); ++i)
        body->setCertificate(i, i < identity.size() ? uint8_t(identity[i]) : 0);
    body->setChunkLength(B(6 + count));
    return body;
}

static Ptr<Chunk> finished12()
{
    auto body = makeShared<tls::TlsFinished>();
    body->setVerifyDataArraySize(12);
    body->setChunkLength(B(12));
    return body;
}

void DtlsAssociation::initializeDtls12(bool server, uint16_t helloSequence)
{
    auto sentInitialHello = flight && flight->size() != 0;
    legacyHandshake = std::make_unique<LegacyHandshake>();
    serverSide = server;
    selection.version = DTLS_12;
    retransmissionTimeout = SimTime(1);
    bool certificate = engine.par("profile").stdstringValue() == "certificate";
    selection.cipherSuite = certificate ? ECDHE_ECDSA_AES_128_GCM_SHA256 : PSK_AES_128_GCM_SHA256;
    selection.authenticationMode = certificate ? tls::TlsHandshakeParameters::AuthenticationMode::CERTIFICATE : tls::TlsHandshakeParameters::AuthenticationMode::EXTERNAL_PSK;
    selection.group = tls::TlsHandshakeParameters::SECP256R1;
    selection.signature = certificate ? tls::TlsHandshakeParameters::ECDSA_P256_SHA256 : 0;
    sendEpoch = receiveEpoch = 1;
    nextApplicationRecord = 0;
    receiveEpoch2.reset();
    receiveEpoch3 = std::make_unique<DtlsRecordState>(engine.par("replayWindowSize").intValue());
    nextLocalMessage = server ? helloSequence : sentInitialHello ? 1 : 0;
    nextPeerMessage = server ? uint32_t(helloSequence) + 1 : 0;
    localMessagesExhausted = false;
    clientAuthRequested = server && certificate && engine.par("clientAuthentication").boolValue();
    state = server ? HandshakeState::SERVER_WAIT_CLIENT_FINISHED : HandshakeState::CLIENT_WAIT_SERVER_HELLO;
    if (server)
        legacyHandshake->phase = clientAuthRequested ? LegacyHandshake::Phase::SERVER_WAIT_CLIENT_CERTIFICATE : LegacyHandshake::Phase::SERVER_WAIT_CLIENT_KEY_EXCHANGE;
    cidOffered = engine.par("connectionIdEnabled").boolValue() && (!server || offer.hasConnectionId);
    cidNegotiated = server && cidOffered;
    if (server || !cidOffered)
        localCid.clear();
    peerCid.clear();
    sparePeerCids.clear();
    if (!cidOffered)
        engine.removeConnectionIds(id);
    if (server && cidNegotiated)
        peerCid = offer.connectionId;
    heartbeatNegotiated = server && engine.par("heartbeatEnabled").boolValue() && offer.heartbeatMode != 0;
    peerAllowsHeartbeat = heartbeatNegotiated && offer.heartbeatMode == 1;
    pendingAcks.clear();
    engine.cancelEvent(delayedAckTimer.get());
    if (offeredSession && offeredSession->version != DTLS_12)
        offeredSession.reset();
    earlyOffered = false;
    if (earlyRequested)
        engine.reportEarlyData(id, false);
}

void DtlsAssociation::sendClientHello12(const std::vector<uint8_t>& cookie)
{
    auto& owner = engine;
    auto handle = id;
    auto hello = makeShared<DtlsClientHello>();
    for (size_t i = 0; i < 32; ++i) {
        if (nextLocalMessage == 0)
            clientRandom[i] = engine.intuniform(0, 255);
        hello->setRandom(i, clientRandom[i]);
    }
    hello->setLegacyCookieArraySize(cookie.size());
    for (size_t i = 0; i < cookie.size(); ++i)
        hello->setLegacyCookie(i, cookie[i]);
    hello->setCipherSuitesArraySize(1);
    hello->setCipherSuites(0, selection.cipherSuite);
    hello->setCompressionMethodsArraySize(1);
    hello->setCompressionMethods(0, 0);
    std::vector<uint8_t> extensions;
    appendClientSession12(hello, extensions);
    if (owner.par("heartbeatEnabled").boolValue())
        extension12(extensions, 15, {uint8_t(owner.par("heartbeatAcceptRequests").boolValue() ? 1 : 2)});
    if (cidOffered) {
        std::vector<uint8_t> cid = {uint8_t(localCid.size())};
        cid.insert(cid.end(), localCid.begin(), localCid.end());
        extension12(extensions, 54, cid);
    }
    if (!selection.usesPsk()) {
        extension12(extensions, 10, {0,2,0,23});
        extension12(extensions, 13, {0,2,4,3});
        extension12(extensions, 11, {1,0});
    }
    hello->setExtensionsArraySize(extensions.size());
    for (size_t i = 0; i < extensions.size(); ++i)
        hello->setExtensions(i, extensions[i]);
    hello->setChunkLength(B(44 + cookie.size() + hello->getSessionIdArraySize() + extensions.size()));
    auto error = tls::TlsHandshakeParameters::decodeOffer(hello, offer);
    if (!error.empty())
        throw cRuntimeError("Invalid DTLS 1.2 ClientHello: %s", error.c_str());
    beginFlight();
    appendHandshake(1, nextLocalMessage++, 0, hello, false, false);
    auto it = owner.associations.find(handle);
    if (it != owner.associations.end())
        it->second->finishFlight();
}

void DtlsAssociation::sendServerFlight12()
{
    auto& owner = engine;
    auto handle = id;
    auto hello = makeShared<tls::TlsServerHello>();
    hello->setLegacyVersion(DTLS_12);
    hello->setCipherSuite(selection.cipherSuite);
    hello->setExtensionsPresent(false);
    for (size_t i = 0; i < 32; ++i)
        hello->setRandom(i, owner.intuniform(0,255));
    if (owner.allowDtls13) {
        static const uint8_t sentinel[] = {'D','O','W','N','G','R','D',1};
        for (size_t i = 0; i < 8; ++i)
            hello->setRandom(24+i, sentinel[i]);
    }
    fillServerSessionHello12(hello);
    beginFlight();
    serverFlightQueued = true;
    appendHandshake(2, nextLocalMessage++, 0, hello, false, false);
    if (!owner.containsAssociation(handle)) return;
    if (selection.authenticationMode == tls::TlsHandshakeParameters::AuthenticationMode::TICKET_PSK) {
        appendSessionTicket12();
        if (!owner.containsAssociation(handle)) return;
        appendChangeCipherSpec12();
        appendHandshake(20, nextLocalMessage++, 1, finished12(), false, false);
        if (owner.containsAssociation(handle)) finishFlight();
        return;
    }
    if (!selection.usesPsk()) {
        appendHandshake(11, nextLocalMessage++, 0, certificate12(owner), false, false);
        if (!owner.containsAssociation(handle)) return;
        auto exchange = makeShared<Dtls12EcdheServerKeyExchange>();
        exchange->setPublicKeyArraySize(65);
        exchange->setPublicKey(0,4);
        for (size_t i = 1; i < 65; ++i) exchange->setPublicKey(i,owner.intuniform(0,255));
        auto signatureBytes = owner.par("signatureBytes").intValue();
        if (signatureBytes <= 0 || signatureBytes > 65535)
            throw cRuntimeError("Invalid DTLS 1.2 signature size");
        exchange->setSignatureArraySize(signatureBytes);
        exchange->setChunkLength(B(73 + signatureBytes));
        appendHandshake(12, nextLocalMessage++, 0, exchange, false, false);
        if (!owner.containsAssociation(handle)) return;
        if (clientAuthRequested) {
            auto request = makeShared<Dtls12CertificateRequest>();
            request->setCertificateTypesArraySize(1); request->setCertificateTypes(0,64);
            request->setSignatureSchemesArraySize(1); request->setSignatureSchemes(0,0x0403);
            request->setChunkLength(B(8));
            appendHandshake(13, nextLocalMessage++, 0, request, false, false);
            if (!owner.containsAssociation(handle)) return;
        }
    }
    appendHandshake(14, nextLocalMessage++, 0, makeShared<Dtls12ServerHelloDone>(), false, false);
    if (owner.containsAssociation(handle)) finishFlight();
}

void DtlsAssociation::appendChangeCipherSpec12()
{
    auto body = makeShared<Dtls12ChangeCipherSpec>();
    body->markImmutable();
    auto index = flight->add(0, nextLocalMessage, 1, 0, 0, body, false, false, 20);
    flight->markReady(index);
}

void DtlsAssociation::sendClientFlight12()
{
    auto& owner = engine;
    auto handle = id;
    beginFlight();
    if (clientAuthRequested) {
        appendHandshake(11, nextLocalMessage++, 0, certificate12(owner), false, false);
        if (!owner.containsAssociation(handle)) return;
    }
    if (selection.usesPsk()) {
        auto identity = owner.par("identity").stdstringValue();
        if (identity.empty() || identity.size() > 65535)
            throw cRuntimeError("Invalid DTLS 1.2 PSK identity size");
        auto exchange = makeShared<Dtls12PskClientKeyExchange>();
        exchange->setIdentityArraySize(identity.size());
        for (size_t i = 0; i < identity.size(); ++i) exchange->setIdentity(i,uint8_t(identity[i]));
        exchange->setChunkLength(B(2 + identity.size()));
        appendHandshake(16, nextLocalMessage++, 0, exchange, false, false);
    }
    else {
        auto exchange = makeShared<Dtls12EcdheClientKeyExchange>();
        exchange->setPublicKeyArraySize(65); exchange->setPublicKey(0,4);
        for (size_t i = 1; i < 65; ++i) exchange->setPublicKey(i,owner.intuniform(0,255));
        exchange->setChunkLength(B(66));
        appendHandshake(16, nextLocalMessage++, 0, exchange, false, false);
    }
    if (!owner.containsAssociation(handle)) return;
    if (clientAuthRequested) {
        appendHandshake(15, nextLocalMessage++, 0, owner.makeCertificateVerify(0x0403), false, false);
        if (!owner.containsAssociation(handle)) return;
    }
    appendChangeCipherSpec12();
    appendHandshake(20, nextLocalMessage++, 1, finished12(), false, false);
    if (!owner.containsAssociation(handle)) return;
    legacyHandshake->phase = legacyHandshake->ticketExpected ? LegacyHandshake::Phase::CLIENT_WAIT_SERVER_TICKET : LegacyHandshake::Phase::CLIENT_WAIT_SERVER_CCS;
    finishFlight();
}

void DtlsAssociation::sendFinalFlight12()
{
    auto& owner = engine;
    auto handle = id;
    beginFlight();
    if (serverSide) {
        appendSessionTicket12();
        if (!owner.containsAssociation(handle)) return;
    }
    appendChangeCipherSpec12();
    appendHandshake(20, nextLocalMessage++, 1, finished12(), true, false);
    if (owner.containsAssociation(handle)) finishFlight();
}

void DtlsAssociation::protectRecord(Packet *packet, uint8_t type, uint64_t epoch, uint64_t number, const std::vector<uint8_t> *cidOverride)
{
    const auto& cid = cidOverride ? *cidOverride : peerCid;
    if (isDtls12() || epoch == 0) {
        if (epoch > 65535)
            throw cRuntimeError("DTLS 1.2 epoch exceeds its record field");
        bool withCid = isDtls12() && epoch != 0 && cidNegotiated && !cid.empty();
        if (isDtls12() && epoch != 0) {
            if (withCid)
                Dtls12RecordProtection::protectCid(packet, uint16_t(epoch), number, type);
            else
                Dtls12RecordProtection::protect(packet, uint16_t(epoch), number);
        }
        packet->insertAtFront(makeRecordHeader12(type, epoch, number, packet->getDataLength().get<B>(), withCid ? cid : std::vector<uint8_t>{}));
    }
    else {
        DtlsRecordProtection::protect(packet,type);
        packet->insertAtFront(makeRecordHeader(epoch,number,packet->getDataLength().get<B>(),&cid));
    }
}

bool DtlsAssociation::writeCipherActive12() const
{
    if (state == HandshakeState::ESTABLISHED)
        return true;
    if (flight)
        for (size_t i=0; i<flight->size(); ++i)
            if (flight->get(i).recordContentType == 20 && flight->get(i).transmissions != 0)
                return true;
    return false;
}

void DtlsAssociation::receiveChangeCipherSpec12()
{
    if (!legacyHandshake || closing || legacyHandshake->phase == LegacyHandshake::Phase::COMPLETE ||
        legacyHandshake->phase == LegacyHandshake::Phase::CLIENT_WAIT_SERVER_FINISHED || legacyHandshake->phase == LegacyHandshake::Phase::SERVER_WAIT_CLIENT_FINISHED)
        return;
    legacyHandshake->pendingCcs = true;
    applyChangeCipherSpec12();
}

void DtlsAssociation::applyChangeCipherSpec12()
{
    using Phase = LegacyHandshake::Phase;
    if (!legacyHandshake->pendingCcs)
        return;
    if (legacyHandshake->phase == Phase::CLIENT_WAIT_SERVER_CCS) {
        legacyHandshake->pendingCcs = false;
        legacyHandshake->phase = Phase::CLIENT_WAIT_SERVER_FINISHED;
    }
    else if (legacyHandshake->phase == Phase::SERVER_WAIT_CLIENT_CCS) {
        legacyHandshake->pendingCcs = false;
        legacyHandshake->phase = Phase::SERVER_WAIT_CLIENT_FINISHED;
    }
}

void DtlsAssociation::retransmitFlight12()
{
    if (!flight || closing || (state == HandshakeState::ESTABLISHED && !retentionTimer->isScheduled()))
        return;
    engine.cancelEvent(retransmissionTimer.get());
    flight->requestRetransmission();
    burstLimit = burstQueued;
    pumpFlight();
}

void DtlsAssociation::receiveHandshake12(Packet *packet, uint64_t epoch)
{
    auto& owner=engine;
    auto handle=id;
    while (packet->getDataLength()>b(0)) {
        if (packet->getDataLength()<B(12)) { owner.failAssociation(handle,"Truncated DTLS 1.2 handshake header",50); return; }
        auto header=packet->peekAtFront<DtlsHandshakeHeader>(B(12));
        auto length=B(12+header->getFragmentLength());
        if (length>packet->getDataLength()) { owner.failAssociation(handle,"Truncated DTLS 1.2 handshake fragment",50); return; }
        Packet fragment("DTLS 1.2 handshake fragment",packet->removeAtFront(length));
        auto current=owner.associations.find(handle);
        if (current==owner.associations.end()) return;
        current->second->receiveHandshakeFragment12(&fragment,epoch);
        if (!owner.containsAssociation(handle)) return;
    }
}

void DtlsAssociation::receiveHandshakeFragment12(Packet *packet, uint64_t epoch)
{
    if (closing || packet->getDataLength() < B(12))
        return;
    auto header = packet->popAtFront<DtlsHandshakeHeader>(B(12));
    auto type = header->getMessageType();
    auto sequence = header->getMessageSequence();
    if (epoch == 1 && state == HandshakeState::ESTABLISHED && (type == 0 || type == 1)) {
        if (serverSide ? type == 1 : type == 0)
            refuseRenegotiation12();
        else
            engine.failAssociation(id,"Unexpected DTLS 1.2 handshake direction",10);
        return;
    }
    if ((type == 20 ? epoch != 1 : epoch != 0) || header->getFragmentLength() != packet->getDataLength().get<B>() ||
        !engine.hasConsistentEncoding(packet)) {
        engine.failAssociation(id,"Invalid DTLS 1.2 handshake record",50);
        return;
    }
    if (sequence < nextPeerMessage && processedHandshakes.find(sequence) == processedHandshakes.end())
        return;
    std::vector<uint8_t> bytes;
    if (packet->getDataLength() > b(0))
        bytes = packet->peekDataAsBytes()->getBytes();
    if (header->getMessageLength() == 0 && (type != 14 || serverSide || epoch != 0)) {
        engine.failAssociation(id,"Invalid empty DTLS 1.2 handshake message",50);
        return;
    }
    if (processingHandshake && sequence == nextPeerMessage) {
        auto offset = header->getFragmentOffset();
        if (type != processingHandshakeType || header->getMessageLength() != processingHandshakeBody.size() ||
            offset > processingHandshakeBody.size() || bytes.size() > processingHandshakeBody.size() - offset ||
            !std::equal(bytes.begin(),bytes.end(),processingHandshakeBody.begin()+offset))
            engine.failAssociation(id,"Conflicting active DTLS 1.2 handshake message",47);
        return;
    }
    DtlsHandshakeReassembly::Result result;
    if (header->getMessageLength() == 0 && header->getFragmentOffset() == 0 && bytes.empty() && type == 14 && !serverSide)
        result = reassembly.insertEmptyMessage(type,sequence);
    else
        result = reassembly.insert(type,sequence,header->getMessageLength(),header->getFragmentOffset(),bytes);
    if (result == DtlsHandshakeReassembly::Result::LIMIT || result == DtlsHandshakeReassembly::Result::CONFLICT || result == DtlsHandshakeReassembly::Result::OUT_OF_RANGE) {
        engine.failAssociation(id,"Invalid or excessive DTLS 1.2 handshake reassembly",result == DtlsHandshakeReassembly::Result::CONFLICT ? 47 : 50);
        return;
    }
    auto& owner = engine;
    auto handle = id;
    owner.emitReassemblyBytes();
    auto current = owner.associations.find(handle);
    if (current == owner.associations.end()) return;
    if ((result == DtlsHandshakeReassembly::Result::COMPLETE || result == DtlsHandshakeReassembly::Result::INCOMPLETE) &&
        current->second->serverSide && sequence == current->second->nextPeerMessage && (type == 11 || type == 16)) {
        owner.cancelEvent(current->second->retransmissionTimer.get());
        if (current->second->flight) current->second->flight->acknowledgeSent(simTime());
    }
    if (result == DtlsHandshakeReassembly::Result::COMPLETE && sequence < current->second->nextPeerMessage) {
        auto message = current->second->reassembly.take(sequence);
        const auto& old = current->second->processedHandshakes.at(sequence);
        if (old.type != message.type || old.body != message.body) {
            owner.failAssociation(handle,"Conflicting DTLS 1.2 retransmission",47);
            return;
        }
        owner.emitReassemblyBytes();
        current = owner.associations.find(handle);
        if (current != owner.associations.end() && (type == 3 || type == 14 || type == 20 ||
            (type == 1 && current->second->serverSide && current->second->state != HandshakeState::ESTABLISHED)))
            current->second->retransmitFlight12();
        return;
    }
    current->second->pumpHandshake12();
}

void DtlsAssociation::pumpHandshake12()
{
    if (closing || processingHandshake || nextPeerMessage > 65535 || !reassembly.isComplete(nextPeerMessage))
        return;
    auto sequence = uint16_t(nextPeerMessage);
    auto message = reassembly.take(sequence);
    auto& owner = engine;
    auto handle = id;
    processingHandshake = true;
    processingHandshakeType = message.type;
    processingHandshakeBody = message.body;
    owner.emitReassemblyBytes();
    auto current = owner.associations.find(handle);
    if (current == owner.associations.end()) return;
    auto delay = owner.processingDelay(handle,"handshakeInDelay");
    auto bytes = message.body.size() + 12;
    auto eligible = [&owner,handle]() { auto it = owner.associations.find(handle); return it != owner.associations.end() && !it->second->closing; };
    if (!owner.inputProcessing->enqueueContinuation(handle,bytes,delay,[&owner,handle,sequence,message]() {
        auto it = owner.associations.find(handle);
        if (it == owner.associations.end()) return;
        it->second->processingHandshake = false;
        it->second->processingHandshakeBody.clear();
        ++it->second->nextPeerMessage;
        it->second->processedHandshakes[sequence] = message;
        try { it->second->processHandshake12(message.type,sequence,message.body); }
        catch (const cRuntimeError&) { owner.failAssociation(handle,"Malformed DTLS 1.2 handshake body",50); return; }
        it = owner.associations.find(handle);
        if (it != owner.associations.end()) it->second->pumpHandshake12();
    },eligible,{},[&owner,delay]() { owner.emit(Dtls::processingDelaySignal,delay.dbl()); }))
        owner.failAssociation(handle,"DTLS 1.2 handshake processor is full");
}

void DtlsAssociation::processHandshake12(uint8_t type, uint16_t sequence, const std::vector<uint8_t>& bytes)
{
    using Phase = LegacyHandshake::Phase;
    auto& owner = engine;
    auto handle = id;
    auto fail = [&](uint8_t alert) { owner.failAssociation(handle,"DTLS 1.2 handshake rejected",alert); };
    Packet body("DTLS 1.2 handshake body");
    if (!bytes.empty()) body.insertAtBack(makeShared<BytesChunk>(bytes));
    auto certificateMatches = [&]() {
        auto certificate = body.popAtFront<Dtls12Certificate>(body.getDataLength());
        auto identity = owner.par("identity").stdstringValue();
        if (identity.empty() || certificate->getCertificateArraySize() < identity.size()) return false;
        for (size_t i=0; i<certificate->getCertificateArraySize(); ++i)
            if (certificate->getCertificate(i) != (i < identity.size() ? uint8_t(identity[i]) : 0)) return false;
        return true;
    };
    auto phase = legacyHandshake->phase;
    if (phase == Phase::CLIENT_WAIT_SERVER_HELLO) {
        if (type == 3) {
            if (retried || sequence != 0) { fail(10); return; }
            auto hvr = body.popAtFront<Dtls12HelloVerifyRequest>(body.getDataLength());
            if (hvr->getCookieArraySize() == 0) { fail(47); return; }
            retryCookie.clear();
            for (size_t i=0; i<hvr->getCookieArraySize(); ++i) retryCookie.push_back(hvr->getCookie(i));
            retried = true;
            sendClientHello12(retryCookie);
            return;
        }
        if (type != 2) { fail(10); return; }
        auto hello = body.popAtFront<tls::TlsServerHello>(body.getDataLength());
        if (hello->getLegacyVersion() != DTLS_12 || hello->getCompressionMethod() != 0 ||
            hello->getCipherSuite() != selection.cipherSuite ||
            std::find(offer.cipherSuites.begin(),offer.cipherSuites.end(),hello->getCipherSuite()) == offer.cipherSuites.end()) { fail(47); return; }
        if (owner.allowDtls13) {
            static const uint8_t sentinel[] = {'D','O','W','N','G','R','D',1};
            bool downgrade = true;
            for (size_t i=0; i<8; ++i) downgrade &= hello->getRandom(24+i) == sentinel[i];
            if (downgrade) { fail(47); return; }
        }
        state = HandshakeState::CLIENT_WAIT_SERVER_FINISHED;
        legacyHandshake->phase = selection.usesPsk() ? Phase::CLIENT_WAIT_SERVER_KEY_EXCHANGE : Phase::CLIENT_WAIT_SERVER_CERTIFICATE;
        if (!processSessionHello12(hello)) return;
        engine.cancelEvent(retransmissionTimer.get());
        if (flight) flight->acknowledgeSent(simTime());
        return;
    }
    if (phase == Phase::CLIENT_WAIT_SERVER_CERTIFICATE || phase == Phase::SERVER_WAIT_CLIENT_CERTIFICATE) {
        if (type != 11) { fail(10); return; }
        if (!certificateMatches()) { fail(42); return; }
        legacyHandshake->phase = serverSide ? Phase::SERVER_WAIT_CLIENT_KEY_EXCHANGE : Phase::CLIENT_WAIT_SERVER_KEY_EXCHANGE;
        return;
    }
    if (phase == Phase::CLIENT_WAIT_SERVER_KEY_EXCHANGE) {
        if (selection.usesPsk() && type == 14 && bytes.empty()) { sendClientFlight12(); return; }
        if (type != 12) { fail(10); return; }
        if (selection.usesPsk()) {
            body.popAtFront<Dtls12PskServerKeyExchange>(body.getDataLength());
            legacyHandshake->phase = Phase::CLIENT_WAIT_SERVER_DONE;
        }
        else {
            auto exchange = body.popAtFront<Dtls12EcdheServerKeyExchange>(body.getDataLength());
            if (exchange->getNamedGroup() != 23 || exchange->getPublicKeyArraySize() != 65 || exchange->getPublicKey(0) != 4 ||
                exchange->getSignatureScheme() != 0x0403 || exchange->getSignatureArraySize() == 0) { fail(47); return; }
            legacyHandshake->phase = Phase::CLIENT_WAIT_REQUEST_OR_DONE;
        }
        return;
    }
    if (phase == Phase::CLIENT_WAIT_REQUEST_OR_DONE && type == 13) {
        auto request = body.popAtFront<Dtls12CertificateRequest>(body.getDataLength());
        bool typeAllowed = false, signatureAllowed = false;
        for (size_t i=0; i<request->getCertificateTypesArraySize(); ++i) typeAllowed |= request->getCertificateTypes(i) == 64;
        for (size_t i=0; i<request->getSignatureSchemesArraySize(); ++i) signatureAllowed |= request->getSignatureSchemes(i) == 0x0403;
        if (!typeAllowed || !signatureAllowed || request->getCertificateAuthoritiesArraySize() != 0) { fail(47); return; }
        clientAuthRequested = true;
        legacyHandshake->phase = Phase::CLIENT_WAIT_SERVER_DONE;
        return;
    }
    if (phase == Phase::CLIENT_WAIT_REQUEST_OR_DONE || phase == Phase::CLIENT_WAIT_SERVER_DONE) {
        if (type != 14 || !bytes.empty()) { fail(10); return; }
        sendClientFlight12();
        return;
    }
    if (phase == Phase::SERVER_WAIT_CLIENT_KEY_EXCHANGE) {
        if (type != 16) { fail(10); return; }
        if (selection.usesPsk()) {
            auto exchange = body.popAtFront<Dtls12PskClientKeyExchange>(body.getDataLength());
            auto identity = owner.par("identity").stdstringValue();
            if (exchange->getIdentityArraySize() != identity.size()) { fail(115); return; }
            for (size_t i=0; i<identity.size(); ++i)
                if (exchange->getIdentity(i) != uint8_t(identity[i])) { fail(115); return; }
        }
        else {
            auto exchange = body.popAtFront<Dtls12EcdheClientKeyExchange>(body.getDataLength());
            if (exchange->getPublicKeyArraySize() != 65 || exchange->getPublicKey(0) != 4) { fail(47); return; }
        }
        legacyHandshake->phase = clientAuthRequested ? Phase::SERVER_WAIT_CLIENT_CERTIFICATE_VERIFY : Phase::SERVER_WAIT_CLIENT_CCS;
        applyChangeCipherSpec12();
        return;
    }
    if (phase == Phase::SERVER_WAIT_CLIENT_CERTIFICATE_VERIFY) {
        if (type != 15) { fail(10); return; }
        auto verify = body.popAtFront<tls::TlsCertificateVerify>(body.getDataLength());
        if (verify->getSignatureScheme() != 0x0403 || verify->getSignatureArraySize() == 0) { fail(47); return; }
        legacyHandshake->phase = Phase::SERVER_WAIT_CLIENT_CCS;
        applyChangeCipherSpec12();
        return;
    }
    if (phase == Phase::CLIENT_WAIT_SERVER_TICKET) {
        if (type != 4) { fail(10); return; }
        receiveSessionTicket12(&body);
        return;
    }
    if (phase == Phase::CLIENT_WAIT_SERVER_FINISHED || phase == Phase::SERVER_WAIT_CLIENT_FINISHED) {
        if (type != 20 || bytes.size() != 12) { fail(10); return; }
        peerFinishedProcessed = true;
        legacyHandshake->phase = Phase::COMPLETE;
        owner.cancelEvent(retransmissionTimer.get());
        bool abbreviated = selection.authenticationMode == tls::TlsHandshakeParameters::AuthenticationMode::TICKET_PSK;
        if (serverSide != abbreviated)
            sendFinalFlight12();
        else {
            flight.reset();
            owner.establish(handle);
        }
        return;
    }
    fail(10);
}

} // namespace inet::dtls
