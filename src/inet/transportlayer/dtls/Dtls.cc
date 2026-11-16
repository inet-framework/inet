// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/Dtls.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <limits>

#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/Icmpv4ErrorTag_m.h"
#include "inet/networklayer/common/Icmpv6ErrorTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/dtls/DtlsPlaintextHeader_m.h"
#include "inet/transportlayer/dtls/DtlsRecordProtection.h"
#include "inet/transportlayer/tls/TlsHandshakeParameters.h"

namespace inet::dtls {

Define_Module(Dtls);

const Protocol dtlsProtocol("dtls", "Datagram Transport Layer Security", Protocol::TransportLayer);
simsignal_t Dtls::packetSentToLowerSignal = registerSignal("packetSentToLower");
simsignal_t Dtls::packetReceivedFromLowerSignal = registerSignal("packetReceivedFromLower");
simsignal_t Dtls::packetSentToUpperSignal = registerSignal("packetSentToUpper");
simsignal_t Dtls::recordAcceptedSignal = registerSignal("recordAccepted");
simsignal_t Dtls::recordDroppedSignal = registerSignal("recordDropped");
simsignal_t Dtls::associationEstablishedSignal = registerSignal("associationEstablished");
simsignal_t Dtls::associationFailedSignal = registerSignal("associationFailed");
simsignal_t Dtls::establishmentTimeSignal = registerSignal("establishmentTime");
simsignal_t Dtls::recordOverheadSignal = registerSignal("recordOverhead");
simsignal_t Dtls::reassemblyBytesSignal = registerSignal("reassemblyBytes");
simsignal_t Dtls::handshakeRetransmissionSignal = registerSignal("handshakeRetransmission");
simsignal_t Dtls::processingDelaySignal = registerSignal("processingDelay");

Dtls::~Dtls()
{
    discardAll("module destroyed", false);
}

bool Dtls::enqueueProcessing(DtlsProcessingQueue *queue, AssociationId id, size_t bytes, simtime_t delay, DtlsProcessingQueue::Completion completion,
                             DtlsProcessingQueue::Eligibility eligible, DtlsProcessingQueue::Completion skipped,
                             DtlsProcessingQueue::Completion started, bool recordDelay)
{
    return queue && queue->enqueue(id, bytes, delay, std::move(completion), std::move(eligible), std::move(skipped), [this, delay, started, recordDelay]() {
        if (started)
            started();
        if (recordDelay)
            emit(processingDelaySignal, delay.dbl());
    });
}

bool Dtls::hasConsistentEncoding(const Packet *packet)
{
    if (packet->getDataLength() == b(0))
        return true;
    try {
        MemoryOutputStream stream;
        Chunk::serialize(stream, packet->peekData(), b(0), packet->getDataLength());
        return stream.getLength() == packet->getDataLength();
    }
    catch (const cRuntimeError&) {
        return false;
    }
}

simtime_t Dtls::processingDelay(AssociationId id, const char *operation)
{
    auto it = associations.find(id);
    bool earlyProtection = it != associations.end() && it->second->earlyOffered && std::string(operation) == "protectOutDelay";
    if (it == associations.end() || (it->second->state == HandshakeState::CLIENT_WAIT_SERVER_HELLO && !earlyProtection))
        return SimTime(par(operation).doubleValue());
    auto code = it->second->state == HandshakeState::CLIENT_WAIT_SERVER_HELLO ? it->second->offeredSession->cipherSuite : it->second->selection.cipherSuite;
    auto suite = tls::TlsHandshakeParameters::findCipherSuite(code);
    std::string name(suite->parameterPrefix);
    name += char(operation[0] - 'a' + 'A');
    name += operation + 1;
    return SimTime(par(name.c_str()).doubleValue());
}

void Dtls::handleStartOperation(LifecycleOperation *)
{
    preferences = tls::TlsHandshakeParameters::parsePreferences(par("cipherSuites"), par("keyExchangeGroups"), par("signatureSchemes"));
    if (par("pathMtuDiscoveryEnabled").boolValue()) {
        auto base = par("pathMtuBase").intValue();
        auto ceiling = par("pathMtuProbeCeiling").intValue();
        auto minimum = 49 + (par("connectionIdEnabled").boolValue() ? 8 : 0);
        auto timeout = par("pathMtuProbeTimeout").doubleValue();
        auto confirmation = par("pathMtuConfirmationInterval").doubleValue();
        auto raise = par("pathMtuRaiseInterval").doubleValue();
        if (base < minimum || ceiling < base || ceiling > par("udpPayloadLimit").intValue() || ceiling > 16406 ||
            !std::isfinite(timeout) || timeout < 1 || !std::isfinite(confirmation) || confirmation <= 0 ||
            !std::isfinite(raise) || raise <= confirmation || SimTime(confirmation) <= SIMTIME_ZERO || SimTime(raise) <= SimTime(confirmation))
            throw cRuntimeError("Invalid DTLS path MTU base, ceiling or timer profile");
    }
    if (par("heartbeatEnabled").boolValue() && (!std::isfinite(par("probeTimeout").doubleValue()) || par("probeTimeout").doubleValue() < 1))
        throw cRuntimeError("DTLS probeTimeout must be finite and at least one second");
    if (std::string(par("profile").stringValue()) == "psk" && std::none_of(preferences.cipherSuites.begin(), preferences.cipherSuites.end(), [](auto code) {
        return tls::TlsHandshakeParameters::findCipherSuite(code)->hashBytes == 32;
    }))
        throw cRuntimeError("The configured external PSK requires a SHA-256 cipher suite");
    auto sessionLimit = par("maxSessions").intValue();
    auto lifetime = par("ticketLifetime").doubleValue();
    if (par("resumptionEnabled").boolValue() && (sessionLimit <= 0 || !std::isfinite(lifetime) || lifetime < 1 || lifetime > 604800 || lifetime != uint32_t(lifetime)))
        throw cRuntimeError("Invalid DTLS session capacity or integral ticket lifetime");
    sessions.configure(par("resumptionEnabled").boolValue() ? sessionLimit : 0);
    if (par("connectionIdEnabled").boolValue() && (par("maxReceiveConnectionIds").intValue() < 1 || par("maxReceiveConnectionIds").intValue() > 4))
        throw cRuntimeError("maxReceiveConnectionIds must be between 1 and 4");
    auto earlyLimit = par("earlyDataBytes").intValue();
    if (par("earlyDataEnabled").boolValue() && (!par("resumptionEnabled").boolValue() || earlyLimit <= 0 || uint64_t(earlyLimit) > UINT32_MAX))
        throw cRuntimeError("DTLS early data requires resumption and a positive 32-bit allowance");
    auto processingBytes = par("maxProcessingBytes").intValue();
    if (processingBytes <= 0)
        throw cRuntimeError("maxProcessingBytes must be positive");
    auto rekeyAfter = par("rekeyAfterRecords").intValue();
    if (rekeyAfter < 1 || uint64_t(rekeyAfter) > Association::MAX_REKEY_AFTER_RECORDS)
        throw cRuntimeError("rekeyAfterRecords must be between 1 and 2^23");
    inputProcessing = std::make_unique<DtlsProcessingQueue>(this, "dtlsInputProcessing", processingBytes);
    outputProcessing = std::make_unique<DtlsProcessingQueue>(this, "dtlsOutputProcessing", processingBytes);
    udpSocket.setOutputGate(gate("socketOut"));
    udpSocket.setCallback(this);
    const char *address = par("localAddress");
    udpSocket.bind(*address ? L3AddressResolver().resolve(address) : L3Address(), par("localPort"));
}

void Dtls::handleStopOperation(LifecycleOperation *)
{
    sessions.clear();
    cidIndex.clear();
    listener = nullptr;
    if (inputProcessing)
        inputProcessing->clear();
    std::set<AssociationId> closingIds;
    for (const auto& entry : associations)
        if (entry.second->closing)
            closingIds.insert(entry.first);
    if (outputProcessing)
        outputProcessing->cancelExcept(closingIds);
    std::vector<uint64_t> attempts;
    for (const auto& attempt : prevalidationAttempts)
        attempts.push_back(attempt->id);
    for (auto id : attempts)
        removePrevalidation(id);
    std::vector<AssociationId> ids;
    for (const auto& entry : associations)
        ids.push_back(entry.first);
    for (auto id : ids) {
        auto it = associations.find(id);
        if (it != associations.end() && it->second->state == HandshakeState::ESTABLISHED)
            beginClose(id);
        else
            finishClose(id, true);
    }
    if (associations.empty())
        udpSocket.close();
    delayActiveOperationFinish(par("closeTimeout"));
}

void Dtls::handleCrashOperation(LifecycleOperation *)
{
    sessions.clear();
    listener = nullptr;
    discardAll("node crashed", true);
    udpSocket.destroy();
}

void Dtls::handleMessageWhenUp(cMessage *message)
{
    if (inputProcessing && inputProcessing->isTimer(message))
        inputProcessing->handleTimer();
    else if (outputProcessing && outputProcessing->isTimer(message))
        outputProcessing->handleTimer();
    else if (message->isSelfMessage() && handleHandshakeDeadline(message))
        return;
    else if (udpSocket.belongsToSocket(message))
        udpSocket.processMessage(message);
    else if (auto tagged = dynamic_cast<ITaggedObject *>(message); tagged && par("migrationEnabled").boolValue() && tagged->getTags().findTag<SocketInd>()) {
        emit(recordDroppedSignal, (long)UNSUPPORTED_PATH);
        delete message;
    }
    else
        throw cRuntimeError("Unexpected message '%s' at DTLS engine", message->getName());
}

void Dtls::listen(ICallback *callback)
{
    Enter_Method("listen");
    if (operationalState != OPERATING)
        return;
    if (callback == nullptr)
        throw cRuntimeError("DTLS listener callback is null");
    if (listener != nullptr && listener != callback)
        throw cRuntimeError("DTLS listener already registered");
    listener = callback;
}

IDtls::AssociationId Dtls::open(const L3Address& peerAddress, int peerPort, ICallback *callback)
{
    return open(peerAddress, peerPort, callback, false);
}

bool Dtls::isResumed(AssociationId id) const
{
    auto it = associations.find(id);
    return it != associations.end() && it->second->state == HandshakeState::ESTABLISHED &&
        it->second->selection.authenticationMode == tls::TlsHandshakeParameters::AuthenticationMode::TICKET_PSK;
}

IDtls::AssociationId Dtls::open(const L3Address& peerAddress, int peerPort, ICallback *callback, bool resumeSession)
{
    return open(peerAddress, peerPort, callback, DtlsOpenOptions{resumeSession, false});
}

IDtls::AssociationId Dtls::open(const L3Address& peerAddress, int peerPort, ICallback *callback, const DtlsOpenOptions& options)
{
    Enter_Method("open");
    if (operationalState != OPERATING)
        return 0;
    if (callback == nullptr || peerAddress.isUnspecified() || peerPort <= 0 || peerPort > 65535)
        throw cRuntimeError("Invalid DTLS open request");
    auto halfOpenLimit = par("maxHalfOpen").intValue();
    auto timeout = SimTime(par("handshakeTimeout").doubleValue());
    if (halfOpenLimit <= 0 || timeout <= SIMTIME_ZERO)
        throw cRuntimeError("Invalid DTLS handshake limit or timeout");
    if (countHalfOpen() >= size_t(halfOpenLimit) || nextAssociationId == 0)
        return 0;
    auto reassemblyLimit = par("maxReassemblyBytes").intValue();
    if (reassemblyLimit < 0)
        throw cRuntimeError("Invalid DTLS reassembly limit");
    auto association = std::make_unique<Association>(*this, reassemblyLimit);
    association->id = nextAssociationId++;
    association->cidOffered = par("connectionIdEnabled").boolValue();
    association->startedAt = simTime();
    association->callback = callback;
    association->peerAddress = peerAddress;
    association->peerPort = peerPort;
    for (const auto& entry : associations)
        if (entry.second->state == HandshakeState::ESTABLISHED && entry.second->peerAddress == peerAddress && entry.second->peerPort == peerPort) {
            association->replacementId = entry.first;
            break;
        }
    association->selection.authenticationMode = std::string(par("profile").stringValue()) == "certificate" ? tls::TlsHandshakeParameters::AuthenticationMode::CERTIFICATE : tls::TlsHandshakeParameters::AuthenticationMode::EXTERNAL_PSK;
    if ((options.resumeSession || options.offerEarlyData) && par("resumptionEnabled").boolValue()) {
        std::vector<uint8_t> hashes;
        for (auto suite : preferences.cipherSuites)
            hashes.push_back(tls::TlsHandshakeParameters::findCipherSuite(suite)->hashBytes);
        association->offeredSession = sessions.select(peerAddress, peerPort, par("identity").stdstringValue(), tls::TlsHandshakeParameters::DTLS_13, simTime(), hashes);
    }
    association->earlyRequested = options.offerEarlyData;
    association->earlyOffered = options.offerEarlyData && par("earlyDataEnabled").boolValue() && association->offeredSession && association->offeredSession->earlyDataBytes != 0;
    if (association->earlyOffered && std::find(preferences.cipherSuites.begin(), preferences.cipherSuites.end(), association->offeredSession->cipherSuite) == preferences.cipherSuites.end())
        association->earlyOffered = false;
    auto replayWindow = par("replayWindowSize").intValue();
    if (replayWindow <= 0 || replayWindow > 65536)
        throw cRuntimeError("Invalid DTLS replay window size");
    association->receiveEpoch2 = std::make_unique<DtlsRecordState>(replayWindow);
    association->receiveEpoch3 = std::make_unique<DtlsRecordState>(replayWindow);
    if (association->cidOffered && par("receiveConnectionId").boolValue()) {
        association->localCid = allocateConnectionId(association->id);
        if (association->localCid.empty())
            return 0;
    }
    association->handshakeDeadline = std::make_unique<cMessage>("dtlsHandshakeDeadline");
    auto id = association->id;
    scheduleAt(simTime() + timeout, association->handshakeDeadline.get());
    associations.emplace(id, std::move(association));
    try {
        sendClientHello(id);
    }
    catch (...) {
        auto it = associations.find(id);
        if (it != associations.end()) {
            cancelEvent(it->second->handshakeDeadline.get());
            it->second->cancelTimers();
            associations.erase(it);
            removeConnectionIds(id);
        }
        throw;
    }
    return containsAssociation(id) ? id : 0;
}

bool Dtls::handleHandshakeDeadline(cMessage *message)
{
    for (const auto& attempt : prevalidationAttempts) {
        if (attempt->retryTimer.get() == message) {
            auto id = attempt->id;
            attempt->retryTimeout = std::min(SimTime(60), attempt->retryTimeout * 2);
            if (attempt->retryFlight) {
                attempt->retryFlight->requestRetransmission();
                attempt->retryBurst = attempt->retryQueued;
                pumpRetry(id);
            }
            return true;
        }
        if (attempt->deadline.get() == message) {
            auto id = attempt->id;
            removePrevalidation(id);
            return true;
        }
    }
    for (const auto& entry : associations) {
        if (entry.second->handleTimer(message))
            return true;
        if (entry.second->handshakeDeadline.get() == message) {
            auto id = entry.first;
            failAssociation(id, "handshake timeout");
            return true;
        }
    }
    return false;
}

Dtls::PrevalidationAttempt *Dtls::findPrevalidation(uint64_t id) const
{
    for (const auto& attempt : prevalidationAttempts)
        if (attempt->id == id)
            return attempt.get();
    return nullptr;
}

size_t Dtls::countHalfOpen() const
{
    size_t count = std::count_if(associations.begin(), associations.end(), [](const auto& entry) { return entry.second->state != HandshakeState::ESTABLISHED; });
    for (const auto& attempt : prevalidationAttempts) {
        bool ownedByCandidate = std::any_of(associations.begin(), associations.end(), [&](const auto& entry) {
            const auto& candidate = *entry.second;
            return candidate.serverSide && candidate.state != HandshakeState::ESTABLISHED &&
                   candidate.localAddress == attempt->localAddress && candidate.localPort == attempt->localPort &&
                   candidate.peerAddress == attempt->peerAddress && candidate.peerPort == attempt->peerPort;
        });
        if (!ownedByCandidate)
            ++count;
    }
    return count;
}

void Dtls::emitReassemblyBytes()
{
    size_t total = 0;
    auto add = [&total](size_t value) {
        total = value > size_t(LONG_MAX) - total ? size_t(LONG_MAX) : total + value;
    };
    for (const auto& entry : associations)
        add(entry.second->getReassemblyBytes());
    for (const auto& entry : prevalidationAttempts)
        add(entry->reassembly.getReservedBytes());
    emit(reassemblyBytesSignal, (long)total);
}

void Dtls::removePrevalidation(uint64_t id)
{
    auto it = std::find_if(prevalidationAttempts.begin(), prevalidationAttempts.end(),
        [id](const auto& entry) { return entry->id == id; });
    if (it == prevalidationAttempts.end())
        return;
    auto attempt = std::move(*it);
    prevalidationAttempts.erase(it);
    auto hadReassembly = attempt->reassembly.getReservedBytes() != 0;
    cancelEvent(attempt->deadline.get());
    cancelEvent(attempt->retryTimer.get());
    if (inputProcessing)
        inputProcessing->cancel(id);
    if (outputProcessing)
        outputProcessing->cancel(id);
    if (hadReassembly)
        emitReassemblyBytes();
}

void Dtls::sendAlert(const L3Address& localAddress, const L3Address& peerAddress, int peerPort, uint8_t description, bool encrypted, uint64_t sequence, uint64_t epoch,
                     const std::vector<uint8_t>& cid, int interfaceId)
{
    auto alert = makeShared<DtlsAlert>();
    alert->setLevel(2);
    alert->setDescription(description);
    auto packet = std::make_shared<std::unique_ptr<Packet>>(new Packet("DTLS fatal alert", alert));
    if (encrypted) {
        DtlsRecordProtection::protect(packet->get(), 21);
        auto header = makeShared<DtlsCiphertextHeader>();
        header->setEpochBits(epoch & 3);
        header->setSequenceNumber(sequence & 0xffff);
        header->setLength((*packet)->getDataLength().get<B>());
        header->setConnectionIdArraySize(cid.size());
        for (size_t i = 0; i < cid.size(); ++i)
            header->setConnectionId(i, cid[i]);
        header->setChunkLength(B(5 + cid.size()));
        (*packet)->insertAtFront(header);
    }
    else {
        auto header = makeShared<DtlsPlaintextHeader>();
        header->setContentType(21);
        header->setLength(2);
        (*packet)->insertAtFront(header);
    }
    (*packet)->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
    if (!localAddress.isUnspecified())
        (*packet)->addTagIfAbsent<L3AddressReq>()->setSrcAddress(localAddress);
    if (interfaceId >= 0)
        (*packet)->addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
    auto delay = encrypted ? SimTime(par("protectOutDelay").doubleValue()) : SIMTIME_ZERO;
    auto overhead = 22 + cid.size();
    if (!enqueueProcessing(outputProcessing.get(), 0, (*packet)->getByteLength(), delay, [this, peerAddress, peerPort, packet, encrypted, overhead]() {
        auto outgoing = packet->release();
        if (encrypted)
            emit(recordOverheadSignal, (long)overhead);
        emit(packetSentToLowerSignal, outgoing);
        sendUdp(outgoing, peerAddress, peerPort);
    }))
        emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
}

void Dtls::failAssociation(AssociationId id, const char *reason, int alertDescription)
{
    auto it = associations.find(id);
    if (it == associations.end())
        return;
    auto association = std::move(it->second);
    associations.erase(it);
    removeConnectionIds(id);
    auto hadReassembly = association->getReassemblyBytes() != 0;
    association->cancelTimers();
    cancelEvent(association->handshakeDeadline.get());
    if (inputProcessing)
        inputProcessing->cancel(id);
    if (outputProcessing)
        outputProcessing->cancel(id);
    if (hadReassembly)
        emitReassemblyBytes();
    if (alertDescription >= 0) {
        bool encrypted = association->state != HandshakeState::CLIENT_WAIT_SERVER_HELLO;
        size_t alertBytes = encrypted ? 2 + association->recordOverhead() : 15;
        auto credit = association->receivedBytes > std::numeric_limits<size_t>::max() / 3 ? std::numeric_limits<size_t>::max() : association->receivedBytes * 3;
        bool permitted = !association->serverSide || association->addressValidated ||
            (association->sentBytes <= credit && alertBytes <= credit - association->sentBytes);
        uint64_t number = 0;
        auto epoch = association->state == HandshakeState::ESTABLISHED || (association->serverSide && association->ownFinishedHandedOff) ? association->sendEpoch : 2;
        bool available = !encrypted || (epoch == 2 ? association->nextEpoch2Record < Association::MAX_PROTECTED_RECORDS : association->allocateApplicationRecord(number));
        if (encrypted && epoch == 2 && available)
            number = association->nextEpoch2Record++;
        if (permitted && available)
            sendAlert(association->localAddress, association->peerAddress, association->peerPort, alertDescription, encrypted, number, epoch,
                      association->cidNegotiated ? association->peerCid : std::vector<uint8_t>{}, association->localInterfaceId);
    }
    auto callback = association->callback;
    if (association->authenticationRequest)
        callback->authenticationResult(id, false, "");
    if (association->earlyRequested && !association->earlyStatusReported)
        callback->earlyDataStatus(id, false);
    for (auto& pending : association->pendingPackets) {
        auto packetId = pending.packet->getId();
        pending.packet.reset();
        callback->sendError(id, packetId, reason);
    }
    for (auto packetId : association->processingPacketIds)
        callback->sendError(id, packetId, reason);
    emit(associationFailedSignal, (long)id);
    callback->error(id, reason);
    if (association->closing)
        callback->closed(id);
    if (operationalState == STOPPING_OPERATION && associations.empty())
        udpSocket.close();
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
    appendUint16(extensions, type);
    appendUint16(extensions, content.size());
    extensions.insert(extensions.end(), content.begin(), content.end());
}

static bool findCookie(const Ptr<const DtlsClientHello>& hello, std::vector<uint8_t>& cookie)
{
    size_t pos = 0;
    size_t end = hello->getExtensionsArraySize();
    while (end - pos >= 4) {
        uint16_t type = (uint16_t(hello->getExtensions(pos)) << 8) | hello->getExtensions(pos + 1);
        uint16_t length = (uint16_t(hello->getExtensions(pos + 2)) << 8) | hello->getExtensions(pos + 3);
        pos += 4;
        if (length > end - pos)
            return false;
        if (type == 44) {
            if (length < 3 || ((uint16_t(hello->getExtensions(pos)) << 8) | hello->getExtensions(pos + 1)) != length - 2)
                return false;
            for (size_t i = pos + 2; i < pos + length; ++i)
                cookie.push_back(hello->getExtensions(i));
            return true;
        }
        pos += length;
    }
    return pos == end;
}

static void appendUint64(std::vector<uint8_t>& output, uint64_t value)
{
    for (int i = 7; i >= 0; --i)
        output.push_back(value >> (i * 8));
}

static std::vector<uint8_t> makeCookie(uint64_t issued, uint64_t started, const L3Address& local, int localPort,
                                       const L3Address& peer, int peerPort, const Ptr<const DtlsClientHello>& hello)
{
    // Opaque modeled token; it is not a cryptographic authenticator.
    std::vector<uint8_t> token;
    appendUint64(token, issued);
    appendUint64(token, started);
    auto localString = local.str();
    auto peerString = peer.str();
    appendUint16(token, localString.size());
    token.insert(token.end(), localString.begin(), localString.end());
    appendUint16(token, localPort);
    appendUint16(token, peerString.size());
    token.insert(token.end(), peerString.begin(), peerString.end());
    appendUint16(token, peerPort);
    for (size_t i = 0; i < 32; ++i)
        token.push_back(hello->getRandom(i));
    return token;
}

static uint64_t readUint64(const std::vector<uint8_t>& input, size_t offset)
{
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i)
        value = (value << 8) | input[offset + i];
    return value;
}

void Dtls::sendClientHello(AssociationId id, const std::vector<uint8_t>& cookie)
{
    auto it = associations.find(id);
    if (it != associations.end())
        it->second->sendClientHello(cookie);
}
void Dtls::sendRetry(const L3Address& localAddress, const L3Address& peerAddress, int peerPort, const std::vector<uint8_t>& cookie, bool requestKeyShare, size_t receivedBytes, PrevalidationAttempt *attempt, uint16_t messageSequence)
{
    // RFC 9147 Section 5.1 uses a cookie retry and bounds traffic before address validation.
    (void)localAddress;
    (void)peerAddress;
    (void)peerPort;
    (void)receivedBytes;
    if (!attempt)
        return;
    auto id = attempt->id;
    if (attempt->retryFlight && attempt->retryFlight->size() &&
        attempt->retryFlight->get(0).messageSequence == messageSequence) {
        if (!attempt->retryGenerating) {
            cancelEvent(attempt->retryTimer.get());
            attempt->retryFlight->requestRetransmission();
            attempt->retryBurst = attempt->retryQueued;
            pumpRetry(id);
        }
        return;
    }
    auto hello = makeShared<tls::TlsServerHello>();
    hello->setLegacyVersion(0xfefd);
    hello->setCipherSuite(attempt->selection.cipherSuite);
    for (size_t i = 0; i < 32; ++i)
        hello->setRandom(i, Association::HELLO_RETRY_RANDOM[i]);
    std::vector<uint8_t> extensions;
    appendExtension(extensions, 43, {0xfe, 0xfc});
    if (requestKeyShare)
        appendExtension(extensions, 51, {uint8_t(attempt->selection.group >> 8), uint8_t(attempt->selection.group)});
    std::vector<uint8_t> cookieExtension;
    appendUint16(cookieExtension, cookie.size());
    cookieExtension.insert(cookieExtension.end(), cookie.begin(), cookie.end());
    appendExtension(extensions, 44, cookieExtension);
    hello->setExtensionsArraySize(extensions.size());
    for (size_t i = 0; i < extensions.size(); ++i)
        hello->setExtensions(i, extensions[i]);
    hello->setChunkLength(B(40 + extensions.size()));
    auto limit = initialPayloadBudget();
    if (limit <= 25) {
        emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
        return;
    }
    cancelEvent(attempt->retryTimer.get());
    auto flight = std::make_shared<DtlsHandshakeFlight>();
    attempt->retryFlight = flight;
    attempt->retryQueued = attempt->retryBurst = 0;
    auto length = hello->getChunkLength().get<B>();
    auto maximum = std::min<size_t>(limit - 25, DtlsRecordProtection::MAX_CONTENT_BYTES - 12);
    if (size_t(length) <= maximum) {
        hello->markImmutable();
        flight->add(2, messageSequence, length, 0, 0, hello);
    }
    else {
        Packet source("DTLS HelloRetryRequest body", hello);
        auto bytes = source.peekDataAsBytes()->getBytes();
        for (size_t offset = 0; offset < bytes.size(); offset += maximum) {
            auto count = std::min(maximum, bytes.size() - offset);
            auto chunk = makeShared<BytesChunk>(std::vector<uint8_t>(bytes.begin() + offset, bytes.begin() + offset + count));
            chunk->markImmutable();
            flight->add(2, messageSequence, length, offset, 0, chunk);
        }
    }
    attempt->retryGenerating = true;
    auto delay = SimTime(par("handshakeOutDelay").doubleValue());
    if (!enqueueProcessing(outputProcessing.get(), id, length + 12, delay, [this, id, flight]() {
        auto current = findPrevalidation(id);
        if (!current || current->retryFlight != flight)
            return;
        for (size_t i = 0; i < flight->size(); ++i)
            flight->markReady(i);
        current->retryGenerating = false;
        pumpRetry(id);
    }, [this, id, flight]() { auto current = findPrevalidation(id); return current && current->retryFlight == flight; })) {
        removePrevalidation(id);
        emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
    }
}

void Dtls::pumpRetry(uint64_t id)
{
    auto attempt = findPrevalidation(id);
    if (!attempt || attempt->failing || attempt->retryPumping || attempt->retryGenerating || !attempt->retryFlight || attempt->retryFlight->isComplete())
        return;
    attempt->retryPumping = true;
    auto flight = attempt->retryFlight;
    for (size_t i = 0; i < flight->size() && attempt->retryBurst < 10; ++i) {
        if (!flight->reserve(i))
            continue;
        auto bytes = size_t(flight->get(i).data->getChunkLength().get<B>()) + 25;
        auto credit = attempt->receivedBytes > std::numeric_limits<size_t>::max() / 3 ? std::numeric_limits<size_t>::max() : attempt->receivedBytes * 3;
        if (attempt->sentBytes > credit || attempt->reservedBytes > credit - attempt->sentBytes || bytes > credit - attempt->sentBytes - attempt->reservedBytes) {
            flight->cancelReservation(i);
            continue;
        }
        attempt->reservedBytes += bytes;
        ++attempt->retryBurst;
        ++attempt->retryQueued;
        auto eligible = [this, id, flight, i]() {
            auto current = findPrevalidation(id);
            return current && current->retryFlight == flight && flight->get(i).queued && !flight->get(i).acknowledged;
        };
        auto skipped = [this, id, flight, i, bytes]() {
            flight->cancelReservation(i);
            auto current = findPrevalidation(id);
            if (!current)
                return;
            current->reservedBytes -= bytes;
            if (current->retryFlight == flight) {
                if (current->retryQueued)
                    --current->retryQueued;
                if (current->retryBurst)
                    --current->retryBurst;
                if (!current->retryPumping)
                    pumpRetry(id);
            }
        };
        auto complete = [this, id, flight, i, bytes]() {
            auto current = findPrevalidation(id);
            if (!current || current->retryFlight != flight)
                return;
            if (current->nextRecord >= (uint64_t(1) << 48)) {
                removePrevalidation(id);
                return;
            }
            auto fragment = flight->get(i);
            auto number = current->nextRecord++;
            auto packet = new Packet("DTLS HelloRetryRequest", fragment.data);
            auto handshake = makeShared<DtlsHandshakeHeader>();
            handshake->setMessageType(fragment.messageType);
            handshake->setMessageSequence(fragment.messageSequence);
            handshake->setMessageLength(fragment.messageLength);
            handshake->setFragmentOffset(fragment.offset);
            handshake->setFragmentLength(fragment.data->getChunkLength().get<B>());
            packet->insertAtFront(handshake);
            auto record = makeShared<DtlsPlaintextHeader>();
            record->setSequenceNumber(number);
            record->setLength(packet->getDataLength().get<B>());
            packet->insertAtFront(record);
            packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
            if (!current->localAddress.isUnspecified())
                packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(current->localAddress);
            auto peer = current->peerAddress;
            auto port = current->peerPort;
            current->reservedBytes -= bytes;
            current->sentBytes += bytes;
            emit(packetSentToLowerSignal, packet);
            sendUdp(packet, peer, port);
            current = findPrevalidation(id);
            if (!current || current->retryFlight != flight)
                return;
            bool retransmission = flight->get(i).transmissions != 0;
            flight->handedOff(i, number, simTime());
            if (current->retryQueued)
                --current->retryQueued;
            if (retransmission)
                emit(handshakeRetransmissionSignal, (long)id);
            current = findPrevalidation(id);
            if (current && current->retryFlight == flight && !current->retryPumping && current->retryQueued == 0 &&
                !flight->isComplete() && !current->retryTimer->isScheduled())
                scheduleAt(simTime() + current->retryTimeout, current->retryTimer.get());
        };
        if (!enqueueProcessing(outputProcessing.get(), id, bytes, SIMTIME_ZERO, complete, eligible, skipped)) {
            removePrevalidation(id);
            emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
            return;
        }
        attempt = findPrevalidation(id);
        if (!attempt || attempt->retryFlight != flight)
            return;
    }
    attempt->retryPumping = false;
    if (attempt->retryQueued == 0 && attempt->retryBurst != 0 && !flight->isComplete() && !attempt->retryTimer->isScheduled())
        scheduleAt(simTime() + attempt->retryTimeout, attempt->retryTimer.get());
}

void Dtls::acknowledgeRetry(uint64_t id, const DtlsAck& ack)
{
    auto attempt = findPrevalidation(id);
    if (!attempt || !attempt->retryFlight || ack.getEpochsArraySize() != ack.getSequenceNumbersArraySize())
        return;
    bool progress = false;
    for (size_t i = 0; i < ack.getEpochsArraySize(); ++i) {
        auto result = attempt->retryFlight->acknowledge(ack.getEpochs(i), ack.getSequenceNumbers(i), simTime());
        progress |= result.matched;
        if (result.rtt)
            attempt->retryTimeout = std::max(SimTime::fromRaw(1), *result.rtt * 1.5);
    }
    if (!progress)
        return;
    cancelEvent(attempt->retryTimer.get());
    if (!attempt->retryFlight->isComplete()) {
        attempt->retryFlight->requestRetransmission();
        attempt->retryBurst = attempt->retryQueued;
        pumpRetry(id);
    }
}

void Dtls::sendPlainHandshake(AssociationId id, uint8_t messageType, uint16_t messageSequence, const Ptr<Chunk>& body)
{
    auto it = associations.find(id);
    if (it != associations.end())
        it->second->appendHandshake(messageType, messageSequence, 0, body);
}

void Dtls::sendProtectedHandshake(AssociationId id, uint8_t messageType, uint16_t messageSequence, const Ptr<Chunk>& body, bool establishAfterSend, bool serverFinished)
{
    auto it = associations.find(id);
    if (it != associations.end())
        it->second->appendHandshake(messageType, messageSequence, 2, body, establishAfterSend, serverFinished);
}
void Dtls::transmitFlightFragment(AssociationId id, const std::shared_ptr<DtlsHandshakeFlight>& flight, size_t index)
{
    auto it = associations.find(id);
    if (it == associations.end() || (it->second->flight != flight && !it->second->findPostHandshakeExchange(flight)))
        return;
    if (it->second->isRebinding() || !it->second->permitsOutput(flight)) {
        if (it->second->keyUpdate().flight == flight)
            it->second->updateRecordSkipped(flight, index);
        else if (it->second->findPostHandshakeExchange(flight))
            flight->cancelReservation(index);
        else
            it->second->flightRecordSkipped(flight, index, 0);
        return;
    }
    bool isPostHandshake = it->second->findPostHandshakeExchange(flight) != nullptr;
    auto fragment = flight->get(index);
    size_t bytes = fragment.data->getChunkLength().get<B>() + (fragment.epoch == 0 ? 25 : 12 + it->second->recordOverhead());
    size_t reservedCredit = it->second->serverSide && !isPostHandshake ? bytes : 0;
    if (it->second->serverSide && !isPostHandshake && fragment.transmissions != 0) {
        if (!it->second->addressValidated) {
            auto received = it->second->receivedBytes;
            auto credit = received > std::numeric_limits<size_t>::max() / 3 ? std::numeric_limits<size_t>::max() : received * 3;
            if (it->second->sentBytes > credit || it->second->reservedBytes > credit - it->second->sentBytes ||
                bytes > credit - it->second->sentBytes - it->second->reservedBytes) {
                flight->cancelReservation(index);
                --it->second->burstQueued;
                --it->second->burstLimit;
                return;
            }
        }
        it->second->reservedBytes += bytes;
    }
    auto eligible = [this, id, flight, index]() {
        auto current = associations.find(id);
        auto exchange = current != associations.end() ? current->second->findPostHandshakeExchange(flight) : nullptr;
        return current != associations.end() && !current->second->closing && !current->second->isRebinding() && current->second->permitsOutput(flight) &&
               (current->second->flight == flight || (exchange && !exchange->recoveryExpired)) &&
               flight->get(index).queued && !flight->get(index).acknowledged && flight->isFirstTransmissionOrdered(index) &&
               (!current->second->pathMtu || flight->get(index).epoch == 0 ||
                flight->get(index).data->getChunkLength() + B(12 + current->second->recordOverhead()) <= B(current->second->udpPayloadBudget()));
    };
    auto skipped = [this, id, flight, index, reservedCredit]() {
        auto current = associations.find(id);
        if (current != associations.end()) {
            if (current->second->keyUpdate().flight == flight)
                current->second->updateRecordSkipped(flight, index);
            else if (current->second->findPostHandshakeExchange(flight)) {
                flight->cancelReservation(index);
                if (current->second->pathMtu && current->second->prepareFlight(flight)) {
                    current = associations.find(id);
                    if (current != associations.end())
                        for (const auto& entry : current->second->postHandshakeExchanges)
                            if (entry.second->flight == flight) {
                                current->second->pumpPostHandshake(entry.first);
                                return;
                            }
                }
            }
            else
                current->second->flightRecordSkipped(flight, index, reservedCredit);
        }
    };
    auto complete = [this, id, flight, index, reservedCredit]() {
        auto current = associations.find(id);
        if (current == associations.end() || (current->second->flight != flight && !current->second->findPostHandshakeExchange(flight)))
            return;
        auto fragment = flight->get(index);
        if (fragment.epoch != 0 && fragment.data->getChunkLength() + B(12 + current->second->recordOverhead()) > B(current->second->udpPayloadBudget())) {
            failAssociation(id, "DTLS CID update exceeds retained handshake record budget");
            return;
        }
        uint64_t recordNumber;
        if (fragment.epoch == 0) {
            if (!current->second->allocatePlaintextRecord(recordNumber)) {
                failAssociation(id, "DTLS plaintext record sequence exhausted");
                return;
            }
        }
        else if (fragment.epoch == 2) {
            if (current->second->nextEpoch2Record >= (uint64_t(1) << 24)) {
                failAssociation(id, "DTLS handshake epoch record limit reached");
                return;
            }
            recordNumber = current->second->nextEpoch2Record++;
        }
        else if (!current->second->allocateRecord(fragment.epoch, recordNumber)) {
            failAssociation(id, "DTLS application epoch record limit reached");
            return;
        }
        auto packet = new Packet("DTLS handshake", fragment.data);
        auto handshake = makeShared<DtlsHandshakeHeader>();
        handshake->setMessageType(fragment.messageType);
        handshake->setMessageSequence(fragment.messageSequence);
        handshake->setMessageLength(fragment.messageLength);
        handshake->setFragmentOffset(fragment.offset);
        handshake->setFragmentLength(fragment.data->getChunkLength().get<B>());
        packet->insertAtFront(handshake);
        if (fragment.epoch == 0) {
            auto header = makeShared<DtlsPlaintextHeader>();
            header->setSequenceNumber(recordNumber);
            header->setLength(packet->getDataLength().get<B>());
            packet->insertAtFront(header);
        }
        else {
            DtlsRecordProtection::protect(packet, 22);
            auto header = current->second->makeRecordHeader(fragment.epoch, recordNumber, packet->getDataLength().get<B>());
            packet->insertAtFront(header);
            emit(recordOverheadSignal, (long)current->second->recordOverhead());
        }
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
        if (!current->second->localAddress.isUnspecified())
            packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(current->second->localAddress);
        if (current->second->localInterfaceId >= 0)
            packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(current->second->localInterfaceId);
        auto peer = current->second->peerAddress;
        auto port = current->second->peerPort;
        if (reservedCredit <= current->second->reservedBytes) {
            current->second->reservedBytes -= reservedCredit;
            current->second->sentBytes += reservedCredit;
        }
        emit(packetSentToLowerSignal, packet);
        sendUdp(packet, peer, port);
        current = associations.find(id);
        if (current != associations.end()) {
            if (current->second->keyUpdate().flight == flight)
                current->second->updateRecordHandedOff(flight, index, recordNumber);
            else if (current->second->findPostHandshakeExchange(flight))
                current->second->postHandshakeRecordHandedOff(flight, index, recordNumber);
            else if (current->second->flight == flight)
                current->second->flightRecordHandedOff(flight, index, recordNumber);
        }
    };
    auto delay = fragment.epoch == 0 ? SIMTIME_ZERO : processingDelay(id, "protectOutDelay");
    auto started = [this, id, flight]() {
        auto it = associations.find(id);
        if (it == associations.end())
            return;
        for (const auto& entry : it->second->postHandshakeExchanges)
            if (entry.first >= Association::AUTHENTICATION_RESPONSE_BASE && entry.second->flight == flight && !entry.second->firstTransmissionComplete) {
                it->second->authenticationOutput = entry.first;
                break;
            }
    };
    if (!enqueueProcessing(outputProcessing.get(), id, bytes, delay, complete, eligible, skipped, started))
        failAssociation(id, "DTLS output processing queue is full");
}

void Dtls::establish(AssociationId id)
{
    auto it = associations.find(id);
    if (it == associations.end())
        return;
    auto established = std::count_if(associations.begin(), associations.end(), [](const auto& entry) { return entry.second->state == HandshakeState::ESTABLISHED; });
    auto limit = par("maxAssociations").intValue();
    auto old = associations.find(it->second->replacementId);
    bool replacesEstablished = old != associations.end() && old->second->state == HandshakeState::ESTABLISHED;
    auto oldId = replacesEstablished ? old->first : 0;
    if (limit <= 0 || established - (replacesEstablished ? 1 : 0) >= limit) {
        failAssociation(id, "DTLS association limit reached", 40);
        return;
    }
    it->second->initialPeerMessageEnd = it->second->nextPeerMessage;
    if (it->second->selection.authenticationMode == tls::TlsHandshakeParameters::AuthenticationMode::TICKET_PSK && it->second->offeredSession) {
        it->second->peerCertificateAuthenticated = it->second->offeredSession->peerCertificateAuthenticated;
        it->second->authenticatedPeerIdentity = it->second->offeredSession->authenticatedPeerIdentity;
    }
    else if (it->second->selection.authenticationMode == tls::TlsHandshakeParameters::AuthenticationMode::CERTIFICATE &&
             (!it->second->serverSide || it->second->clientAuthRequested)) {
        it->second->peerCertificateAuthenticated = true;
        it->second->authenticatedPeerIdentity = par("identity").stdstringValue();
    }
    it->second->state = HandshakeState::ESTABLISHED;
    auto elapsed = simTime() - it->second->startedAt;
    cancelEvent(it->second->handshakeDeadline.get());
    auto hadReassembly = it->second->reassembly.getReservedBytes() != 0;
    it->second->reassembly.clear();
    it->second->completeRecordNumbers.clear();
    if (hadReassembly) {
        emitReassemblyBytes();
        it = associations.find(id);
        if (it == associations.end())
            return;
    }
    // RFC 9147 Section 5.8.1 retains the server's final response for at least twice the default MSL (240 seconds).
    if (it->second->serverSide)
        scheduleAt(simTime() + SimTime(240), it->second->retentionTimer.get());
    if (replacesEstablished) {
        close(oldId);
        it = associations.find(id);
        if (it == associations.end())
            return;
    }
    auto callback = it->second->callback;
    emit(establishmentTimeSignal, elapsed.dbl());
    emit(associationEstablishedSignal, (long)id);
    callback->established(id);
    it = associations.find(id);
    if (it != associations.end()) {
        if (isResumed(id))
            emit(registerSignal("sessionResumed"), (long)id);
        it = associations.find(id);
        if (it != associations.end())
            it->second->issueTicket();
    }
    it = associations.find(id);
    if (it != associations.end())
        it->second->startPathMtuDiscovery();
    resumeApplications(id);
}

bool Dtls::holdApplication(AssociationId id, std::unique_ptr<Packet> packet, std::optional<simtime_t> sampledDelay, bool paid)
{
    auto it = associations.find(id);
    if (it == associations.end())
        return false;
    auto bytes = std::max(size_t(1), (size_t)packet->getByteLength());
    auto limit = par("maxPendingBytes").intValue();
    auto occupied = it->second->pendingBytes + ((par("postHandshakeAuthEnabled").boolValue() || it->second->pathMtu) ? it->second->retainedPostHandshakeBytes() : 0);
    if (it->second->closing || limit <= 0 || bytes > size_t(limit) || occupied > size_t(limit) - bytes) {
        auto callback = it->second->callback;
        auto packetId = packet->getId();
        packet.reset();
        callback->sendError(id, packetId, "DTLS pending application queue is full or closing");
        return false;
    }
    it->second->pendingBytes += bytes;
    it->second->pendingPackets.push_back({std::move(packet), sampledDelay, paid});
    return true;
}

void Dtls::resumeApplications(AssociationId id)
{
    while (true) {
        auto it = associations.find(id);
        if (it == associations.end() || it->second->closing || it->second->state != HandshakeState::ESTABLISHED ||
            it->second->keyUpdate().flight || it->second->peerResponseDue || it->second->isRebinding() || it->second->authenticationOutput || it->second->isPathUnavailable() || it->second->pendingPackets.empty())
            return;
        auto pending = std::move(it->second->pendingPackets.front());
        it->second->pendingPackets.pop_front();
        it->second->pendingBytes -= std::max(size_t(1), (size_t)pending.packet->getByteLength());
        sendApplication(id, pending.packet.release(), pending.delay, pending.paid);
    }
}

void Dtls::sendApplication(AssociationId id, Packet *packet, std::optional<simtime_t> sampledDelay, bool paid)
{
    // RFC 9147 Sections 4.3 and 4.5.3 bound datagrams and key usage; processing delays are modeled here.
    auto it = associations.find(id);
    if (it == associations.end()) {
        delete packet;
        return;
    }
    auto callback = it->second->callback;
    auto packetId = packet->getId();
    auto bytes = packet->getDataLength();
    if (bytes.get<b>() % 8 != 0 || bytes > B(DtlsRecordProtection::MAX_CONTENT_BYTES) || bytes + B(it->second->recordOverhead()) > B(it->second->udpPayloadBudget()) ||
        it->second->closing) {
        delete packet;
        callback->sendError(id, packetId, "DTLS application record size or epoch limit exceeded");
        return;
    }
    if (it->second->keyUpdate().flight || it->second->peerResponseDue || it->second->isRebinding() || it->second->authenticationOutput || it->second->isPathUnavailable()) {
        holdApplication(id, std::unique_ptr<Packet>(packet), sampledDelay, paid);
        return;
    }
    auto threshold = uint64_t(par("rekeyAfterRecords").intValue());
    if (it->second->nextApplicationRecord >= threshold && it->second->sendEpoch < Association::MAX_SEND_EPOCH) {
        if (!holdApplication(id, std::unique_ptr<Packet>(packet), sampledDelay, paid))
            return;
        auto current = associations.find(id);
        if (current != associations.end() && !current->second->keyUpdate().flight && !current->second->requestUpdate(false) && containsAssociation(id))
            failAssociation(id, "DTLS automatic KeyUpdate could not start");
        return;
    }
    auto retained = std::make_shared<std::unique_ptr<Packet>>(packet);
    auto delay = paid ? SIMTIME_ZERO : (sampledDelay ? *sampledDelay : processingDelay(id, "protectOutDelay"));
    auto started = std::make_shared<bool>(false);
    auto overhead = it->second->recordOverhead();
    it->second->processingPacketIds.insert(packetId);
    auto eligible = [this, id, started, overhead, bytes]() {
        auto current = associations.find(id);
        if (current == associations.end() || current->second->closing)
            return false;
        if (current->second->recordOverhead() != overhead)
            return false;
        if (current->second->isRebinding() || current->second->authenticationOutput || current->second->isPathUnavailable() ||
            bytes + B(current->second->recordOverhead()) > B(current->second->udpPayloadBudget()))
            return false;
        return *started || (!current->second->keyUpdate().flight && !current->second->peerResponseDue &&
               (current->second->nextApplicationRecord < uint64_t(par("rekeyAfterRecords").intValue()) || current->second->sendEpoch >= Association::MAX_SEND_EPOCH));
    };
    auto skipped = [this, id, packetId, retained, delay, paid, overhead, started, bytes]() {
        auto current = associations.find(id);
        if (current == associations.end())
            return;
        current->second->processingPacketIds.erase(packetId);
        if (current->second->recordOverhead() != overhead) {
            auto callback = current->second->callback;
            retained->reset();
            callback->sendError(id, packetId, "DTLS header budget changed before output handoff");
            return;
        }
        if (bytes + B(current->second->recordOverhead()) > B(current->second->udpPayloadBudget())) {
            auto callback = current->second->callback;
            retained->reset();
            callback->sendError(id, packetId, "DTLS path payload budget decreased before output handoff");
            return;
        }
        if (current->second->isRebinding() || current->second->authenticationOutput || current->second->isPathUnavailable()) {
            holdApplication(id, std::move(*retained), delay, paid || *started);
            return;
        }
        if (!holdApplication(id, std::move(*retained), delay, paid))
            return;
        current = associations.find(id);
        if (current != associations.end() && !current->second->keyUpdate().flight && !current->second->peerResponseDue &&
            !current->second->requestUpdate(false) && containsAssociation(id))
            failAssociation(id, "DTLS automatic KeyUpdate could not start");
    };
    if (!enqueueProcessing(outputProcessing.get(), id, size_t(bytes.get<B>()) + it->second->recordOverhead(), delay, [this, id, packetId, retained, delay]() {
        auto current = associations.find(id);
        if (current != associations.end()) {
            if (current->second->keyUpdate().flight || current->second->peerResponseDue) {
                current->second->processingPacketIds.erase(packetId);
                holdApplication(id, std::move(*retained), delay, true);
                return;
            }
            uint64_t number;
            if (!current->second->allocateApplicationRecord(number)) {
                retained->reset();
                failAssociation(id, "DTLS application epoch record limit exceeded");
                return;
            }
            current->second->processingPacketIds.erase(packetId);
            auto packet = retained->get();
            DtlsRecordProtection::protect(packet, 23);
            auto header = current->second->makeRecordHeader(current->second->sendEpoch, number, packet->getDataLength().get<B>());
            packet->insertAtFront(header);
            packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
            if (!current->second->localAddress.isUnspecified())
                packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(current->second->localAddress);
            if (current->second->localInterfaceId >= 0)
                packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(current->second->localInterfaceId);
            auto peer = current->second->peerAddress;
            auto port = current->second->peerPort;
            auto outgoing = retained->release();
            emit(recordOverheadSignal, (long)current->second->recordOverhead());
            emit(packetSentSignal, outgoing);
            emit(packetSentToLowerSignal, outgoing);
            sendUdp(outgoing, peer, port);
        }
    }, eligible, skipped, [started]() { *started = true; }, !paid)) {
        auto current = associations.find(id);
        if (current != associations.end())
            current->second->processingPacketIds.erase(packetId);
        retained.reset();
        callback->sendError(id, packetId, "DTLS output processing queue is full");
    }
}

Ptr<Chunk> Dtls::makeCertificate(const std::vector<uint8_t>& context) const
{
    std::string identity = par("identity").stringValue();
    auto bytes = par("certificateBytes").intValue();
    if (identity.empty() || bytes < (int64_t)identity.size() || bytes > 0xffffff - 5)
        throw cRuntimeError("Invalid modeled DTLS certificate size or identity");
    auto certificate = makeShared<tls::TlsCertificate>();
    certificate->setRequestContextArraySize(context.size());
    for (size_t i = 0; i < context.size(); ++i)
        certificate->setRequestContext(i, context[i]);
    certificate->setCertificateListArraySize(bytes + 5);
    certificate->setCertificateList(0, (bytes >> 16) & 0xff);
    certificate->setCertificateList(1, (bytes >> 8) & 0xff);
    certificate->setCertificateList(2, bytes & 0xff);
    for (size_t i = 0; i < identity.size(); ++i)
        certificate->setCertificateList(i + 3, identity[i]);
    certificate->setChunkLength(B(bytes + 9 + context.size()));
    return certificate;
}

Ptr<Chunk> Dtls::makeCertificateVerify(uint16_t scheme) const
{
    auto bytes = scheme == tls::TlsHandshakeParameters::RSA_PSS_RSAE_SHA256 ? 256 : par("signatureBytes").intValue();
    if (bytes <= 0 || bytes > 65535)
        throw cRuntimeError("Invalid modeled DTLS signature size");
    auto verify = makeShared<tls::TlsCertificateVerify>();
    verify->setSignatureScheme(scheme);
    verify->setSignatureArraySize(bytes);
    verify->setChunkLength(B(bytes + 4));
    return verify;
}

int Dtls::checkCertificate(Packet *body, const std::vector<uint8_t>& context) const
{
    Ptr<const tls::TlsCertificate> certificate;
    try {
        certificate = body->popAtFront<tls::TlsCertificate>(body->getDataLength());
    }
    catch (const cRuntimeError&) {
        return 50;
    }
    if (body->getDataLength() != b(0))
        return 50;
    auto size = certificate->getCertificateListArraySize();
    if (certificate->getRequestContextArraySize() != context.size())
        return 47;
    for (size_t i = 0; i < context.size(); ++i)
        if (certificate->getRequestContext(i) != context[i])
            return 47;
    if (size == 0 && !context.empty())
        return 116; // The selected PHA policy requires a certificate.
    if (size < 6)
        return 50;
    auto bytes = (uint32_t(certificate->getCertificateList(0)) << 16) |
                 (uint32_t(certificate->getCertificateList(1)) << 8) | certificate->getCertificateList(2);
    std::string identity = par("identity").stringValue();
    if (bytes + 5 != size)
        return 50;
    if (certificate->getCertificateList(bytes + 3) != 0 || certificate->getCertificateList(bytes + 4) != 0)
        return 47;
    if (identity.empty() || bytes < identity.size())
        return 42;
    for (size_t i = 0; i < bytes; ++i)
        if (certificate->getCertificateList(i + 3) != (i < identity.size() ? uint8_t(identity[i]) : 0))
            return 42;
    return 0;
}

int Dtls::checkCertificateVerify(Packet *body, uint16_t& selectedScheme) const
{
    Ptr<const tls::TlsCertificateVerify> verify;
    try {
        verify = body->popAtFront<tls::TlsCertificateVerify>(body->getDataLength());
    }
    catch (const cRuntimeError&) {
        return 50;
    }
    if (body->getDataLength() != b(0) || verify->getSignatureArraySize() == 0)
        return 50;
    auto scheme = verify->getSignatureScheme();
    if (std::find(preferences.signatures.begin(), preferences.signatures.end(), scheme) == preferences.signatures.end() ||
        (scheme == tls::TlsHandshakeParameters::RSA_PSS_RSAE_SHA256 && verify->getSignatureArraySize() != 256))
        return 47;
    selectedScheme = scheme;
    return 0;
}

void Dtls::sendServerFlight(AssociationId id)
{
    auto it = associations.find(id);
    if (it != associations.end())
        it->second->sendServerFlight();
}
void Dtls::sendFinalAck(AssociationId id, uint64_t sequence)
{
    auto it = associations.find(id);
    if (it == associations.end())
        return;
    it->second->pendingAcks.emplace(2, sequence);
    it->second->flushAcknowledgements(it->second->sendEpoch);
}

void Dtls::sendAcknowledgements(AssociationId id, uint64_t sendEpoch, const std::vector<std::pair<uint64_t, uint64_t>>& records)
{
    // RFC 9147 Sections 7 and 7.1 identify acknowledged records by epoch and sequence number.
    auto association = associations.find(id);
    if (association == associations.end())
        return;
    auto udpLimit = association->second->udpPayloadBudget();
    if (association->second->isRebinding() || association->second->authenticationOutput) {
        association->second->pendingAcks.insert(records.begin(), records.end());
        return;
    }
    int overhead = sendEpoch == 0 ? 13 : association->second->recordOverhead();
    if (udpLimit < overhead + 18) {
        failAssociation(id, "DTLS UDP payload limit cannot carry an ACK");
        return;
    }
    size_t maxRecords = std::min<size_t>((DtlsRecordProtection::MAX_CONTENT_BYTES - 2) / 16, (udpLimit - overhead - 2) / 16);
    for (size_t begin = 0; begin < records.size(); begin += maxRecords) {
        auto current = associations.find(id);
        if (current == associations.end())
            return;
        auto count = std::min(maxRecords, records.size() - begin);
        auto ack = makeShared<DtlsAck>();
        ack->setEpochsArraySize(count);
        ack->setSequenceNumbersArraySize(count);
        for (size_t i = 0; i < count; ++i) {
            ack->setEpochs(i, records[begin + i].first);
            ack->setSequenceNumbers(i, records[begin + i].second);
        }
        ack->setChunkLength(B(2 + 16 * count));
        auto delay = sendEpoch == 0 ? SIMTIME_ZERO : processingDelay(id, "protectOutDelay");
        size_t bytes = overhead + 2 + 16 * count;
        size_t reservedCredit = current->second->serverSide && !current->second->addressValidated ? bytes : 0;
        if (reservedCredit) {
            auto received = current->second->receivedBytes;
            auto credit = received > std::numeric_limits<size_t>::max() / 3 ? std::numeric_limits<size_t>::max() : received * 3;
            if (current->second->sentBytes > credit || current->second->reservedBytes > credit - current->second->sentBytes ||
                reservedCredit > credit - current->second->sentBytes - current->second->reservedBytes)
                continue;
            current->second->reservedBytes += reservedCredit;
        }
        auto eligible = [this, id, ack, sendEpoch]() {
            auto it = associations.find(id);
            return it != associations.end() && !it->second->closing && !it->second->isRebinding() && !it->second->authenticationOutput &&
                   (!it->second->pathMtu || sendEpoch == 0 || ack->getChunkLength() + B(it->second->recordOverhead()) <= B(it->second->udpPayloadBudget()));
        };
        auto skipped = [this, id, ack, reservedCredit]() {
            auto it = associations.find(id);
            if (it == associations.end())
                return;
            if (reservedCredit <= it->second->reservedBytes)
                it->second->reservedBytes -= reservedCredit;
            if (!it->second->closing)
                for (size_t i = 0; i < ack->getEpochsArraySize(); ++i)
                    it->second->pendingAcks.emplace(ack->getEpochs(i), ack->getSequenceNumbers(i));
            if (it->second->pathMtu && !it->second->closing && !it->second->isRebinding() && !it->second->authenticationOutput)
                it->second->flushAcknowledgements(it->second->sendEpoch);
        };
        if (!enqueueProcessing(outputProcessing.get(), id, bytes, delay, [this, id, sendEpoch, ack, reservedCredit]() {
            auto it = associations.find(id);
            if (it == associations.end())
                return;
            auto actualEpoch = sendEpoch >= 3 ? it->second->sendEpoch : sendEpoch;
            if (actualEpoch != 0 && ack->getChunkLength() + B(it->second->recordOverhead()) > B(it->second->udpPayloadBudget())) {
                failAssociation(id, "DTLS CID update exceeds retained ACK budget");
                return;
            }
            uint64_t number;
            if (actualEpoch == 0) {
                if (!it->second->allocatePlaintextRecord(number)) {
                    failAssociation(id, "DTLS plaintext record sequence exhausted");
                    return;
                }
            }
            else if (actualEpoch == 2) {
                if (it->second->nextEpoch2Record >= (uint64_t(1) << 24)) {
                    failAssociation(id, "DTLS handshake epoch record limit reached");
                    return;
                }
                number = it->second->nextEpoch2Record++;
            }
            else {
                if (!it->second->allocateApplicationRecord(number)) {
                    failAssociation(id, "DTLS application epoch record limit reached");
                    return;
                }
            }
            auto packet = new Packet("DTLS ACK", ack);
            if (actualEpoch == 0) {
                auto header = makeShared<DtlsPlaintextHeader>();
                header->setContentType(26);
                header->setSequenceNumber(number);
                header->setLength(packet->getDataLength().get<B>());
                packet->insertAtFront(header);
            }
            else {
                DtlsRecordProtection::protect(packet, 26);
                auto header = it->second->makeRecordHeader(actualEpoch, number, packet->getDataLength().get<B>());
                packet->insertAtFront(header);
                emit(recordOverheadSignal, (long)it->second->recordOverhead());
            }
            packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
            if (!it->second->localAddress.isUnspecified())
                packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(it->second->localAddress);
            if (it->second->localInterfaceId >= 0)
                packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(it->second->localInterfaceId);
            auto peer = it->second->peerAddress;
            auto port = it->second->peerPort;
            if (reservedCredit) {
                it->second->reservedBytes -= reservedCredit;
                it->second->sentBytes += reservedCredit;
            }
            emit(packetSentToLowerSignal, packet);
            sendUdp(packet, peer, port);
        }, eligible, skipped)) {
            auto it = associations.find(id);
            if (it != associations.end())
                it->second->reservedBytes -= reservedCredit;
            failAssociation(id, "DTLS output processing queue is full");
            return;
        }
    }
}

void Dtls::acknowledgePrevalidation(PrevalidationAttempt *attempt, uint64_t recordSequence)
{
    for (const auto& entry : associations) {
        auto& association = *entry.second;
        if (association.serverSide && association.state != HandshakeState::ESTABLISHED &&
            association.localAddress == attempt->localAddress && association.localPort == attempt->localPort &&
            association.peerAddress == attempt->peerAddress && association.peerPort == attempt->peerPort) {
            auto received = attempt->receivedBytes;
            association.receivedBytes = received > std::numeric_limits<size_t>::max() - association.receivedBytes ?
                std::numeric_limits<size_t>::max() : association.receivedBytes + received;
            attempt->receivedBytes = 0;
            sendAcknowledgements(entry.first, 0, {{0, recordSequence}});
            return;
        }
    }
    constexpr size_t BYTES = 31; // plaintext header, ACK vector length, and one record number
    auto credit = attempt->receivedBytes > std::numeric_limits<size_t>::max() / 3 ? std::numeric_limits<size_t>::max() : attempt->receivedBytes * 3;
    if (initialPayloadBudget() < (int)BYTES || attempt->sentBytes > credit ||
        attempt->reservedBytes > credit - attempt->sentBytes || BYTES > credit - attempt->sentBytes - attempt->reservedBytes ||
        attempt->nextRecord >= (uint64_t(1) << 48))
        return;
    auto attemptId = attempt->id;
    auto number = attempt->nextRecord++;
    auto peer = attempt->peerAddress;
    auto port = attempt->peerPort;
    auto local = attempt->localAddress;
    attempt->reservedBytes += BYTES;
    auto findAttempt = [this, attemptId]() -> PrevalidationAttempt * {
        for (auto& candidate : prevalidationAttempts)
            if (candidate->id == attemptId)
                return candidate.get();
        return nullptr;
    };
    if (!enqueueProcessing(outputProcessing.get(), attemptId, BYTES, SIMTIME_ZERO, [this, findAttempt, number, recordSequence, peer, port, local]() {
        auto current = findAttempt();
        if (!current)
            return;
        auto ack = makeShared<DtlsAck>();
        ack->setEpochsArraySize(1);
        ack->setSequenceNumbersArraySize(1);
        ack->setEpochs(0, 0);
        ack->setSequenceNumbers(0, recordSequence);
        ack->setChunkLength(B(18));
        auto packet = new Packet("DTLS ACK", ack);
        auto header = makeShared<DtlsPlaintextHeader>();
        header->setContentType(26);
        header->setSequenceNumber(number);
        header->setLength(18);
        packet->insertAtFront(header);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
        if (!local.isUnspecified())
            packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(local);
        current->reservedBytes -= BYTES;
        current->sentBytes += BYTES;
        emit(packetSentToLowerSignal, packet);
        sendUdp(packet, peer, port);
    }, [findAttempt]() { return findAttempt() != nullptr; })) {
        if (auto current = findAttempt())
            current->reservedBytes -= BYTES;
        emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
    }
}

void Dtls::failPrevalidation(PrevalidationAttempt *attempt, uint8_t alertDescription)
{
    constexpr size_t BYTES = 15;
    auto id = attempt->id;
    for (const auto& entry : associations) {
        auto& association = *entry.second;
        if (association.serverSide && association.state != HandshakeState::ESTABLISHED &&
            association.localAddress == attempt->localAddress && association.localPort == attempt->localPort &&
            association.peerAddress == attempt->peerAddress && association.peerPort == attempt->peerPort) {
            auto candidateId = entry.first;
            auto received = attempt->receivedBytes;
            association.receivedBytes = received > std::numeric_limits<size_t>::max() - association.receivedBytes ?
                std::numeric_limits<size_t>::max() : association.receivedBytes + received;
            removePrevalidation(id);
            failAssociation(candidateId, "conflicting fragmented ClientHello", alertDescription);
            return;
        }
    }
    attempt->failing = true;
    auto hadReassembly = attempt->reassembly.getReservedBytes() != 0;
    attempt->reassembly.clear();
    if (hadReassembly) {
        emitReassemblyBytes();
        auto current = std::find_if(prevalidationAttempts.begin(), prevalidationAttempts.end(),
            [id](const auto& entry) { return entry->id == id; });
        if (current == prevalidationAttempts.end())
            return;
        attempt = current->get();
    }
    inputProcessing->cancel(id);
    outputProcessing->cancel(id);
    auto current = std::find_if(prevalidationAttempts.begin(), prevalidationAttempts.end(),
        [id](const auto& entry) { return entry->id == id; });
    if (current == prevalidationAttempts.end())
        return;
    attempt = current->get();
    attempt->reservedBytes = 0;
    auto credit = attempt->receivedBytes > std::numeric_limits<size_t>::max() / 3 ? std::numeric_limits<size_t>::max() : attempt->receivedBytes * 3;
    if (initialPayloadBudget() < (int)BYTES || attempt->sentBytes > credit ||
        attempt->reservedBytes > credit - attempt->sentBytes || BYTES > credit - attempt->sentBytes - attempt->reservedBytes ||
        attempt->nextRecord >= (uint64_t(1) << 48)) {
        removePrevalidation(id);
        return;
    }
    attempt->reservedBytes += BYTES;
    auto number = attempt->nextRecord++;
    auto peer = attempt->peerAddress;
    auto port = attempt->peerPort;
    auto local = attempt->localAddress;
    auto findAttempt = [this, id]() -> PrevalidationAttempt * {
        for (auto& candidate : prevalidationAttempts)
            if (candidate->id == id)
                return candidate.get();
        return nullptr;
    };
    if (!enqueueProcessing(outputProcessing.get(), id, BYTES, SIMTIME_ZERO, [this, id, findAttempt, number, peer, port, local, alertDescription]() {
        auto current = findAttempt();
        if (!current)
            return;
        auto alert = makeShared<DtlsAlert>();
        alert->setLevel(2);
        alert->setDescription(alertDescription);
        auto packet = new Packet("DTLS fatal alert", alert);
        auto header = makeShared<DtlsPlaintextHeader>();
        header->setContentType(21);
        header->setSequenceNumber(number);
        header->setLength(2);
        packet->insertAtFront(header);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
        if (!local.isUnspecified())
            packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(local);
        current->reservedBytes -= BYTES;
        current->sentBytes += BYTES;
        emit(packetSentToLowerSignal, packet);
        sendUdp(packet, peer, port);
        removePrevalidation(id);
    }, [findAttempt]() { return findAttempt() != nullptr; }))
        removePrevalidation(id);
}

void Dtls::handlePlainHandshakeRecord(Packet *body, const L3Address& localAddress, int localPort, const L3Address& peerAddress, int peerPort, size_t datagramBytes, const std::shared_ptr<bool>& creditCounted, uint64_t recordSequence)
{
    std::vector<std::unique_ptr<Packet>> fragments;
    uint8_t type = 0;
    uint16_t sequence = 0;
    while (body->getDataLength() > b(0)) {
        if (body->getDataLength() < B(12)) {
            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            return;
        }
        auto header = body->popAtFront<DtlsHandshakeHeader>(B(12));
        auto length = header->getFragmentLength();
        if (length == 0 || header->getFragmentOffset() > header->getMessageLength() ||
            length > header->getMessageLength() - header->getFragmentOffset() || body->getDataLength() < B(length) ||
            (!fragments.empty() && (type != header->getMessageType() || sequence != header->getMessageSequence()))) {
            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            return;
        }
        if (fragments.empty()) {
            type = header->getMessageType();
            sequence = header->getMessageSequence();
        }
        auto fragment = std::make_unique<Packet>("DTLS plaintext handshake fragment", body->removeAtFront(B(length)));
        fragment->insertAtFront(header);
        fragments.push_back(std::move(fragment));
    }
    if (fragments.empty())
        return;
    if (fragments.size() == 1) {
        handlePlainHandshake(fragments.front().get(), localAddress, localPort, peerAddress, peerPort, datagramBytes, creditCounted, recordSequence);
        return;
    }
    if (type == 1 && (!listener || sequence > 1))
        return;
    auto limit = par("maxReassemblyBytes").intValue();
    if (limit < 0)
        return;
    DtlsHandshakeReassembly probe(limit);
    PrevalidationAttempt *attempt = nullptr;
    AssociationId associationId = 0;
    if (type == 1) {
        for (auto& candidate : prevalidationAttempts)
            if (candidate->localAddress == localAddress && candidate->localPort == localPort &&
                candidate->peerAddress == peerAddress && candidate->peerPort == peerPort) {
                attempt = candidate.get();
                probe = candidate->reassembly;
                break;
            }
    }
    else if (type == 2) {
        for (const auto& entry : associations) {
            auto& candidate = *entry.second;
            if (candidate.state == HandshakeState::CLIENT_WAIT_SERVER_HELLO && candidate.peerAddress == peerAddress && candidate.peerPort == peerPort &&
                (candidate.localAddress.isUnspecified() || candidate.localAddress == localAddress) &&
                (candidate.localPort < 0 || candidate.localPort == localPort)) {
                if (sequence != candidate.serverMessageBase && !(candidate.retried && sequence == 0))
                    return;
                associationId = entry.first;
                probe = candidate.reassembly;
                break;
            }
        }
        if (associationId == 0)
            return;
    }
    else
        return;
    size_t lastNeeded = fragments.size() - 1;
    bool foundComplete = false;
    for (size_t i = 0; i < fragments.size(); ++i) {
        const auto& fragment = fragments[i];
        Packet copy(*fragment);
        auto header = copy.popAtFront<DtlsHandshakeHeader>(B(12));
        if (!hasConsistentEncoding(&copy)) {
            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            return;
        }
        auto result = probe.insert(type, sequence, header->getMessageLength(), header->getFragmentOffset(), copy.peekDataAsBytes()->getBytes());
        if (result == DtlsHandshakeReassembly::Result::COMPLETE && !foundComplete) {
            foundComplete = true;
            lastNeeded = i;
        }
        if (result == DtlsHandshakeReassembly::Result::CONFLICT || result == DtlsHandshakeReassembly::Result::LIMIT) {
            if (attempt) {
                if (!*creditCounted) {
                    attempt->receivedBytes = datagramBytes > std::numeric_limits<size_t>::max() - attempt->receivedBytes ?
                        std::numeric_limits<size_t>::max() : attempt->receivedBytes + datagramBytes;
                    *creditCounted = true;
                }
                if (result == DtlsHandshakeReassembly::Result::CONFLICT)
                    failPrevalidation(attempt, 47);
                else
                    removePrevalidation(attempt->id);
            }
            else if (associationId != 0)
                failAssociation(associationId, "invalid or excessive fragmented ServerHello", result == DtlsHandshakeReassembly::Result::CONFLICT ? 47 : -1);
            else if (result == DtlsHandshakeReassembly::Result::CONFLICT)
                sendAlert(localAddress, peerAddress, peerPort, 47, false);
            emit(recordDroppedSignal, (long)(result == DtlsHandshakeReassembly::Result::LIMIT ? RESOURCE_LIMIT : MALFORMED_RECORD));
            return;
        }
    }
    auto firstHeader = fragments.front()->peekAtFront<DtlsHandshakeHeader>(B(12));
    bool recordChargePending = firstHeader->getFragmentOffset() != 0 || firstHeader->getFragmentLength() != firstHeader->getMessageLength();
    for (size_t i = 0; i <= lastNeeded; ++i) {
        handlePlainHandshake(fragments[i].get(), localAddress, localPort, peerAddress, peerPort, datagramBytes, creditCounted, recordSequence, i == lastNeeded, recordChargePending);
        if (associationId != 0) {
            auto current = associations.find(associationId);
            if (current == associations.end() || current->second->state != HandshakeState::CLIENT_WAIT_SERVER_HELLO)
                break;
        }
        else if (type == 1 && i != lastNeeded) {
            auto current = std::find_if(prevalidationAttempts.begin(), prevalidationAttempts.end(), [&](const auto& candidate) {
                return candidate->localAddress == localAddress && candidate->localPort == localPort && candidate->peerAddress == peerAddress && candidate->peerPort == peerPort;
            });
            if (current == prevalidationAttempts.end())
                break;
        }
    }
}

void Dtls::handlePlainHandshake(Packet *body, const L3Address& localAddress, int localPort, const L3Address& peerAddress, int peerPort, size_t datagramBytes, const std::shared_ptr<bool>& creditCounted, uint64_t recordSequence, bool acknowledgePartial, bool recordChargePending)
{
    using Parameters = tls::TlsHandshakeParameters;
    if (body->getDataLength() < B(12) || peerAddress.isUnspecified() || peerPort < 0)
        return;
    auto handshake = body->popAtFront<DtlsHandshakeHeader>(B(12));
    if (handshake->getFragmentOffset() > handshake->getMessageLength() ||
        handshake->getFragmentLength() > handshake->getMessageLength() - handshake->getFragmentOffset() ||
        body->getDataLength() != B(handshake->getFragmentLength()) || handshake->getFragmentLength() == 0)
        return;
    PrevalidationAttempt *attempt = nullptr;
    if (handshake->getMessageType() == 1 && listener && handshake->getMessageSequence() <= 1) {
        for (auto& candidate : prevalidationAttempts)
            if (candidate->localAddress == localAddress && candidate->localPort == localPort &&
                candidate->peerAddress == peerAddress && candidate->peerPort == peerPort) {
                attempt = candidate.get();
                break;
            }
        if (attempt && attempt->failing)
            return;
        bool fragmented = handshake->getFragmentOffset() != 0 || handshake->getFragmentLength() != handshake->getMessageLength();
        if (!attempt && fragmented) {
            auto maxHalfOpen = par("maxHalfOpen").intValue();
            auto limit = par("maxReassemblyBytes").intValue();
            auto timeout = SimTime(par("handshakeTimeout").doubleValue());
            if (maxHalfOpen <= 0 || countHalfOpen() >= size_t(maxHalfOpen) || limit < 0 || timeout <= SIMTIME_ZERO || nextAssociationId == 0) {
                emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
                return;
            }
            auto created = std::make_unique<PrevalidationAttempt>(limit);
            created->id = nextAssociationId++;
            created->localAddress = localAddress;
            created->localPort = localPort;
            created->peerAddress = peerAddress;
            created->peerPort = peerPort;
            scheduleAt(simTime() + timeout, created->deadline.get());
            attempt = created.get();
            prevalidationAttempts.push_back(std::move(created));
        }
        if (attempt && !*creditCounted) {
            attempt->receivedBytes = datagramBytes > std::numeric_limits<size_t>::max() - attempt->receivedBytes ?
                std::numeric_limits<size_t>::max() : attempt->receivedBytes + datagramBytes;
            *creditCounted = true;
        }
        if (fragmented || (attempt && attempt->reassembly.hasEntry(handshake->getMessageSequence()))) {
            if (!hasConsistentEncoding(body))
                return;
            auto result = attempt->reassembly.insert(1, handshake->getMessageSequence(), handshake->getMessageLength(),
                handshake->getFragmentOffset(), body->peekDataAsBytes()->getBytes());
            auto attemptId = attempt->id;
            emitReassemblyBytes();
            auto currentAttempt = std::find_if(prevalidationAttempts.begin(), prevalidationAttempts.end(),
                [attemptId](const auto& entry) { return entry->id == attemptId; });
            if (currentAttempt == prevalidationAttempts.end())
                return;
            attempt = currentAttempt->get();
            if ((result == DtlsHandshakeReassembly::Result::INCOMPLETE || result == DtlsHandshakeReassembly::Result::COMPLETE) &&
                handshake->getMessageSequence() == 1 && attempt->retryFlight) {
                auto acknowledged = attempt->retryFlight->acknowledgeSent(simTime());
                if (acknowledged.rtt)
                    attempt->retryTimeout = std::max(SimTime::fromRaw(1), *acknowledged.rtt * 1.5);
                if (acknowledged.flightComplete)
                    cancelEvent(attempt->retryTimer.get());
            }
            if (result != DtlsHandshakeReassembly::Result::COMPLETE) {
                if (result == DtlsHandshakeReassembly::Result::INCOMPLETE && acknowledgePartial)
                    acknowledgePrevalidation(attempt, recordSequence);
                else if (result == DtlsHandshakeReassembly::Result::CONFLICT || result == DtlsHandshakeReassembly::Result::LIMIT) {
                    if (result == DtlsHandshakeReassembly::Result::CONFLICT)
                        failPrevalidation(attempt, 47);
                    else
                        removePrevalidation(attemptId);
                    emit(recordDroppedSignal, (long)(result == DtlsHandshakeReassembly::Result::LIMIT ? RESOURCE_LIMIT : MALFORMED_RECORD));
                }
                return;
            }
            auto complete = attempt->reassembly.take(handshake->getMessageSequence());
            emitReassemblyBytes();
            currentAttempt = std::find_if(prevalidationAttempts.begin(), prevalidationAttempts.end(),
                [attemptId](const auto& entry) { return entry->id == attemptId; });
            if (currentAttempt == prevalidationAttempts.end())
                return;
            auto assembled = std::make_shared<std::unique_ptr<Packet>>(new Packet("reassembled DTLS ClientHello", makeShared<BytesChunk>(complete.body)));
            auto fullHeader = makeShared<DtlsHandshakeHeader>();
            fullHeader->setMessageType(1);
            fullHeader->setMessageSequence(handshake->getMessageSequence());
            fullHeader->setMessageLength(complete.body.size());
            fullHeader->setFragmentLength(complete.body.size());
            (*assembled)->insertAtFront(fullHeader);
            auto eligible = [this, attemptId]() {
                return std::any_of(prevalidationAttempts.begin(), prevalidationAttempts.end(),
                    [attemptId](const auto& entry) { return entry->id == attemptId; });
            };
            bool duplicateInitial = handshake->getMessageSequence() == 0 &&
                ((!attempt->initialHello.empty() && attempt->initialHello == complete.body) ||
                 (!attempt->pendingInitialHello.empty() && attempt->pendingInitialHello == complete.body));
            if (handshake->getMessageSequence() == 0 && attempt->pendingInitialHello.empty())
                attempt->pendingInitialHello = complete.body;
            auto delay = (fragmented || recordChargePending) && !duplicateInitial ? SimTime(par("handshakeInDelay").doubleValue()) : SIMTIME_ZERO;
            attempt->processingHello = true;
            if (!enqueueProcessing(inputProcessing.get(), attemptId, complete.body.size() + 12, delay,
                [this, assembled, localAddress, localPort, peerAddress, peerPort, datagramBytes, creditCounted, recordSequence, attemptId]() {
                    auto current = findPrevalidation(attemptId);
                    if (!current)
                        return;
                    current->processingHello = false;
                    current->pendingInitialHello.clear();
                    try {
                        handlePlainHandshake(assembled->get(), localAddress, localPort, peerAddress, peerPort, datagramBytes, creditCounted, recordSequence);
                    }
                    catch (const cRuntimeError&) {
                        emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                    }
                }, eligible)) {
                currentAttempt = std::find_if(prevalidationAttempts.begin(), prevalidationAttempts.end(),
                    [attemptId](const auto& entry) { return entry->id == attemptId; });
                if (currentAttempt != prevalidationAttempts.end()) {
                    removePrevalidation(attemptId);
                }
                emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
            }
            return;
        }
        auto profile = std::string(par("profile").stringValue());
        if (profile != "psk" && profile != "certificate")
            return;
        bool certificateProfile = profile == "certificate";
        AssociationId replacementId = 0;
        for (const auto& entry : associations)
            if (entry.second->peerAddress == peerAddress && entry.second->peerPort == peerPort &&
                entry.second->localAddress == localAddress && entry.second->localPort == localPort) {
                if (entry.second->state == HandshakeState::ESTABLISHED) {
                    replacementId = entry.first;
                    continue;
                }
                auto candidateId = entry.first;
                if (attempt) {
                    auto credit = attempt->receivedBytes;
                    entry.second->receivedBytes = credit > std::numeric_limits<size_t>::max() - entry.second->receivedBytes ?
                        std::numeric_limits<size_t>::max() : entry.second->receivedBytes + credit;
                    removePrevalidation(attempt->id);
                    attempt = nullptr;
                }
                auto candidate = associations.find(candidateId);
                if (candidate == associations.end())
                    return;
                if (candidate->second->state == HandshakeState::SERVER_WAIT_CLIENT_FINISHED ||
                    candidate->second->state == HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE) {
                    if (!*creditCounted) {
                        candidate->second->receivedBytes = datagramBytes > std::numeric_limits<size_t>::max() - candidate->second->receivedBytes ?
                            std::numeric_limits<size_t>::max() : candidate->second->receivedBytes + datagramBytes;
                        *creditCounted = true;
                    }
                    if (!candidate->second->serverFlightQueued)
                        sendServerFlight(candidateId);
                    else
                        candidate->second->pumpFlight();
                }
                return;
            }
        auto failHello = [&](uint8_t description) {
            if (attempt)
                failPrevalidation(attempt, description);
            else
                sendAlert(localAddress, peerAddress, peerPort, description, false);
        };
        if (!hasConsistentEncoding(body)) {
            failHello(50);
            return;
        }
        auto encodedHello = body->peekDataAsBytes()->getBytes();
        Ptr<const DtlsClientHello> hello;
        try {
            hello = body->popAtFront<DtlsClientHello>(body->getDataLength());
        }
        catch (const cRuntimeError&) {
            failHello(50);
            return;
        }
        if (body->getDataLength() != b(0)) {
            failHello(50);
            return;
        }
        Parameters::Offer offer;
        Parameters::Selection selection;
        auto identity = std::string(par("identity").stringValue());
        auto alert = Association::validateClientHello(hello, certificateProfile, identity, offer, selection,
            par("resumptionEnabled").boolValue() ? &sessions : nullptr, preferences);
        if (alert != 0) {
            failHello(alert);
            return;
        }
        std::vector<uint8_t> cookie;
        if (!findCookie(hello, cookie)) {
            failHello(50);
            return;
        }
        auto startedAt = attempt ? attempt->deadline->getArrivalTime() - SimTime(par("handshakeTimeout").doubleValue()) : simTime();
        auto issueRetry = [&](bool repeatedInitial) {
            if (!attempt) {
                auto maxHalfOpen = par("maxHalfOpen").intValue();
                auto limit = par("maxReassemblyBytes").intValue();
                auto timeout = SimTime(par("handshakeTimeout").doubleValue());
                if (maxHalfOpen <= 0 || countHalfOpen() >= size_t(maxHalfOpen) || limit < 0 || timeout <= SIMTIME_ZERO || nextAssociationId == 0)
                    return;
                auto created = std::make_unique<PrevalidationAttempt>(limit);
                created->id = nextAssociationId++;
                created->localAddress = localAddress;
                created->localPort = localPort;
                created->peerAddress = peerAddress;
                created->peerPort = peerPort;
                created->receivedBytes = *creditCounted ? 0 : datagramBytes;
                scheduleAt(simTime() + timeout, created->deadline.get());
                attempt = created.get();
                prevalidationAttempts.push_back(std::move(created));
                *creditCounted = true;
            }
            auto issued = uint64_t(simTime().raw());
            attempt->selection = selection;
            auto started = uint64_t((attempt->deadline->getArrivalTime() - SimTime(par("handshakeTimeout").doubleValue())).raw());
            auto token = makeCookie(issued, started, localAddress, localPort, peerAddress, peerPort, hello);
            if (repeatedInitial && !attempt->cookie.empty()) {
                auto originalIssued = readUint64(attempt->cookie, 0);
                auto originalStart = readUint64(attempt->cookie, 8);
                if (attempt->initialHello != encodedHello || attempt->cookie != makeCookie(originalIssued, originalStart, localAddress, localPort, peerAddress, peerPort, hello))
                    return;
                token = attempt->cookie;
            }
            else if (repeatedInitial) {
                attempt->cookie = token;
                attempt->initialHello = encodedHello;
                attempt->requestKeyShare = selection.retryKeyShare;
            }
            sendRetry(localAddress, peerAddress, peerPort, token, repeatedInitial ? attempt->requestKeyShare : selection.retryKeyShare,
                      attempt->receivedBytes, attempt, handshake->getMessageSequence());
        };
        if (handshake->getMessageSequence() == 0) {
            if (!cookie.empty())
                return;
            if (par("cookieRequired").boolValue() || selection.retryKeyShare || replacementId != 0) {
                issueRetry(true);
                return;
            }
        }
        else {
            auto lifetime = SimTime(par("cookieLifetime").doubleValue()).raw();
            if (attempt && attempt->retryFlight && (selection.cipherSuite != attempt->selection.cipherSuite ||
                (attempt->requestKeyShare && (selection.group != attempt->selection.group || offer.keyShares.size() != 1 || offer.keyShares.front() != attempt->selection.group)))) {
                failHello(47);
                return;
            }
            if (cookie.empty()) {
                issueRetry(false);
                return;
            }
            if (cookie.size() < 16 || lifetime <= 0 || selection.retryKeyShare) {
                failHello(47);
                return;
            }
            auto issued = readUint64(cookie, 0);
            auto started = readUint64(cookie, 8);
            auto now = uint64_t(simTime().raw());
            auto handshakeLifetime = SimTime(par("handshakeTimeout").doubleValue()).raw();
            if (started > issued || issued > now || now - issued > uint64_t(lifetime) || handshakeLifetime <= 0 || now - started >= uint64_t(handshakeLifetime) ||
                cookie != makeCookie(issued, started, localAddress, localPort, peerAddress, peerPort, hello)) {
                failHello(47);
                return;
            }
            startedAt = SimTime::fromRaw(started);
        }
        auto maxHalfOpen = par("maxHalfOpen").intValue();
        auto replayWindow = par("replayWindowSize").intValue();
        auto timeout = SimTime(par("handshakeTimeout").doubleValue());
        auto deadline = attempt ? std::min(attempt->deadline->getArrivalTime(), startedAt + timeout) : startedAt + timeout;
        if (maxHalfOpen <= 0 || countHalfOpen() - (attempt ? 1 : 0) >= size_t(maxHalfOpen) || replayWindow <= 0 || replayWindow > 65536 ||
            timeout <= SIMTIME_ZERO || deadline <= simTime() || nextAssociationId == 0) {
            emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
            return;
        }
        auto reassemblyLimit = par("maxReassemblyBytes").intValue();
        if (reassemblyLimit < 0)
            throw cRuntimeError("Invalid DTLS reassembly limit");
        auto association = std::make_unique<Association>(*this, reassemblyLimit);
        association->id = nextAssociationId++;
        association->startedAt = startedAt;
        association->replacementId = replacementId;
        association->callback = listener;
        association->peerAddress = peerAddress;
        association->peerPort = peerPort;
        association->localAddress = localAddress;
        association->localPort = localPort;
        association->selection = selection;
        association->offer = offer;
        if (selection.authenticationMode == Parameters::AuthenticationMode::TICKET_PSK) {
            const auto& identityBytes = offer.pskIdentities.at(selection.selectedPsk);
            association->offeredSession = sessions.find(std::vector<uint8_t>(identityBytes.begin(), identityBytes.end()), true, simTime());
        }
        if (offer.hasEarlyData && selection.authenticationMode == Parameters::AuthenticationMode::TICKET_PSK && selection.selectedPsk == 0 &&
            handshake->getMessageSequence() == 0 && par("earlyDataEnabled").boolValue() && association->offeredSession &&
            association->offeredSession->cipherSuite == selection.cipherSuite) {
            std::vector<uint8_t> random;
            for (size_t i = 0; i < 32; ++i)
                random.push_back(hello->getRandom(i));
            association->earlyAccepted = sessions.consumeEarly(association->offeredSession->ticket, random, association->id, simTime());
            if (association->earlyAccepted)
                association->receiveEarly = std::make_unique<DtlsRecordState>(replayWindow);
        }
        association->clientAuthRequested = !selection.usesPsk() && par("clientAuthentication").boolValue();
        association->serverSide = true;
        association->addressValidated = handshake->getMessageSequence() == 1;
        association->state = association->clientAuthRequested ? HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE : HandshakeState::SERVER_WAIT_CLIENT_FINISHED;
        association->serverMessageBase = handshake->getMessageSequence();
        association->nextPeerMessage = association->serverMessageBase + 1;
        association->receiveEpoch2 = std::make_unique<DtlsRecordState>(replayWindow);
        association->receiveEpoch3 = std::make_unique<DtlsRecordState>(replayWindow);
        association->receivedBytes = attempt ? attempt->receivedBytes : (*creditCounted ? 0 : datagramBytes);
        *creditCounted = true;
        if (attempt) {
            association->sentBytes = attempt->sentBytes;
            association->nextEpoch0Record = attempt->nextRecord;
            removePrevalidation(attempt->id);
            if (operationalState != OPERATING || listener == nullptr)
                return;
            association->callback = listener;
        }
        association->handshakeDeadline = std::make_unique<cMessage>("dtlsHandshakeDeadline");
        association->cidOffered = offer.hasConnectionId && par("connectionIdEnabled").boolValue();
        association->cidNegotiated = association->cidOffered;
        association->heartbeatNegotiated = offer.heartbeatMode != 0 && par("heartbeatEnabled").boolValue();
        association->peerAllowsHeartbeat = association->heartbeatNegotiated && offer.heartbeatMode == 1;
        if (association->cidNegotiated) {
            association->peerCid = offer.connectionId;
            if (par("receiveConnectionId").boolValue()) {
                association->localCid = allocateConnectionId(association->id);
                if (association->localCid.empty()) {
                    emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
                    return;
                }
            }
        }
        auto id = association->id;
        scheduleAt(deadline, association->handshakeDeadline.get());
        associations.emplace(id, std::move(association));
        listener->accepted(id, localAddress, localPort, peerAddress, peerPort);
        if (associations.find(id) != associations.end())
            sendServerFlight(id);
    }
    else if (handshake->getMessageType() == 2) {
        for (const auto& entry : associations) {
            auto& association = *entry.second;
            if (association.peerAddress == peerAddress && association.peerPort == peerPort &&
                (association.localAddress.isUnspecified() || association.localAddress == localAddress) &&
                (association.localPort < 0 || association.localPort == localPort) &&
                association.state == HandshakeState::CLIENT_WAIT_SERVER_HELLO) {
                if (handshake->getMessageSequence() != association.serverMessageBase &&
                    !(association.retried && handshake->getMessageSequence() == 0))
                    return;
                if (handshake->getFragmentOffset() == 0 && handshake->getFragmentLength() == handshake->getMessageLength() &&
                    !association.reassembly.hasEntry(handshake->getMessageSequence())) {
                    auto id = entry.first;
                    association.processServerHello(handshake->getMessageSequence(), body, localAddress, localPort);
                    auto current = associations.find(id);
                    if (current != associations.end() && current->second->state != HandshakeState::CLIENT_WAIT_SERVER_HELLO)
                        current->second->noteHandshakeRecord(0, recordSequence, false);
                }
                else {
                    if (!hasConsistentEncoding(body))
                        return;
                    auto id = entry.first;
                    auto result = association.reassembly.insert(handshake->getMessageType(), handshake->getMessageSequence(),
                        handshake->getMessageLength(), handshake->getFragmentOffset(), body->peekDataAsBytes()->getBytes());
                    emitReassemblyBytes();
                    auto current = associations.find(id);
                    if (current == associations.end())
                        return;
                    if (result == DtlsHandshakeReassembly::Result::INCOMPLETE &&
                        handshake->getMessageSequence() == current->second->serverMessageBase)
                        current->second->acknowledgeFlightImplicitly();
                    if (result == DtlsHandshakeReassembly::Result::COMPLETE) {
                        auto message = current->second->reassembly.take(handshake->getMessageSequence());
                        emitReassemblyBytes();
                        current = associations.find(id);
                        if (current == associations.end())
                            return;
                        if (!current->second->pendingServerHello.empty()) {
                            if (current->second->pendingServerHelloSequence == handshake->getMessageSequence() &&
                                current->second->pendingServerHello == message.body)
                                sendAcknowledgements(id, 0, {{0, recordSequence}});
                            return;
                        }
                        current->second->pendingServerHello = message.body;
                        current->second->pendingServerHelloSequence = handshake->getMessageSequence();
                        auto complete = std::make_shared<std::unique_ptr<Packet>>(new Packet("reassembled DTLS ServerHello", makeShared<BytesChunk>(message.body)));
                        auto messageSequence = handshake->getMessageSequence();
                        bool fragmented = handshake->getFragmentOffset() != 0 || handshake->getFragmentLength() != handshake->getMessageLength();
                        bool duplicateRetry = current->second->retried && messageSequence == 0;
                        auto delay = (fragmented || recordChargePending) && !duplicateRetry ? SimTime(par("handshakeInDelay").doubleValue()) : SIMTIME_ZERO;
                        if (!enqueueProcessing(inputProcessing.get(), id, message.body.size() + 12, delay,
                            [this, id, messageSequence, complete, localAddress, localPort, recordSequence]() {
                                auto it = associations.find(id);
                                if (it != associations.end()) {
                                    it->second->pendingServerHello.clear();
                                    it->second->processServerHello(messageSequence, complete->get(), localAddress, localPort);
                                    it = associations.find(id);
                                    if (it != associations.end() && it->second->state != HandshakeState::CLIENT_WAIT_SERVER_HELLO)
                                        it->second->noteHandshakeRecord(0, recordSequence, false);
                                }
                            }))
                            failAssociation(id, "DTLS input processing queue is full");
                    }
                    else if (result == DtlsHandshakeReassembly::Result::INCOMPLETE && acknowledgePartial)
                        sendAcknowledgements(id, 0, {{0, recordSequence}});
                    else if (result == DtlsHandshakeReassembly::Result::CONFLICT || result == DtlsHandshakeReassembly::Result::LIMIT)
                        failAssociation(id, "invalid or excessive fragmented ServerHello", result == DtlsHandshakeReassembly::Result::CONFLICT ? 47 : -1);
                }
                return;
            }
        }
    }
}

void Dtls::handleProtectedHandshake(AssociationId id, uint16_t epoch, uint16_t truncatedSequence, unsigned sequenceBits, Packet *body, size_t recordBytes)
{
    auto it = associations.find(id);
    if (it == associations.end()) {
        emit(recordDroppedSignal, (long)UNKNOWN_EPOCH);
        return;
    }
    auto retained = std::make_shared<std::unique_ptr<Packet>>(body->dup());
    if (!inputProcessing->enqueue(id, recordBytes, SIMTIME_ZERO, [this, id, epoch, truncatedSequence, sequenceBits, recordBytes, retained]() {
        auto current = associations.find(id);
        if (current == associations.end())
            return;
        auto state = current->second->state;
        bool waitingForShare = epoch == 2 && state == HandshakeState::CLIENT_WAIT_SERVER_HELLO;
        bool serverAwaitingFlight = (state == HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE ||
                                     state == HandshakeState::SERVER_WAIT_CLIENT_CERTIFICATE_VERIFY ||
                                     state == HandshakeState::SERVER_WAIT_CLIENT_FINISHED) && epoch == 2 && !current->second->serverFlightQueued;
        bool earlyEpoch = epoch == 1 && current->second->receiveEarly && !current->second->peerFinishedProcessed;
        if (epoch == 1)
            EV_DEBUG << "DTLS epoch 1 input: association=" << id << ", early=" << earlyEpoch << ", closing=" << current->second->closing << endl;
        bool knownEpoch = earlyEpoch || (epoch == 2 && current->second->receiveEpoch2) || (current->second->receiveEpoch & 3) == epoch ||
                          (current->second->previousReceiveEpoch && (current->second->previousReceiveEpochNumber & 3) == epoch);
        if (waitingForShare || serverAwaitingFlight || !knownEpoch ||
            (epoch != 2 && !earlyEpoch && state != HandshakeState::ESTABLISHED && !(current->second->serverSide && current->second->ownFinishedHandedOff))) {
            emit(recordDroppedSignal, (long)UNKNOWN_EPOCH);
            return;
        }
        handleAvailableProtectedHandshake(id, epoch, truncatedSequence, sequenceBits, retained->get(), recordBytes);
    }))
        emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
}

void Dtls::handleAvailableProtectedHandshake(AssociationId id, uint16_t epoch, uint16_t truncatedSequence, unsigned sequenceBits, Packet *body, size_t recordBytes)
{
    // RFC 9147 Sections 4.2.2 and 4.5.1 select an epoch and check its replay window.
    auto it = associations.find(id);
    if (it == associations.end())
        return;
    auto retained = std::make_shared<std::unique_ptr<Packet>>(body->dup());
    auto delay = processingDelay(id, "protectInDelay");
    if (!inputProcessing->enqueueContinuation(id, recordBytes, delay, [this, id, epoch, truncatedSequence, sequenceBits, retained, recordBytes]() {
        auto current = associations.find(id);
        if (current == associations.end())
            return;
        if (auto socket = (*retained)->findTag<SocketInd>(); socket && socket->getSocketId() != udpSocket.getSocketId()) {
            emit(recordDroppedSignal, (long)UNSUPPORTED_PATH);
            return;
        }
        uint8_t contentType;
        if (!DtlsRecordProtection::unprotect(retained->get(), contentType)) {
            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            return;
        }
        auto& association = *current->second;
        uint64_t logicalEpoch = 0;
        DtlsRecordState *replay = nullptr;
        bool mainHandshake = epoch == 2 && association.receiveEpoch2 && association.state != HandshakeState::ESTABLISHED;
        if (epoch == 2 && association.receiveEpoch2 && association.state == HandshakeState::ESTABLISHED && contentType == 22 && (*retained)->getDataLength() >= B(12)) {
            try {
                auto header = (*retained)->peekAtFront<DtlsHandshakeHeader>(B(12));
                mainHandshake = header->getMessageSequence() < association.initialPeerMessageEnd;
            }
            catch (const cRuntimeError&) {
                emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                return;
            }
        }
        if (epoch == 2 && association.receiveEpoch2 && association.state == HandshakeState::ESTABLISHED && contentType == 26 &&
            (association.receiveEpoch & 3) != epoch &&
            (!association.previousReceiveEpoch || (association.previousReceiveEpochNumber & 3) != epoch))
            mainHandshake = true;
        if (epoch == 1 && association.receiveEarly && !association.peerFinishedProcessed) {
            logicalEpoch = 1;
            replay = association.receiveEarly.get();
        }
        else if (mainHandshake) {
            logicalEpoch = 2;
            replay = association.receiveEpoch2.get();
        }
        else if ((association.receiveEpoch & 3) == epoch &&
                 (association.state == HandshakeState::ESTABLISHED || (association.serverSide && association.ownFinishedHandedOff))) {
            logicalEpoch = association.receiveEpoch;
            replay = association.receiveEpoch3.get();
        }
        else if (association.previousReceiveEpoch && (association.previousReceiveEpochNumber & 3) == epoch) {
            logicalEpoch = association.previousReceiveEpochNumber;
            replay = association.previousReceiveEpoch.get();
        }
        if (!replay) {
            emit(recordDroppedSignal, (long)UNKNOWN_EPOCH);
            return;
        }
        auto sequence = replay->reconstruct(truncatedSequence, sequenceBits);
        if (!replay->isAcceptable(sequence)) {
            emit(recordDroppedSignal, (long)REPLAY);
            return;
        }
        auto path = association.receivedPath(retained->get());
        bool changedPath = !sameTuple(path, association.currentPath());
        bool fresh = association.isNewerRecord(logicalEpoch, sequence);
        if (changedPath) {
            if (logicalEpoch < 3 || (contentType != 21 && contentType != 22 && contentType != 23 && contentType != 24 && contentType != 26) ||
                !association.considerPath(path, fresh)) {
                emit(recordDroppedSignal, (long)UNSUPPORTED_PATH);
                return;
            }
            current = associations.find(id);
            if (current == associations.end())
                return;
        }
        auto retirePrevious = [this, &association, logicalEpoch]() {
            if (logicalEpoch == association.receiveEpoch && !par("resumptionEnabled").boolValue() && !association.cidNegotiated && !par("postHandshakeAuthEnabled").boolValue())
                association.previousReceiveEpoch.reset();
        };
        if (logicalEpoch == 1) {
            EV_DEBUG << "DTLS early content: type=" << int(contentType) << ", server=" << association.serverSide << ", accepted=" << association.earlyAccepted << endl;
            if (contentType != 23 || !association.serverSide || !association.earlyAccepted || association.closing)
                return;
            auto bytes = (*retained)->getDataLength().get<B>();
            auto limit = association.offeredSession->earlyDataBytes;
            if (uint64_t(bytes) > limit - association.earlyReceivedBytes) {
                failAssociation(id, "DTLS early data allowance exceeded", 10);
                return;
            }
            replay->accept(sequence);
            association.earlyReceivedBytes += bytes;
            auto incoming = retained->release();
            auto callback = association.callback;
            emit(packetReceivedSignal, incoming);
            emit(packetSentToUpperSignal, incoming);
            drop(incoming);
            callback->earlyDataArrived(id, incoming);
            return;
        }
        if (contentType == 21 && (*retained)->getDataLength() == B(2)) {
            try {
                auto alert = (*retained)->popAtFront<DtlsAlert>(B(2));
                replay->accept(sequence);
                retirePrevious();
                if (alert->getDescription() == 0)
                    close(id);
                else if (alert->getDescription() != 90)
                    failAssociation(id, "peer sent a fatal DTLS alert");
            }
            catch (const cRuntimeError&) {
                emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            }
            return;
        }
        if ((logicalEpoch == association.receiveEpoch || changedPath) && contentType == 24 && association.state == HandshakeState::ESTABLISHED && !association.closing) {
            association.receiveHeartbeat(logicalEpoch, sequence, retained->get(), path, fresh, changedPath, recordBytes);
            return;
        }
        if (logicalEpoch >= 3 && contentType == 23) {
            if (current->second->state != HandshakeState::ESTABLISHED || current->second->closing) {
                emit(recordDroppedSignal, (long)UNKNOWN_EPOCH);
                return;
            }
            replay->accept(sequence);
            retirePrevious();
            auto callback = current->second->callback;
            auto incoming = retained->release();
            emit(packetReceivedSignal, incoming);
            emit(packetSentToUpperSignal, incoming);
            drop(incoming);
            callback->dataArrived(id, incoming);
            return;
        }
        if (contentType == 26) {
            if ((*retained)->getDataLength() < B(2))
                return;
            if (!hasConsistentEncoding(retained->get())) {
                emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                return;
            }
            try {
                auto ack = (*retained)->popAtFront<DtlsAck>((*retained)->getDataLength());
                if ((*retained)->getDataLength() != b(0)) {
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                    return;
                }
                replay->accept(sequence);
                retirePrevious();
                current->second->acceptAck(*ack);
            }
            catch (const cRuntimeError&) {
                emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                return;
            }
            return;
        }
        if (logicalEpoch >= 3 && contentType == 22 && current->second->state == HandshakeState::ESTABLISHED) {
            try {
                auto type = (*retained)->getDataLength() >= B(12) ? (*retained)->peekAtFront<DtlsHandshakeHeader>(B(12))->getMessageType() : 0;
                if (type == 4)
                    current->second->receiveTicket(logicalEpoch, sequence, retained->get());
                else if (type == 9 || type == 10)
                    current->second->receiveConnectionIdMessage(logicalEpoch, sequence, retained->get());
                else if (type == 13 || type == 11 || type == 15 || type == 20)
                    current->second->receiveAuthentication(logicalEpoch, sequence, retained->get());
                else
                    current->second->receiveKeyUpdate(logicalEpoch, sequence, retained->get());
            }
            catch (const cRuntimeError&) {
                emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            }
            return;
        }
        if (logicalEpoch != 2 || contentType != 22) {
            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            return;
        }
        try {
            current->second->receiveHandshakeFragment(sequence, retained->get());
        }
        catch (const cRuntimeError&) {
            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
        }
    }, {}, {}, [this, delay]() { emit(processingDelaySignal, delay.dbl()); }))
        emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
}

void Dtls::send(AssociationId id, Packet *packet, ICallback *requester)
{
    Enter_Method("send");
    if (packet == nullptr || requester == nullptr)
        throw cRuntimeError("Invalid DTLS send request");
    take(packet);
    if (operationalState != OPERATING) {
        auto packetId = packet->getId();
        delete packet;
        requester->sendError(id, packetId, "DTLS engine is not operating");
        return;
    }
    auto it = associations.find(id);
    if (it == associations.end() || it->second->callback != requester) {
        auto packetId = packet->getId();
        delete packet;
        requester->sendError(id, packetId, "unknown or foreign DTLS association");
        return;
    }
    if (it->second->closing) {
        auto packetId = packet->getId();
        delete packet;
        requester->sendError(id, packetId, "DTLS association is closing");
        return;
    }
    if (it->second->state == HandshakeState::ESTABLISHED) {
        sendApplication(id, packet);
        return;
    }
    auto bytes = std::max(size_t(1), (size_t)packet->getByteLength());
    auto limit = par("maxPendingBytes").intValue();
    if (limit <= 0 || bytes > (size_t)limit || it->second->pendingBytes > (size_t)limit - bytes) {
        auto packetId = packet->getId();
        delete packet;
        requester->sendError(id, packetId, "DTLS pending application queue is full");
        return;
    }
    it->second->pendingBytes += bytes;
    it->second->pendingPackets.push_back({std::unique_ptr<Packet>(packet), {}, false});
}

void Dtls::close(AssociationId id)
{
    Enter_Method("close");
    if (operationalState != OPERATING)
        return;
    beginClose(id);
}

void Dtls::beginClose(AssociationId id)
{
    // RFC 9846 Section 6.1 sends close_notify; the bounded drain deadline is model policy.
    auto it = associations.find(id);
    if (it == associations.end() || it->second->closing)
        return;
    it->second->closing = true;
    bool established = it->second->state == HandshakeState::ESTABLISHED;
    cancelEvent(it->second->handshakeDeadline.get());
    it->second->cancelTimers();
    if (inputProcessing)
        inputProcessing->cancel(id);
    if (outputProcessing)
        outputProcessing->cancel(id);
    it = associations.find(id);
    if (it == associations.end())
        return;
    auto callback = it->second->callback;
    auto pending = std::move(it->second->pendingPackets);
    it->second->pendingPackets.clear();
    it->second->pendingBytes = 0;
    auto processingIds = std::move(it->second->processingPacketIds);
    it->second->processingPacketIds.clear();
    for (auto& item : pending) {
        auto packetId = item.packet->getId();
        item.packet.reset();
        callback->sendError(id, packetId, "association closing");
    }
    for (auto packetId : processingIds)
        callback->sendError(id, packetId, "association closing");
    it = associations.find(id);
    if (it == associations.end())
        return;
    if (!established) {
        finishClose(id, false);
        return;
    }
    auto timeout = SimTime(par("closeTimeout").doubleValue());
    if (timeout <= SIMTIME_ZERO) {
        finishClose(id, true);
        return;
    }
    scheduleAt(simTime() + timeout, it->second->closeDeadline.get());
    auto delay = processingDelay(id, "protectOutDelay");
    if (!enqueueProcessing(outputProcessing.get(), id, 2 + it->second->recordOverhead(), delay, [this, id]() {
        auto current = associations.find(id);
        if (current == associations.end() || !current->second->closing)
            return;
        uint64_t number;
        if (!current->second->allocateApplicationRecord(number)) {
            finishClose(id, true);
            return;
        }
        auto alert = makeShared<DtlsAlert>();
        alert->setLevel(1);
        alert->setDescription(0);
        auto packet = new Packet("DTLS close_notify", alert);
        DtlsRecordProtection::protect(packet, 21);
        auto header = current->second->makeRecordHeader(current->second->sendEpoch, number, packet->getDataLength().get<B>());
        packet->insertAtFront(header);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
        if (!current->second->localAddress.isUnspecified())
            packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(current->second->localAddress);
        if (current->second->localInterfaceId >= 0)
            packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(current->second->localInterfaceId);
        auto peer = current->second->peerAddress;
        auto port = current->second->peerPort;
        emit(packetSentToLowerSignal, packet);
        sendUdp(packet, peer, port);
        finishClose(id, false);
    }))
        finishClose(id, true);
}

void Dtls::finishClose(AssociationId id, bool forced)
{
    auto it = associations.find(id);
    if (it == associations.end())
        return;
    auto association = std::move(it->second);
    auto callback = association->callback;
    auto hadReassembly = association->getReassemblyBytes() != 0;
    cancelEvent(association->handshakeDeadline.get());
    association->cancelTimers();
    associations.erase(it);
    removeConnectionIds(id);
    if (inputProcessing)
        inputProcessing->cancel(id);
    if (outputProcessing)
        outputProcessing->cancel(id);
    if (hadReassembly)
        emitReassemblyBytes();
    for (auto& pending : association->pendingPackets) {
        auto packetId = pending.packet->getId();
        pending.packet.reset();
        callback->sendError(id, packetId, "association closed");
    }
    for (auto packetId : association->processingPacketIds)
        callback->sendError(id, packetId, "association closed");
    if (association->earlyRequested && !association->earlyStatusReported)
        callback->earlyDataStatus(id, false);
    if (association->authenticationRequest)
        callback->authenticationResult(id, false, "");
    if (forced)
        callback->error(id, "DTLS closure forced");
    callback->closed(id);
    if (operationalState == STOPPING_OPERATION && associations.empty())
        udpSocket.close();
}

bool Dtls::requestKeyUpdate(AssociationId id, bool requestPeerUpdate)
{
    Enter_Method("requestKeyUpdate");
    if (operationalState != OPERATING)
        return false;
    auto it = associations.find(id);
    return it != associations.end() && it->second->requestUpdate(requestPeerUpdate);
}

void Dtls::discardAll(const char *reason, bool notify)
{
    bool hadReassembly = false;
    for (const auto& attempt : prevalidationAttempts)
        hadReassembly |= attempt->reassembly.getReservedBytes() != 0;
    for (const auto& entry : associations)
        hadReassembly |= entry.second->getReassemblyBytes() != 0;
    if (inputProcessing)
        inputProcessing->clear();
    if (outputProcessing)
        outputProcessing->clear();
    for (const auto& attempt : prevalidationAttempts)
    {
        cancelEvent(attempt->deadline.get());
        cancelEvent(attempt->retryTimer.get());
    }
    prevalidationAttempts.clear();
    for (const auto& entry : associations) {
        cancelEvent(entry.second->handshakeDeadline.get());
        entry.second->cancelTimers();
    }
    auto old = std::move(associations);
    associations.clear();
    cidIndex.clear();
    if (notify && hadReassembly)
        emitReassemblyBytes();
    if (notify) {
        for (const auto& entry : old) {
            if (entry.second->authenticationRequest)
                entry.second->callback->authenticationResult(entry.first, false, "");
            if (entry.second->earlyRequested && !entry.second->earlyStatusReported)
                entry.second->callback->earlyDataStatus(entry.first, false);
            for (auto& pending : entry.second->pendingPackets) {
                auto packetId = pending.packet->getId();
                pending.packet.reset();
                entry.second->callback->sendError(entry.first, packetId, reason);
            }
            for (auto packetId : entry.second->processingPacketIds)
                entry.second->callback->sendError(entry.first, packetId, reason);
            entry.second->callback->error(entry.first, reason);
            entry.second->callback->closed(entry.first);
        }
    }
}

void Dtls::socketDataArrived(UdpSocket *, Packet *packet)
{
    if (operationalState != OPERATING) {
        delete packet;
        return;
    }
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&dtlsProtocol);
    emit(packetReceivedFromLowerSignal, packet);
    if (packet->hasBitError()) {
        emit(recordDroppedSignal, (long)MALFORMED_RECORD);
        delete packet;
        return;
    }
    parseDatagram(packet);
    delete packet;
}

void Dtls::parseDatagram(Packet *packet)
{
    // RFC 9147 Sections 4.1 and 4.3 permit several DTLS records in one datagram.
    const auto addressTag = packet->findTag<L3AddressInd>();
    const auto portTag = packet->findTag<L4PortInd>();
    L3Address localAddress = addressTag ? addressTag->getDestAddress() : L3Address();
    L3Address peerAddress = addressTag ? addressTag->getSrcAddress() : L3Address();
    int localPort = portTag ? portTag->getDestPort() : -1;
    int peerPort = portTag ? portTag->getSrcPort() : -1;
    size_t datagramBytes = packet->getByteLength();
    auto creditCounted = std::make_shared<bool>(false);
    AssociationId datagramAssociation = 0;
    while (packet->getDataLength() > b(0)) {
        try {
            auto first = packet->peekAtFront<BytesChunk>(B(1))->getByte(0);
            if (first == 21 || first == 22 || first == 26) {
                if (packet->getDataLength() < B(13)) {
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                    return;
                }
                auto header = packet->removeAtFront<DtlsPlaintextHeader>(B(13));
                // RFC 9147 section 4: legacy_record_version is ignored on receipt.
                if (header->getEpoch() != 0 || packet->getDataLength() < B(header->getLength())) {
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                    return;
                }
                emit(recordAcceptedSignal, (long)header->getEpoch());
                if (header->getLength() > DtlsRecordProtection::MAX_CONTENT_BYTES) {
                    packet->removeAtFront(B(header->getLength()));
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                    continue;
                }
                if (header->getLength() != 0) {
                    Packet body("DTLS plaintext record", packet->removeAtFront(B(header->getLength())));
                    if (header->getContentType() == 22) {
                        if (body.getDataLength() < B(12)) {
                            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                            continue;
                        }
                        auto retained = std::make_shared<std::unique_ptr<Packet>>(body.dup());
                        AssociationId id = 0;
                        bool attemptOwned = false;
                        bool createdAttempt = false;
                        auto handshake = body.peekAtFront<DtlsHandshakeHeader>(B(12));
                        bool fragmented = handshake->getFragmentOffset() != 0 || handshake->getFragmentLength() != handshake->getMessageLength();
                        if (handshake->getMessageType() == 1 && listener && handshake->getMessageSequence() <= 1 &&
                            handshake->getFragmentLength() != 0 && handshake->getFragmentOffset() <= handshake->getMessageLength() &&
                            handshake->getFragmentLength() <= handshake->getMessageLength() - handshake->getFragmentOffset() &&
                            body.getDataLength() >= B(12 + handshake->getFragmentLength()) && !peerAddress.isUnspecified() && peerPort >= 0) {
                            for (const auto& attempt : prevalidationAttempts)
                                if (attempt->localAddress == localAddress && attempt->localPort == localPort &&
                                    attempt->peerAddress == peerAddress && attempt->peerPort == peerPort) {
                                    id = attempt->id;
                                    attemptOwned = true;
                                    break;
                                }
                            AssociationId candidateId = 0;
                            if (!attemptOwned)
                                for (const auto& entry : associations)
                                    if (entry.second->serverSide && entry.second->state != HandshakeState::ESTABLISHED &&
                                        entry.second->localAddress == localAddress && entry.second->localPort == localPort &&
                                        entry.second->peerAddress == peerAddress && entry.second->peerPort == peerPort) {
                                        candidateId = entry.first;
                                        break;
                                    }
                            if (!attemptOwned && candidateId != 0 && !fragmented)
                                id = candidateId;
                            else if (!attemptOwned) {
                                auto maxHalfOpen = par("maxHalfOpen").intValue();
                                auto limit = par("maxReassemblyBytes").intValue();
                                auto timeout = SimTime(par("handshakeTimeout").doubleValue());
                                if (maxHalfOpen <= 0 || (candidateId == 0 && countHalfOpen() >= size_t(maxHalfOpen)) ||
                                    limit < 0 || timeout <= SIMTIME_ZERO || nextAssociationId == 0) {
                                    emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
                                    continue;
                                }
                                auto attempt = std::make_unique<PrevalidationAttempt>(limit);
                                attempt->id = nextAssociationId++;
                                attempt->localAddress = localAddress;
                                attempt->localPort = localPort;
                                attempt->peerAddress = peerAddress;
                                attempt->peerPort = peerPort;
                                id = attempt->id;
                                scheduleAt(simTime() + timeout, attempt->deadline.get());
                                prevalidationAttempts.push_back(std::move(attempt));
                                attemptOwned = createdAttempt = true;
                            }
                        }
                        else if (handshake->getMessageType() != 1)
                            for (const auto& entry : associations)
                                if (entry.second->peerAddress == peerAddress && entry.second->peerPort == peerPort &&
                                    (entry.second->localAddress.isUnspecified() || entry.second->localAddress == localAddress) &&
                                    (entry.second->localPort < 0 || entry.second->localPort == localPort) &&
                                    entry.second->state != HandshakeState::ESTABLISHED) {
                                    id = entry.first;
                                    break;
                                }
                        bool duplicate = false;
                        bool markedInitialHello = false;
                        bool markedServerHello = false;
                        if (attemptOwned && handshake->getMessageType() == 1 && handshake->getMessageSequence() == 0) {
                            auto attempt = findPrevalidation(id);
                            if (attempt && !fragmented && body.getDataLength() == B(12 + handshake->getFragmentLength())) {
                                Packet copy(body);
                                copy.popAtFront<DtlsHandshakeHeader>(B(12));
                                if (hasConsistentEncoding(&copy)) {
                                    auto bytes = copy.peekDataAsBytes()->getBytes();
                                    duplicate = (!attempt->initialHello.empty() && attempt->initialHello == bytes) ||
                                                (!attempt->pendingInitialHello.empty() && attempt->pendingInitialHello == bytes);
                                    if (attempt->pendingInitialHello.empty()) {
                                        attempt->pendingInitialHello = std::move(bytes);
                                        markedInitialHello = true;
                                    }
                                }
                            }
                        }
                        else if (id != 0 && handshake->getMessageType() == 2 && handshake->getMessageSequence() == 0) {
                            auto association = associations.find(id);
                            duplicate = association != associations.end() && association->second->retried;
                        }
                        if (id != 0 && handshake->getMessageType() == 2 && !fragmented &&
                            body.getDataLength() == B(12 + handshake->getFragmentLength())) {
                            auto association = associations.find(id);
                            if (association != associations.end() && association->second->state == HandshakeState::CLIENT_WAIT_SERVER_HELLO &&
                                (handshake->getMessageSequence() == association->second->serverMessageBase ||
                                 (association->second->retried && handshake->getMessageSequence() == 0))) {
                                Packet copy(body);
                                copy.popAtFront<DtlsHandshakeHeader>(B(12));
                                if (hasConsistentEncoding(&copy)) {
                                    auto bytes = copy.peekDataAsBytes()->getBytes();
                                    if (association->second->pendingServerHello.empty()) {
                                        association->second->pendingServerHello = std::move(bytes);
                                        association->second->pendingServerHelloSequence = handshake->getMessageSequence();
                                        markedServerHello = true;
                                    }
                                    else if (association->second->pendingServerHelloSequence == handshake->getMessageSequence() &&
                                             association->second->pendingServerHello == bytes)
                                        duplicate = true;
                                }
                            }
                        }
                        auto delay = fragmented || duplicate ? SIMTIME_ZERO : SimTime(par("handshakeInDelay").doubleValue());
                        auto recordSequence = header->getSequenceNumber();
                        auto eligible = [this, id, attemptOwned]() { return id == 0 || (attemptOwned ? findPrevalidation(id) != nullptr : containsAssociation(id)); };
                        if (!enqueueProcessing(inputProcessing.get(), id, header->getLength() + 13, delay, [this, retained, localAddress, localPort, peerAddress, peerPort, datagramBytes, creditCounted, recordSequence, id, attemptOwned, markedInitialHello, markedServerHello]() {
                            if (markedInitialHello) {
                                auto current = findPrevalidation(id);
                                if (current)
                                    current->pendingInitialHello.clear();
                            }
                            if (markedServerHello) {
                                auto association = associations.find(id);
                                if (association != associations.end())
                                    association->second->pendingServerHello.clear();
                            }
                            try {
                                handlePlainHandshakeRecord(retained->get(), localAddress, localPort, peerAddress, peerPort, datagramBytes, creditCounted, recordSequence);
                            }
                            catch (const cRuntimeError&) {
                                emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                            }
                            if (attemptOwned) {
                                auto current = findPrevalidation(id);
                                if (current && !current->failing && !current->processingHello && !current->retryFlight && current->initialHello.empty() && current->reassembly.getEntryCount() == 0)
                                    removePrevalidation(id);
                            }
                        }, eligible)) {
                            if (createdAttempt)
                                removePrevalidation(id);
                            else if (markedInitialHello) {
                                auto attempt = findPrevalidation(id);
                                if (attempt)
                                    attempt->pendingInitialHello.clear();
                            }
                            if (markedServerHello) {
                                auto association = associations.find(id);
                                if (association != associations.end())
                                    association->second->pendingServerHello.clear();
                            }
                            emit(recordDroppedSignal, (long)RESOURCE_LIMIT);
                        }
                    }
                    else if (header->getContentType() == 26) {
                        if (body.getDataLength() < B(2) || !hasConsistentEncoding(&body)) {
                            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                            continue;
                        }
                        auto ack = body.popAtFront<DtlsAck>(body.getDataLength());
                        if (body.getDataLength() != b(0)) {
                            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                            continue;
                        }
                        uint64_t retryId = 0;
                        for (const auto& attempt : prevalidationAttempts)
                            if (attempt->localAddress == localAddress && attempt->localPort == localPort &&
                                attempt->peerAddress == peerAddress && attempt->peerPort == peerPort) {
                                retryId = attempt->id;
                                break;
                            }
                        if (retryId != 0) {
                            auto attempt = findPrevalidation(retryId);
                            if (attempt && !*creditCounted) {
                                attempt->receivedBytes = datagramBytes > std::numeric_limits<size_t>::max() - attempt->receivedBytes ?
                                    std::numeric_limits<size_t>::max() : attempt->receivedBytes + datagramBytes;
                                *creditCounted = true;
                            }
                            acknowledgeRetry(retryId, *ack);
                            pumpRetry(retryId);
                        }
                        for (const auto& entry : associations)
                            if (entry.second->peerAddress == peerAddress && entry.second->peerPort == peerPort &&
                                (entry.second->localAddress.isUnspecified() || entry.second->localAddress == localAddress) &&
                                (entry.second->localPort < 0 || entry.second->localPort == localPort)) {
                                entry.second->acceptAck(*ack);
                                break;
                            }
                    }
                    else if (header->getContentType() == 21 && body.getDataLength() == B(2)) {
                        auto alert = body.popAtFront<DtlsAlert>(B(2));
                        if (alert->getDescription() != 90) {
                            AssociationId target = 0;
                            for (const auto& entry : associations)
                                if (entry.second->peerAddress == peerAddress && entry.second->peerPort == peerPort &&
                                    (entry.second->localAddress.isUnspecified() || entry.second->localAddress == localAddress) &&
                                    (entry.second->localPort < 0 || entry.second->localPort == localPort) &&
                                    entry.second->state == HandshakeState::CLIENT_WAIT_SERVER_HELLO) {
                                    target = entry.first;
                                    break;
                                }
                            if (target != 0) {
                                if (alert->getDescription() == 0)
                                    beginClose(target);
                                else
                                    failAssociation(target, "peer sent a fatal DTLS alert");
                            }
                        }
                    }
                }
            }
            else if ((first & 0xe0) == 0x20) {
                size_t headerLength = 1 + ((first & 0x10) ? 8 : 0) + ((first & 8) ? 2 : 1) + ((first & 4) ? 2 : 0);
                if (packet->getDataLength() < B(headerLength)) {
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                    return;
                }
                auto header = packet->removeAtFront<DtlsCiphertextHeader>(B(headerLength));
                b length = header->getLengthPresent() ? b(B(header->getLength())) : packet->getDataLength();
                if (length > packet->getDataLength()) {
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                    return;
                }
                emit(recordAcceptedSignal, (long)header->getEpochBits());
                if (length > B(DtlsRecordProtection::MAX_CIPHERTEXT_BYTES)) {
                    packet->removeAtFront(length);
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                    continue;
                }
                auto match = associations.end();
                if (first & 0x10) {
                    std::vector<uint8_t> cid;
                    for (size_t i = 0; i < header->getConnectionIdArraySize(); ++i)
                        cid.push_back(header->getConnectionId(i));
                    auto indexed = cidIndex.find(cid);
                    if (indexed != cidIndex.end())
                        match = associations.find(indexed->second);
                    if (match == associations.end() || !match->second->cidNegotiated || match->second->localCid.empty()) {
                        emit(recordDroppedSignal, (long)UNKNOWN_CID);
                        return;
                    }
                    if (match->second->peerAddress != peerAddress || match->second->peerPort != peerPort ||
                        (!match->second->localAddress.isUnspecified() && match->second->localAddress != localAddress) ||
                        (match->second->localPort >= 0 && match->second->localPort != localPort)) {
                        if (!par("migrationEnabled").boolValue() || match->second->state != HandshakeState::ESTABLISHED) {
                            emit(recordDroppedSignal, (long)UNSUPPORTED_PATH);
                            return;
                        }
                    }
                }
                else for (auto it = associations.begin(); it != associations.end(); ++it) {
                    if (it->second->peerAddress == peerAddress && it->second->peerPort == peerPort &&
                        (it->second->localAddress.isUnspecified() || it->second->localAddress == localAddress) &&
                        (it->second->localPort < 0 || it->second->localPort == localPort)) {
                        match = it;
                        if ((header->getEpochBits() == 1 && it->second->receiveEarly) ||
                            (header->getEpochBits() == 2 && it->second->state != HandshakeState::ESTABLISHED) ||
                            (header->getEpochBits() == 3 && it->second->state == HandshakeState::ESTABLISHED))
                            break;
                    }
                }
                if (match != associations.end()) {
                    // Epoch 1 may precede the current ServerHello and therefore its CID negotiation.
                    bool unnegotiatedEarly = header->getEpochBits() == 1 && match->second->receiveEarly && !match->second->peerFinishedProcessed;
                    if (!(first & 0x10) && !unnegotiatedEarly && match->second->cidNegotiated && !match->second->localCid.empty()) {
                        emit(recordDroppedSignal, (long)UNKNOWN_CID);
                        return;
                    }
                    if (datagramAssociation != 0 && datagramAssociation != match->first) {
                        emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                        return;
                    }
                    datagramAssociation = match->first;
                }
                if (length > b(0)) {
                    auto bodyOffset = packet->getFrontOffset();
                    auto tags = packet->getRegionTags();
                    Packet body("DTLS protected record", packet->removeAtFront(length));
                    body.getRegionTags().copyTags(tags, bodyOffset, body.getFrontOffset(), length);
                    if (par("migrationEnabled").boolValue()) {
                        auto address = body.addTag<L3AddressInd>();
                        address->setSrcAddress(peerAddress);
                        address->setDestAddress(localAddress);
                        auto port = body.addTag<L4PortInd>();
                        port->setSrcPort(peerPort);
                        port->setDestPort(localPort);
                        if (auto interface = packet->findTag<InterfaceInd>())
                            body.addTag<InterfaceInd>()->setInterfaceId(interface->getInterfaceId());
                        body.addTag<SocketInd>()->setSocketId(udpSocket.getSocketId());
                    }
                    if (match != associations.end())
                        handleProtectedHandshake(match->first, header->getEpochBits(), header->getSequenceNumber(), header->getSequenceNumberLength() * 8, &body, (B(headerLength) + length).get<B>());
                    else
                        emit(recordDroppedSignal, (long)UNKNOWN_EPOCH);
                }
                else
                    emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            }
            else {
                emit(recordDroppedSignal, (long)MALFORMED_RECORD);
                return;
            }
        }
        catch (const cRuntimeError&) {
            emit(recordDroppedSignal, (long)MALFORMED_RECORD);
            return;
        }
    }
}

void Dtls::socketErrorArrived(UdpSocket *, Indication *indication)
{
    if (par("pathMtuDiscoveryEnabled").boolValue()) {
        const Packet *quote = nullptr;
        int mtu = -1;
        if (auto error = indication->findTag<Icmpv4ErrorInd>(); error && error->getType() == ICMP_DESTINATION_UNREACHABLE && error->getCode() == ICMP_DU_FRAGMENTATION_NEEDED) {
            quote = error->getOriginalPacket();
            mtu = error->getMtu();
        }
        else if (auto error = indication->findTag<Icmpv6ErrorInd>(); error && error->getType() == ICMPv6_PACKET_TOO_BIG) {
            quote = error->getOriginalPacket();
            mtu = error->getMtu();
        }
        if (quote) {
            std::vector<AssociationId> handles;
            for (const auto& entry : associations)
                handles.push_back(entry.first);
            for (auto handle : handles) {
                auto it = associations.find(handle);
                if (it != associations.end())
                    it->second->handlePacketTooBig(quote, mtu);
            }
        }
    }
    EV_WARN << "DTLS UDP error " << indication->getName() << endl;
    delete indication;
}

void Dtls::socketClosed(UdpSocket *)
{
    if (operationalState == STOPPING_OPERATION)
        finishActiveOperation();
}

} // namespace inet::dtls
