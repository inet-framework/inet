// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSASSOCIATION_H
#define INET_DTLSASSOCIATION_H

#include <array>
#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <vector>

#include "inet/transportlayer/contract/dtls/IDtls.h"
#include "inet/transportlayer/dtls/DtlsRecordState.h"
#include "inet/transportlayer/dtls/DtlsHandshakeReassembly.h"
#include "inet/transportlayer/dtls/DtlsHandshakeFlight.h"
#include "inet/transportlayer/dtls/DtlsPostHandshakeExchange.h"
#include "inet/transportlayer/dtls/DtlsPathMtuState.h"
#include "inet/transportlayer/dtls/DtlsSessionStore.h"
#include "inet/transportlayer/tls/TlsHandshakeParameters.h"

namespace inet::dtls {

class Dtls;
class DtlsClientHello;
class DtlsAck;
class DtlsCiphertextHeader;
class DtlsHeartbeat;

/** Per-peer handshake, record and application state owned by one DTLS engine. */
class INET_API DtlsAssociation
{
    friend class Dtls;

  public:
    using AssociationId = IDtls::AssociationId;
    enum class HandshakeState { CLIENT_WAIT_SERVER_HELLO, CLIENT_WAIT_SERVER_EXTENSIONS, CLIENT_WAIT_SERVER_CERTIFICATE,
        CLIENT_WAIT_SERVER_CERTIFICATE_VERIFY, CLIENT_WAIT_SERVER_FINISHED, CLIENT_WAIT_FINISHED_HANDOFF,
        SERVER_WAIT_CLIENT_CERTIFICATE, SERVER_WAIT_CLIENT_CERTIFICATE_VERIFY, SERVER_WAIT_CLIENT_FINISHED, ESTABLISHED };

  protected:
    // RFC 9147 Section 8 limits the sending epoch; the protected-record cap is model policy.
    static constexpr uint64_t MAX_SEND_EPOCH = (uint64_t(1) << 48) - 1;
    static constexpr uint64_t MAX_PLAINTEXT_RECORDS = uint64_t(1) << 48;
    static constexpr uint64_t MAX_PROTECTED_RECORDS = uint64_t(1) << 24;
    static constexpr uint64_t MAX_REKEY_AFTER_RECORDS = uint64_t(1) << 23;
    static constexpr uint16_t DTLS_12 = 0xfefd;
    static constexpr uint16_t PSK_AES_128_GCM_SHA256 = 0x00a8;
    static constexpr uint16_t ECDHE_ECDSA_AES_128_GCM_SHA256 = 0xc02b;
    static constexpr std::array<uint8_t, 32> HELLO_RETRY_RANDOM = {
        0xcf, 0x21, 0xad, 0x74, 0xe5, 0x9a, 0x61, 0x11, 0xbe, 0x1d, 0x8c, 0x02, 0x1e, 0x65, 0xb8, 0x91,
        0xc2, 0xa2, 0x11, 0x16, 0x7a, 0xbb, 0x8c, 0x5e, 0x07, 0x9e, 0x09, 0xe2, 0xc8, 0xa8, 0x33, 0x9c
    };
    Dtls& engine;
    struct LegacyHandshake {
        enum class Phase { CLIENT_WAIT_SERVER_HELLO, CLIENT_WAIT_SERVER_CERTIFICATE,
            CLIENT_WAIT_SERVER_KEY_EXCHANGE, CLIENT_WAIT_REQUEST_OR_DONE, CLIENT_WAIT_SERVER_DONE,
            CLIENT_WAIT_SERVER_TICKET, CLIENT_WAIT_SERVER_CCS, CLIENT_WAIT_SERVER_FINISHED, SERVER_WAIT_CLIENT_CERTIFICATE,
            SERVER_WAIT_CLIENT_KEY_EXCHANGE, SERVER_WAIT_CLIENT_CERTIFICATE_VERIFY,
            SERVER_WAIT_CLIENT_CCS, SERVER_WAIT_CLIENT_FINISHED, COMPLETE };
        Phase phase = Phase::CLIENT_WAIT_SERVER_HELLO;
        bool pendingCcs = false;
        bool ticketExpected = false;
        std::vector<uint8_t> sessionId;
        std::optional<DtlsSessionStore::Entry> pendingSession;
    };
    std::unique_ptr<LegacyHandshake> legacyHandshake;
    bool isDtls12() const { return selection.version == DTLS_12; }
    void initializeDtls12(bool server, uint16_t helloSequence);
    void sendClientHello12(const std::vector<uint8_t>& cookie = {});
    void sendServerFlight12();
    void sendClientFlight12();
    void sendFinalFlight12();
    void appendClientSession12(const Ptr<DtlsClientHello>& hello, std::vector<uint8_t>& extensions);
    void prepareServerSession12(const Ptr<const DtlsClientHello>& hello);
    void fillServerSessionHello12(const Ptr<tls::TlsServerHello>& hello);
    bool processSessionHello12(const Ptr<const tls::TlsServerHello>& hello);
    void appendSessionTicket12();
    void receiveSessionTicket12(Packet *body);
    void cacheSession12();
    void invalidateSessions12();
    std::vector<uint8_t> offeredSessionId12() const;
    void appendChangeCipherSpec12();
    void receiveHandshake12(Packet *body, uint64_t epoch);
    void receiveHandshakeFragment12(Packet *body, uint64_t epoch);
    void pumpHandshake12();
    void processHandshake12(uint8_t type, uint16_t sequence, const std::vector<uint8_t>& bytes);
    void receiveChangeCipherSpec12();
    void refuseRenegotiation12();
    void applyChangeCipherSpec12();
    void retransmitFlight12();
    void protectRecord(Packet *packet, uint8_t type, uint64_t epoch, uint64_t number, const std::vector<uint8_t> *cidOverride = nullptr);
    static Ptr<Chunk> makeRecordHeader12(uint8_t type, uint16_t epoch, uint64_t number, uint16_t length, const std::vector<uint8_t>& cid = {});
    bool writeCipherActive12() const;
    DtlsHandshakeReassembly reassembly;
    std::shared_ptr<DtlsHandshakeFlight> flight;
    std::unique_ptr<cMessage> retransmissionTimer;
    std::unique_ptr<cMessage> delayedAckTimer;
    std::unique_ptr<cMessage> retentionTimer;
    std::unique_ptr<cMessage> closeDeadline;
    // Outgoing single messages use their type; incoming owners set bit 8.
    // PHA response flights use a separate range indexed by the peer request sequence.
    std::map<uint32_t, std::unique_ptr<DtlsPostHandshakeExchange>> postHandshakeExchanges;
    static constexpr uint32_t AUTHENTICATION_RESPONSE_BASE = 0x10000;
    static bool isOutgoingExchange(uint32_t key) { return key < 256 || key >= AUTHENTICATION_RESPONSE_BASE; }
    struct AuthenticationRequest {
        std::vector<uint8_t> context;
        uint8_t expectedType = 11;
        std::string identity;
    };
    std::optional<AuthenticationRequest> authenticationRequest;
    std::optional<uint32_t> authenticationOutput;
    bool peerCertificateAuthenticated = false;
    std::string authenticatedPeerIdentity;
    bool authenticationActive() const;
    bool permitsOutput(const std::shared_ptr<DtlsHandshakeFlight>& value) const;
    size_t retainedPostHandshakeBytes() const;
    bool requestClientAuthentication();
    void receiveAuthentication(uint64_t epoch, uint64_t number, Packet *body);
    void applyAuthentication(uint8_t type, uint16_t sequence);
    void resumeDeferredOutput();
    void suspendHeartbeatForAuthentication();
    void resumeHeartbeatAfterAuthentication();
    DtlsPostHandshakeExchange& keyUpdate() { return *postHandshakeExchanges.at(24); }
    bool updateRequested = false;
    bool awaitingPeerUpdate = false;
    bool peerResponseDue = false;
    bool closing = false;
    simtime_t retransmissionTimeout = 1;
    bool buildingFlight = false;
    bool pumpingFlight = false;
    size_t generationPending = 0;
    size_t burstQueued = 0;
    size_t burstLimit = 0;
    uint64_t nextEpoch0Record = 0;
    std::set<std::pair<uint64_t, uint64_t>> pendingAcks;
    AssociationId id = 0;
    AssociationId replacementId = 0;
    IDtls::ICallback *callback = nullptr;
    L3Address peerAddress;
    int peerPort = -1;
    L3Address localAddress;
    int localPort = -1;
    int localInterfaceId = -1;
    struct CandidatePath {
        DtlsPath path;
        std::vector<uint8_t> peerCid;
        bool reboundSocket = false;
    };
    std::optional<CandidatePath> candidatePath;
    DtlsPath currentPath() const;
    DtlsPath receivedPath(const Packet *packet) const;
    bool isNewerRecord(uint64_t epoch, uint64_t number) const;
    bool considerPath(const DtlsPath& path, bool fresh);
    void tryStartPathValidation();
    void finishPathValidation(bool success);
    bool isRebinding() const { return candidatePath && candidatePath->reboundSocket; }
    const std::vector<uint8_t>& cidForPath(const DtlsPath& path) const;
    bool hasMigrationCid() const;
    std::vector<uint8_t> takeMigrationCid();
    struct PathMtuDiscovery {
        DtlsPathMtuState state;
        std::unique_ptr<cMessage> timer = std::make_unique<cMessage>("dtlsPathMtuSchedule");
        bool probeDue = false;
        std::optional<simtime_t> raiseAt;
        std::optional<simtime_t> pausedAt;
        std::optional<simtime_t> pausedTimerRemaining;
        PathMtuDiscovery(size_t base, size_t ceiling) : state(base, ceiling) {}
    };
    std::unique_ptr<PathMtuDiscovery> pathMtu;
    int64_t udpPayloadBudget() const;
    bool isPathUnavailable() const { return pathMtu && !pathMtu->state.isAvailable(); }
    void startPathMtuDiscovery();
    void resetPathMtuDiscovery(size_t previousMaximum);
    void tryStartPathMtuProbe();
    void finishPathMtuProbe(IDtls::ProbeResult result, size_t size, std::optional<size_t> hint);
    void schedulePathMtuProbe();
    void handlePathMtuTimer();
    bool prepareFlight(const std::shared_ptr<DtlsHandshakeFlight>& value);
    void notifyPathMtu(size_t previousMaximum, bool confirmed = false);
    void handlePacketTooBig(const Packet *quote, int mtu);
    HandshakeState state = HandshakeState::CLIENT_WAIT_SERVER_HELLO;
    tls::TlsHandshakeParameters::Offer offer;
    tls::TlsHandshakeParameters::Selection selection;
    bool cidOffered = false;
    bool cidNegotiated = false;
    std::vector<uint8_t> localCid;
    std::vector<uint8_t> peerCid;
    std::deque<std::vector<uint8_t>> sparePeerCids;
    std::vector<std::vector<uint8_t>> pendingLocalCids;
    bool pendingCidsImmediate = false;
    bool cidRequestUnfulfilled = false;
    std::optional<uint8_t> deferredPeerCidRequest;
    std::map<uint8_t, std::pair<uint16_t, std::vector<uint8_t>>> lastPeerCidMessages;
    bool heartbeatNegotiated = false;
    bool peerAllowsHeartbeat = false;
    enum class ProbePurpose { DIAGNOSTIC = 0, PATH_VALIDATION = 1, PATH_MTU = 2 };
    struct HeartbeatProbe {
        std::vector<uint8_t> payload;
        DtlsPath path;
        ProbePurpose purpose = ProbePurpose::DIAGNOSTIC;
        size_t udpBytes = 0;
        bool queued = false;
        std::optional<simtime_t> pausedDeadline;
        std::optional<simtime_t> pausedRetry;
        bool suspended = false;
        unsigned transmissions = 0;
        struct Handoff {
            uint64_t epoch;
            uint64_t number;
            std::vector<uint8_t> cid;
        };
        std::optional<Handoff> lastHandoff;
        simtime_t retryTimeout = 1;
        std::unique_ptr<cMessage> retryTimer;
        std::unique_ptr<cMessage> deadline;
    };
    std::unique_ptr<HeartbeatProbe> probe;
    bool startHeartbeatProbe(size_t udpBytes, ProbePurpose purpose = ProbePurpose::DIAGNOSTIC);
    void pumpHeartbeatProbe();
    void finishHeartbeatProbe(IDtls::ProbeResult result, std::optional<size_t> hint = {});
    void receiveHeartbeat(uint64_t epoch, uint64_t number, Packet *body, const DtlsPath& path, bool fresh, bool changed, size_t receivedBytes);
    bool sendHeartbeat(const Ptr<DtlsHeartbeat>& body, bool request, const DtlsPath& path, size_t replyLimit = 0);
    Ptr<DtlsHeartbeat> makeHeartbeat(uint8_t type, const std::vector<uint8_t>& payload, size_t padding);
    bool requestConnectionIds(int count);
    bool provideConnectionIds(int count, bool immediate);
    bool activateSpareConnectionId();
    void receiveConnectionIdMessage(uint64_t epoch, uint64_t number, Packet *body);
    void applyConnectionIdMessage(uint8_t type, uint16_t sequence);
    void finishConnectionIdUpdate();
    void serveConnectionIdRequest(uint8_t count);
    size_t recordOverheadForCid(const std::vector<uint8_t>& cid) const { return isDtls12() ? 37 + (cidNegotiated && !cid.empty() ? 1 + cid.size() : 0) : 22 + (cidNegotiated ? cid.size() : 0); }
    size_t recordOverhead() const { return recordOverheadForCid(peerCid); }
    size_t maximumRecordContent() const { return isDtls12() && cidNegotiated && !peerCid.empty() ? 16383 : 16384; }
    Ptr<DtlsCiphertextHeader> makeRecordHeader(uint64_t epoch, uint64_t number, uint16_t length, const std::vector<uint8_t> *cidOverride = nullptr) const;
    std::optional<DtlsSessionStore::Entry> offeredSession;
    bool earlyRequested = false;
    bool earlyOffered = false;
    bool earlyAccepted = false;
    bool earlyStatusReported = false;
    uint64_t earlySentBytes = 0;
    uint64_t earlyReceivedBytes = 0;
    uint64_t nextEarlyRecord = 0;
    std::unique_ptr<DtlsRecordState> receiveEarly;
    std::map<uint64_t, uint64_t> retainedSendCounters;
    DtlsPostHandshakeExchange *findPostHandshakeExchange(const std::shared_ptr<DtlsHandshakeFlight>& value);
    void issueTicket();
    bool startPostHandshake(uint8_t type, const Ptr<Chunk>& body, const char *name);
    bool startPostHandshakeFlight(uint32_t key, const std::vector<std::pair<uint8_t, Ptr<Chunk>>>& messages, const char *name,
                                 const std::vector<uint8_t>& context = {});
    void pumpPostHandshake(uint32_t key);
    void postHandshakeRecordHandedOff(const std::shared_ptr<DtlsHandshakeFlight>& value, size_t index, uint64_t number);
    void pruneRetainedSendCounters();
    void receiveTicket(uint64_t epoch, uint64_t number, Packet *body);
    bool allocateRecord(uint64_t epoch, uint64_t& number);
    std::unique_ptr<DtlsRecordState> receiveEpoch2;
    std::unique_ptr<DtlsRecordState> receiveEpoch3;
    std::unique_ptr<DtlsRecordState> previousReceiveEpoch;
    uint64_t nextEpoch2Record = 0;
    uint64_t nextApplicationRecord = 0;
    uint64_t sendEpoch = 3;
    uint64_t receiveEpoch = 3;
    uint64_t previousReceiveEpochNumber = 0;
    uint16_t nextLocalMessage = 0;
    bool localMessagesExhausted = false;
    uint16_t lastPeerUpdateMessage = 0;
    uint64_t lastPeerUpdateEpoch = 0;
    uint8_t lastPeerUpdateFlag = 0;
    bool hasPeerUpdate = false;
    size_t receivedBytes = 0;
    size_t sentBytes = 0;
    size_t reservedBytes = 0;
    bool serverFlightQueued = false;
    bool clientAuthRequested = false;
    bool ownFinishedHandedOff = false;
    bool peerFinishedProcessed = false;
    uint64_t peerFinishedRecord = 0;
    bool addressValidated = false;
    bool serverSide = false;
    bool retried = false;
    bool retryRequestedShare = false;
    uint16_t retryGroup = 0;
    uint16_t retryCipherSuite = 0;
    uint16_t clientAuthSignature = tls::TlsHandshakeParameters::ECDSA_P256_SHA256;
    std::vector<uint8_t> retryCookie;
    std::vector<uint8_t> pendingServerHello;
    uint16_t pendingServerHelloSequence = 0;
    std::array<uint8_t, 32> clientRandom{};
    std::vector<uint8_t> clientShare;
    uint16_t serverMessageBase = 0;
    uint32_t nextPeerMessage = 0;
    uint32_t initialPeerMessageEnd = 0;
    bool processingHandshake = false;
    uint8_t processingHandshakeType = 0;
    std::vector<uint8_t> processingHandshakeBody;
    std::map<uint16_t, uint64_t> completeRecordNumbers;
    std::map<uint16_t, DtlsHandshakeReassembly::Message> processedHandshakes;
    struct PendingPacket {
        std::unique_ptr<Packet> packet;
        std::optional<simtime_t> delay;
        bool paid = false;
    };
    std::deque<PendingPacket> pendingPackets;
    size_t pendingBytes = 0;
    std::set<long> processingPacketIds;
    simtime_t startedAt = SIMTIME_ZERO;
    std::unique_ptr<cMessage> handshakeDeadline;

    void processHandshake(uint64_t sequence, Packet *body);
    void receiveHandshakeFragment(uint64_t recordSequence, Packet *body);
    bool retainHandshakeFragment(uint64_t recordSequence, Packet *body, bool& immediateAck, bool& duplicate, bool& implicitResponse);
    void scheduleNextHandshake();
    static int validateClientHello(const Ptr<const DtlsClientHello>& hello, bool certificateProfile, const std::string& identity,
                                   tls::TlsHandshakeParameters::Offer& offer, tls::TlsHandshakeParameters::Selection& selection,
                                   const DtlsSessionStore *sessions = nullptr,
                                   const tls::TlsHandshakeParameters::Preferences& preferences = tls::TlsHandshakeParameters::Preferences{});
    void processServerHello(uint16_t messageSequence, Packet *body, const L3Address& localAddress, int localPort);
    void sendServerFlight();
    void sendClientHello(const std::vector<uint8_t>& cookie);
    void serverFinishedHandedOff();
    void beginFlight();
    void appendHandshake(uint8_t type, uint16_t messageSequence, uint64_t epoch, const Ptr<Chunk>& body, bool establishAfterSend = false, bool serverFinished = false);
    void finishFlight();
    void pumpFlight();
    void flightRecordHandedOff(const std::shared_ptr<DtlsHandshakeFlight>& sentFlight, size_t index, uint64_t recordNumber);
    void flightRecordSkipped(const std::shared_ptr<DtlsHandshakeFlight>& sentFlight, size_t index, size_t reservedCredit);
    void acceptAck(const DtlsAck& ack);
    void acknowledgeFlightImplicitly();
    void noteHandshakeRecord(uint64_t epoch, uint64_t recordNumber, bool immediate);
    void flushAcknowledgements(uint64_t sendEpoch);
    void clearAcknowledgements();
    bool requestUpdate(bool requestPeerUpdate);
    void pumpUpdate();
    void updateRecordHandedOff(const std::shared_ptr<DtlsHandshakeFlight>& sentFlight, size_t index, uint64_t recordNumber);
    void updateRecordSkipped(const std::shared_ptr<DtlsHandshakeFlight>& sentFlight, size_t index);
    void receiveKeyUpdate(uint64_t epoch, uint64_t recordSequence, Packet *body);
    void applyKeyUpdate(uint64_t epoch, uint64_t recordSequence, uint16_t messageSequence, uint8_t flag, bool satisfiesRequest);
    bool allocateApplicationRecord(uint64_t& number);
    bool allocatePlaintextRecord(uint64_t& number);
    bool advanceSendEpoch();
    bool handleTimer(cMessage *message);
    void cancelTimers();
    // Retained-message count for derived association diagnostics.
    [[nodiscard]] size_t getReassemblyEntryCount() const { return reassembly.getEntryCount(); }

  public:
    explicit DtlsAssociation(Dtls& engine, size_t reassemblyLimit);
    [[nodiscard]] size_t getReassemblyBytes() const { return reassembly.getReservedBytes(); }
};

} // namespace inet::dtls
#endif
