// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include <algorithm>
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/transportlayer/dtls/Dtls12HelloVerifyRequest_m.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"

namespace inet::dtls {

bool Dtls::isProcessedHello12(const DtlsPath& path, uint16_t sequence, const std::vector<uint8_t>& encoded) const
{
    for (const auto& entry : associations) {
        const auto& value=*entry.second;
        if (!value.isDtls12() || !value.serverSide || !sameTuple(value.currentPath(),path)) continue;
        auto previous=value.processedHandshakes.find(sequence);
        if (previous!=value.processedHandshakes.end() && previous->second.type==1 && previous->second.body==encoded)
            return true;
    }
    return false;
}

void Dtls::requestCookieFromPrefix12(PrevalidationAttempt *attempt)
{
    auto prefix = attempt->reassembly.getContiguousPrefix(0);
    if (prefix.size() < 36 || prefix[0] != 0xfe || prefix[1] != 0xfd)
        return;
    auto sessionLength = prefix[34];
    if (sessionLength > 32 || prefix.size() < size_t(36 + sessionLength) || prefix[35 + sessionLength] != 0)
        return;
    auto hello = makeShared<DtlsClientHello>();
    for (size_t i = 0; i < 32; ++i)
        hello->setRandom(i, prefix[i + 2]);
    sendRetry12(attempt, hello, {});
}

void Dtls::sendRetry12(PrevalidationAttempt *attempt, const Ptr<const DtlsClientHello>& hello, const std::vector<uint8_t>& encoded)
{
    if (!attempt || attempt->failing)
        return;
    auto handle = attempt->id;
    if (attempt->retryFlight) {
        attempt->retryFlight->requestRetransmission();
        attempt->retryBurst = attempt->retryQueued;
        pumpRetry(handle);
        return;
    }
    auto started = attempt->deadline->getArrivalTime() - SimTime(par("handshakeTimeout").doubleValue());
    attempt->cookie = makeCookie(simTime().raw(),started.raw(),attempt->localAddress,attempt->localPort,attempt->peerAddress,attempt->peerPort,hello);
    if (attempt->cookie.size() > 255 || initialPayloadBudget() <= 25) {
        removePrevalidation(handle);
        emit(recordDroppedSignal,long(RESOURCE_LIMIT));
        return;
    }
    attempt->initialHello = encoded;
    attempt->selection.version = Association::DTLS_12;
    auto body = makeShared<Dtls12HelloVerifyRequest>();
    body->setCookieArraySize(attempt->cookie.size());
    for (size_t i=0; i<attempt->cookie.size(); ++i) body->setCookie(i,attempt->cookie[i]);
    body->setChunkLength(B(3+attempt->cookie.size()));
    body->markImmutable();
    auto flight = std::make_shared<DtlsHandshakeFlight>();
    Packet source("DTLS 1.2 cookie body",body);
    auto count = size_t(body->getChunkLength().get<B>());
    auto maximum = size_t(initialPayloadBudget()-25);
    for (size_t offset=0; offset<count; offset+=maximum) {
        auto length = std::min(maximum,count-offset);
        flight->add(3,0,count,offset,0,source.peekDataAt(B(offset),B(length)));
    }
    attempt->retryFlight = flight;
    attempt->retryGenerating = true;
    auto delay = SimTime(par("handshakeOutDelay").doubleValue());
    auto eligible = [this,handle,flight]() { auto value=findPrevalidation(handle); return value && value->retryFlight==flight && !value->failing; };
    if (!enqueueProcessing(outputProcessing.get(),handle,count+12,delay,[this,handle,flight]() {
        auto value=findPrevalidation(handle);
        if (!value || value->retryFlight!=flight) return;
        value->retryGenerating=false;
        for (size_t i=0; i<flight->size(); ++i) flight->markReady(i);
        pumpRetry(handle);
    },eligible)) {
        removePrevalidation(handle);
        emit(recordDroppedSignal,long(RESOURCE_LIMIT));
    }
}

bool Dtls::handleClientHello12(PrevalidationAttempt *attempt, const Ptr<const DtlsClientHello>& hello, uint16_t sequence, const std::vector<uint8_t>& encoded)
{
    if (!allowDtls12)
        return false;
    tls::TlsHandshakeParameters::Offer offer;
    auto error = tls::TlsHandshakeParameters::decodeOffer(hello,offer);
    auto fail = [&](uint8_t code) { if (attempt) failPrevalidation(attempt,code); };
    if (!error.empty()) { fail(error == "invalid heartbeat mode" ? 47 : 50); return true; }
    auto contains = [](const auto& values, auto value) { return std::find(values.begin(),values.end(),value)!=values.end(); };
    if (allowDtls13 && contains(offer.versions,tls::TlsHandshakeParameters::DTLS_13))
        return false;
    if ((!offer.versions.empty() && !contains(offer.versions,Association::DTLS_12)) || hello->getLegacyVersion()!=Association::DTLS_12) { fail(70); return true; }
    bool certificate = par("profile").stdstringValue()=="certificate";
    auto suite = certificate ? Association::ECDHE_ECDSA_AES_128_GCM_SHA256 : Association::PSK_AES_128_GCM_SHA256;
    if (!contains(offer.cipherSuites,suite) || (certificate &&
        (!contains(offer.groups,tls::TlsHandshakeParameters::SECP256R1) || !contains(offer.signatures,tls::TlsHandshakeParameters::ECDSA_P256_SHA256)))) { fail(40); return true; }
    if (!attempt || listener==nullptr || operationalState!=OPERATING)
        return true;
    if (hello->getCompressionMethodsArraySize()!=1 || hello->getCompressionMethods(0)!=0) { fail(47); return true; }
    if (par("connectionIdEnabled").boolValue() && offer.hasConnectionId && !offer.connectionId.empty() &&
        initialPayloadBudget() <= int64_t(50 + offer.connectionId.size())) { fail(47); return true; }
    std::vector<uint8_t> cookie;
    for (size_t i=0; i<hello->getLegacyCookieArraySize(); ++i) cookie.push_back(hello->getLegacyCookie(i));
    std::vector<AssociationId> replaced;
    for (const auto& entry : associations) {
        const auto& old=*entry.second;
        if (!old.isDtls12() || old.localAddress!=attempt->localAddress || old.localPort!=attempt->localPort ||
            old.peerAddress!=attempt->peerAddress || old.peerPort!=attempt->peerPort) continue;
        bool sameRandom=true;
        for (size_t i=0; i<32; ++i) sameRandom &= old.clientRandom[i]==hello->getRandom(i);
        auto previous=old.processedHandshakes.find(sequence);
        if (sameRandom && (sequence==0 || (previous!=old.processedHandshakes.end() && previous->second.body==encoded))) {
            auto oldId=entry.first;
            removePrevalidation(attempt->id);
            auto current=associations.find(oldId);
            if (current!=associations.end() && current->second->state!=HandshakeState::ESTABLISHED)
                current->second->retransmitFlight12();
            return true;
        }
        replaced.push_back(entry.first);
    }
    auto started = attempt->deadline->getArrivalTime() - SimTime(par("handshakeTimeout").doubleValue());
    if (sequence==0 && cookie.empty() && (par("cookieRequired").boolValue() || !replaced.empty() || attempt->retryFlight)) {
        sendRetry12(attempt,hello,encoded);
        return true;
    }
    bool validated = false;
    if (!cookie.empty()) {
        auto lifetime = SimTime(par("cookieLifetime").doubleValue()).raw();
        if (sequence!=1 || cookie.size()<16 || lifetime<=0) { fail(47); return true; }
        auto issued=readUint64(cookie,0), originalStart=readUint64(cookie,8);
        auto now=uint64_t(simTime().raw());
        auto timeout=SimTime(par("handshakeTimeout").doubleValue()).raw();
        if (originalStart>issued || issued>now || now-issued>uint64_t(lifetime) || timeout<=0 || now-originalStart>=uint64_t(timeout) ||
            cookie!=makeCookie(issued,originalStart,attempt->localAddress,attempt->localPort,attempt->peerAddress,attempt->peerPort,hello)) { fail(47); return true; }
        started=SimTime::fromRaw(originalStart);
        validated=true;
    }
    else if (sequence!=0 || par("cookieRequired").boolValue()) { fail(47); return true; }
    auto maximum=par("maxHalfOpen").intValue();
    auto timeout=SimTime(par("handshakeTimeout").doubleValue());
    auto deadline=std::min(attempt->deadline->getArrivalTime(),started+timeout);
    if (maximum<=0 || countHalfOpen()>size_t(maximum) || nextAssociationId==0 || deadline<=simTime()) {
        removePrevalidation(attempt->id);
        emit(recordDroppedSignal,long(RESOURCE_LIMIT));
        return true;
    }
    auto association=std::make_unique<Association>(*this,par("maxReassemblyBytes").intValue());
    association->id=nextAssociationId++;
    association->callback=listener;
    association->localAddress=attempt->localAddress;
    association->localPort=attempt->localPort;
    association->peerAddress=attempt->peerAddress;
    association->peerPort=attempt->peerPort;
    association->startedAt=started;
    association->offer=offer;
    for (size_t i=0; i<32; ++i) association->clientRandom[i]=hello->getRandom(i);
    association->initializeDtls12(true,sequence);
    if (association->cidNegotiated && par("receiveConnectionId").boolValue()) {
        association->localCid=allocateConnectionId(association->id);
        if (association->localCid.empty()) {
            removePrevalidation(attempt->id);
            emit(recordDroppedSignal,long(RESOURCE_LIMIT));
            return true;
        }
    }
    association->prepareServerSession12(hello);
    association->addressValidated=validated;
    association->receivedBytes=attempt->receivedBytes;
    association->sentBytes=attempt->sentBytes;
    association->nextEpoch0Record=attempt->nextRecord;
    association->processedHandshakes[sequence]={1,encoded};
    association->handshakeDeadline=std::make_unique<cMessage>("dtlsHandshakeDeadline");
    auto handle=association->id;
    auto path=association->currentPath();
    auto attemptId=attempt->id;
    scheduleAt(deadline,association->handshakeDeadline.get());
    associations.emplace(handle,std::move(association));
    removePrevalidation(attemptId);
    for (auto oldId : replaced)
        finishClose(oldId,false);
    auto current=associations.find(handle);
    if (current==associations.end() || listener==nullptr || operationalState!=OPERATING) return true;
    listener->accepted(handle,path.localAddress,path.localPort,path.peerAddress,path.peerPort);
    current=associations.find(handle);
    if (current!=associations.end()) current->second->sendServerFlight12();
    return true;
}

} // namespace inet::dtls
