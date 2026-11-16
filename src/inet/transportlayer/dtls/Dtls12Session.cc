// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include <algorithm>
#include "inet/transportlayer/dtls/Dtls12HelloVerifyRequest_m.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"

namespace inet::dtls {

template<typename T>
static std::map<uint16_t, std::vector<uint8_t>> extensions12(const Ptr<T>& hello)
{
    std::map<uint16_t, std::vector<uint8_t>> result;
    auto size = hello->getExtensionsArraySize();
    for (size_t offset = 0; offset < size;) {
        if (size - offset < 4)
            throw cRuntimeError("Truncated DTLS 1.2 Hello extension");
        auto type = uint16_t((uint16_t(hello->getExtensions(offset)) << 8) | hello->getExtensions(offset + 1));
        auto length = size_t((uint16_t(hello->getExtensions(offset + 2)) << 8) | hello->getExtensions(offset + 3));
        offset += 4;
        if (length > size - offset || result.count(type))
            throw cRuntimeError("Invalid DTLS 1.2 Hello extension vector");
        auto& bytes = result[type];
        for (size_t i = 0; i < length; ++i)
            bytes.push_back(hello->getExtensions(offset + i));
        offset += length;
    }
    return result;
}

std::vector<uint8_t> DtlsAssociation::offeredSessionId12() const
{
    if (!offeredSession || offeredSession->version != DTLS_12)
        return {};
    if (!offeredSession->ticket.empty())
        return {clientRandom.begin(), clientRandom.begin() + 16};
    return offeredSession->sessionId;
}

void DtlsAssociation::appendClientSession12(const Ptr<DtlsClientHello>& hello, std::vector<uint8_t>& extensions)
{
    if (!engine.par("resumptionEnabled").boolValue())
        return;
    auto sessionId = offeredSessionId12();
    hello->setSessionIdArraySize(sessionId.size());
    for (size_t i = 0; i < sessionId.size(); ++i)
        hello->setSessionId(i, sessionId[i]);
    if (engine.par("dtls12SessionTicketsEnabled").boolValue()) {
        std::vector<uint8_t> ticket;
        if (offeredSession && offeredSession->version == DTLS_12)
            ticket = offeredSession->ticket;
        extensions.insert(extensions.end(), {0, 35, uint8_t(ticket.size() >> 8), uint8_t(ticket.size())});
        extensions.insert(extensions.end(), ticket.begin(), ticket.end());
    }
}

void DtlsAssociation::prepareServerSession12(const Ptr<const DtlsClientHello>& hello)
{
    if (!engine.par("resumptionEnabled").boolValue())
        return;
    auto extensions = extensions12(hello);
    auto ticket = extensions.find(35);
    std::vector<uint8_t> sessionId;
    for (size_t i = 0; i < hello->getSessionIdArraySize(); ++i)
        sessionId.push_back(hello->getSessionId(i));
    if (ticket != extensions.end() && !ticket->second.empty()) {
        if (engine.par("dtls12SessionTicketsEnabled").boolValue())
            offeredSession = engine.sessions.find(ticket->second, true, simTime(), DTLS_12);
    }
    else
        offeredSession = engine.sessions.findSessionId(sessionId, true, simTime());
    if (offeredSession && (offeredSession->cipherSuite != selection.cipherSuite || offeredSession->peerIdentity != engine.par("identity").stdstringValue()))
        offeredSession.reset();
    if (offeredSession) {
        selection.authenticationMode = tls::TlsHandshakeParameters::AuthenticationMode::TICKET_PSK;
        clientAuthRequested = false;
        legacyHandshake->phase = LegacyHandshake::Phase::SERVER_WAIT_CLIENT_CCS;
        legacyHandshake->sessionId = sessionId;
    }
    bool issueTicket = ticket != extensions.end() && engine.par("dtls12SessionTicketsEnabled").boolValue();
    if (engine.par("maxSessions").intValue() <= 0 || engine.nextTicketSerial == 0)
        return;
    DtlsSessionStore::Entry entry;
    entry.serverSide = true;
    entry.version = DTLS_12;
    entry.cipherSuite = selection.cipherSuite;
    entry.peerAddress = peerAddress;
    entry.peerPort = peerPort;
    entry.peerIdentity = engine.par("identity").stdstringValue();
    entry.issuedAt = simTime();
    entry.expiresAt = simTime() + SimTime(engine.par("ticketLifetime").doubleValue());
    if (offeredSession && !issueTicket) {
        legacyHandshake->pendingSession = *offeredSession;
        return;
    }
    auto serial = engine.nextTicketSerial++;
    std::vector<uint8_t> identifier;
    for (auto value : {uint64_t(engine.getId()), serial})
        for (int shift = 56; shift >= 0; shift -= 8)
            identifier.push_back(value >> shift);
    if (issueTicket) {
        entry.ticket = identifier;
        legacyHandshake->ticketExpected = true;
    }
    else {
        entry.sessionId = identifier;
        legacyHandshake->sessionId = identifier;
    }
    legacyHandshake->pendingSession = std::move(entry);
}

void DtlsAssociation::fillServerSessionHello12(const Ptr<tls::TlsServerHello>& hello)
{
    hello->setSessionIdArraySize(legacyHandshake->sessionId.size());
    for (size_t i = 0; i < legacyHandshake->sessionId.size(); ++i)
        hello->setSessionId(i, legacyHandshake->sessionId[i]);
    std::vector<uint8_t> extensions;
    if (legacyHandshake->ticketExpected)
        extensions = {0, 35, 0, 0};
    if (heartbeatNegotiated)
        extensions.insert(extensions.end(), {0, 15, 0, 1, uint8_t(engine.par("heartbeatAcceptRequests").boolValue() ? 1 : 2)});
    if (cidNegotiated) {
        extensions.insert(extensions.end(), {0, 54, 0, uint8_t(1 + localCid.size()), uint8_t(localCid.size())});
        extensions.insert(extensions.end(), localCid.begin(), localCid.end());
    }
    hello->setExtensionsPresent(!extensions.empty());
    hello->setExtensionsArraySize(extensions.size());
    for (size_t i = 0; i < extensions.size(); ++i)
        hello->setExtensions(i, extensions[i]);
    hello->setChunkLength(B(38 + legacyHandshake->sessionId.size() + (extensions.empty() ? 0 : 2 + extensions.size())));
}

bool DtlsAssociation::processSessionHello12(const Ptr<const tls::TlsServerHello>& hello)
{
    auto extensions = extensions12(hello);
    for (const auto& extension : extensions) {
        if (extension.first == 15 && engine.par("heartbeatEnabled").boolValue()) {
            if (extension.second.size() != 1 || (extension.second[0] != 1 && extension.second[0] != 2))
                throw cRuntimeError("Invalid DTLS 1.2 server heartbeat extension");
            heartbeatNegotiated = true;
            peerAllowsHeartbeat = extension.second[0] == 1;
            continue;
        }
        if (extension.first == 54 && cidOffered) {
            if (extension.second.empty() || extension.second[0] != extension.second.size() - 1)
                throw cRuntimeError("Invalid DTLS 1.2 server CID extension");
            peerCid.assign(extension.second.begin() + 1, extension.second.end());
            cidNegotiated = true;
            if (engine.initialPayloadBudget() <= int64_t(12 + recordOverhead())) {
                engine.failAssociation(id, "DTLS 1.2 peer CID cannot fit protected handshake fragments", 47);
                return false;
            }
            continue;
        }
        if (extension.first != 35 || !engine.par("resumptionEnabled").boolValue() || !engine.par("dtls12SessionTicketsEnabled").boolValue()) {
            engine.failAssociation(id, "Unsupported DTLS 1.2 ServerHello extension", 110);
            return false;
        }
        if (!extension.second.empty())
            throw cRuntimeError("Invalid DTLS 1.2 server ticket extension");
    }
    legacyHandshake->ticketExpected = extensions.count(35);
    for (size_t i = 0; i < hello->getSessionIdArraySize(); ++i)
        legacyHandshake->sessionId.push_back(hello->getSessionId(i));
    auto offeredId = offeredSessionId12();
    bool resumed = !offeredId.empty() && legacyHandshake->sessionId == offeredId;
    if (resumed) {
        if (!offeredSession || offeredSession->version != DTLS_12 || offeredSession->cipherSuite != selection.cipherSuite || offeredSession->expiresAt <= simTime())
            throw cRuntimeError("Incompatible DTLS 1.2 resumed session");
        selection.authenticationMode = tls::TlsHandshakeParameters::AuthenticationMode::TICKET_PSK;
        clientAuthRequested = false;
        legacyHandshake->pendingSession = *offeredSession;
        legacyHandshake->phase = legacyHandshake->ticketExpected ? LegacyHandshake::Phase::CLIENT_WAIT_SERVER_TICKET : LegacyHandshake::Phase::CLIENT_WAIT_SERVER_CCS;
    }
    else if (engine.par("resumptionEnabled").boolValue()) {
        DtlsSessionStore::Entry entry;
        entry.version = DTLS_12;
        entry.cipherSuite = selection.cipherSuite;
        entry.peerAddress = peerAddress;
        entry.peerPort = peerPort;
        entry.peerIdentity = engine.par("identity").stdstringValue();
        entry.sessionId = legacyHandshake->sessionId;
        entry.issuedAt = simTime();
        entry.expiresAt = simTime() + SimTime(engine.par("ticketLifetime").doubleValue());
        legacyHandshake->pendingSession = std::move(entry);
    }
    if (legacyHandshake->ticketExpected && legacyHandshake->pendingSession) {
        legacyHandshake->pendingSession->ticket.clear();
        legacyHandshake->pendingSession->sessionId.clear();
    }
    applyChangeCipherSpec12();
    return true;
}

void DtlsAssociation::appendSessionTicket12()
{
    if (!legacyHandshake->ticketExpected)
        return;
    auto body = makeShared<Dtls12NewSessionTicket>();
    if (legacyHandshake->pendingSession) {
        body->setTicketLifetime(engine.par("ticketLifetime").doubleValue());
        auto& ticket = legacyHandshake->pendingSession->ticket;
        body->setTicketArraySize(ticket.size());
        for (size_t i = 0; i < ticket.size(); ++i)
            body->setTicket(i, ticket[i]);
    }
    body->setChunkLength(B(6 + body->getTicketArraySize()));
    appendHandshake(4, nextLocalMessage++, 0, body, false, false);
}

void DtlsAssociation::receiveSessionTicket12(Packet *body)
{
    auto ticket = body->popAtFront<Dtls12NewSessionTicket>(body->getDataLength());
    if (!legacyHandshake->ticketExpected || !legacyHandshake->pendingSession)
        throw cRuntimeError("Unexpected DTLS 1.2 session ticket");
    if (ticket->getTicketArraySize() == 0)
        legacyHandshake->pendingSession.reset();
    else {
        auto& entry = *legacyHandshake->pendingSession;
        entry.ticket.clear();
        entry.sessionId.clear();
        for (size_t i = 0; i < ticket->getTicketArraySize(); ++i)
            entry.ticket.push_back(ticket->getTicket(i));
        auto lifetime = ticket->getTicketLifetime() == 0 ? engine.par("ticketLifetime").doubleValue() : std::min<uint32_t>(604800, ticket->getTicketLifetime());
        entry.issuedAt = simTime();
        entry.expiresAt = simTime() + SimTime(lifetime);
    }
    legacyHandshake->phase = LegacyHandshake::Phase::CLIENT_WAIT_SERVER_CCS;
    applyChangeCipherSpec12();
}

void DtlsAssociation::cacheSession12()
{
    if (!legacyHandshake->pendingSession)
        return;
    auto& entry = *legacyHandshake->pendingSession;
    entry.peerCertificateAuthenticated = peerCertificateAuthenticated;
    entry.authenticatedPeerIdentity = authenticatedPeerIdentity;
    engine.sessions.insert(entry, simTime());
}

void DtlsAssociation::invalidateSessions12()
{
    if (offeredSession && offeredSession->version == DTLS_12)
        engine.sessions.remove(*offeredSession);
    if (legacyHandshake->pendingSession)
        engine.sessions.remove(*legacyHandshake->pendingSession);
}

} // namespace inet::dtls
