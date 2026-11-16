// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsAssociation.h"

#include <algorithm>
#include <limits>

#include "inet/transportlayer/dtls/Dtls.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"

namespace inet::dtls {

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
    owner.sendPlainHandshake(handle, 1, retried ? 1 : 0, hello);
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
    size_t flightBytes = 13 + 12 + 40 + 46 + (useCertificate ? 0 : 6) + (5 + 12 + 2 + 1 + 16) + (5 + 12 + 32 + 1 + 16);
    if (useCertificate) {
        certificate = owner.makeCertificate();
        verify = owner.makeCertificateVerify();
        flightBytes += (5 + 12 + 1 + 16) * 2 + certificate->getChunkLength().get<B>() + verify->getChunkLength().get<B>();
        if (clientAuth)
            flightBytes += 5 + 12 + 11 + 1 + 16;
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
    owner.sendProtectedHandshake(handle, 20, messageSequence, finished, false, true);
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
}

void DtlsAssociation::processHandshake(uint64_t sequence, Packet *body)
{
    if (!receiveEpoch2->isAcceptable(sequence))
        return;
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
        receiveEpoch2->accept(sequence);
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
        receiveEpoch2->accept(sequence);
        ++nextPeerMessage;
        clientAuthRequested = true;
    }
    else if (state == HandshakeState::CLIENT_WAIT_SERVER_CERTIFICATE && handshake->getMessageType() == 11) {
        auto alert = engine.checkCertificate(body);
        if (alert != 0) {
            engine.failAssociation(id, "invalid peer certificate", alert);
            return;
        }
        receiveEpoch2->accept(sequence);
        ++nextPeerMessage;
        state = HandshakeState::CLIENT_WAIT_SERVER_CERTIFICATE_VERIFY;
    }
    else if (state == HandshakeState::CLIENT_WAIT_SERVER_CERTIFICATE_VERIFY && handshake->getMessageType() == 15) {
        auto alert = engine.checkCertificateVerify(body);
        if (alert != 0) {
            engine.failAssociation(id, "invalid certificate signature", alert);
            return;
        }
        receiveEpoch2->accept(sequence);
        ++nextPeerMessage;
        state = HandshakeState::CLIENT_WAIT_SERVER_FINISHED;
    }
    else if (state == HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE && handshake->getMessageType() == 11) {
        auto alert = engine.checkCertificate(body);
        if (alert != 0) {
            engine.failAssociation(id, "invalid peer certificate", alert);
            return;
        }
        receiveEpoch2->accept(sequence);
        ++nextPeerMessage;
        state = HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE_VERIFY;
    }
    else if (state == HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE_VERIFY && handshake->getMessageType() == 15) {
        auto alert = engine.checkCertificateVerify(body);
        if (alert != 0) {
            engine.failAssociation(id, "invalid certificate signature", alert);
            return;
        }
        receiveEpoch2->accept(sequence);
        ++nextPeerMessage;
        state = HandshakeState::SERVER_WAIT_CLIENT_FINISHED;
    }
    else if (state == HandshakeState::CLIENT_WAIT_SERVER_FINISHED && handshake->getMessageType() == 20) {
        if (body->getDataLength() != B(32)) {
            engine.failAssociation(id, "invalid Finished length", 50);
            return;
        }
        receiveEpoch2->accept(sequence);
        state = HandshakeState::CLIENT_WAIT_FINISHED_HANDOFF;
        auto handle = id;
        auto& owner = engine;
        auto nextMessage = serverMessageBase + 1;
        bool sendClientCertificate = clientAuthRequested;
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
        owner.sendProtectedHandshake(handle, 20, nextMessage, finished, true);
    }
    else if (state == HandshakeState::SERVER_WAIT_CLIENT_FINISHED && handshake->getMessageType() == 20) {
        if (peerFinishedProcessed)
            return;
        if (body->getDataLength() != B(32)) {
            engine.failAssociation(id, "invalid Finished length", 50);
            return;
        }
        receiveEpoch2->accept(sequence);
        peerFinishedProcessed = true;
        peerFinishedRecord = sequence;
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
