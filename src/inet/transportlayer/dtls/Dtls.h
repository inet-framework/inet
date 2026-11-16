// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLS_H
#define INET_DTLS_H

#include <map>
#include <memory>

#include "inet/common/Protocol.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/transportlayer/contract/dtls/IDtls.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/transportlayer/dtls/DtlsProcessingQueue.h"
#include "inet/transportlayer/dtls/DtlsAssociation.h"

namespace inet::dtls {

extern INET_API const Protocol dtlsProtocol;

/** Datagram security engine. All protection is modeled; no cryptography is performed. */
class INET_API Dtls : public OperationalBase, public UdpSocket::ICallback, public IDtls
{
    friend class DtlsAssociation;
  protected:
    using ICallback = IDtls::ICallback;
    enum DropReason { MALFORMED_RECORD = 1, UNKNOWN_EPOCH = 2, REPLAY = 3, RESOURCE_LIMIT = 4, UNSUPPORTED_PATH = 5, UNKNOWN_CID = 6 };
    using Association = DtlsAssociation;
    using HandshakeState = DtlsAssociation::HandshakeState;
    tls::TlsHandshakeParameters::Preferences preferences;
    bool allowDtls13 = true;
    bool allowDtls12 = false;

    struct PrevalidationAttempt {
        uint64_t id = 0;
        tls::TlsHandshakeParameters::Selection selection;
        L3Address localAddress;
        int localPort;
        L3Address peerAddress;
        int peerPort;
        DtlsHandshakeReassembly reassembly;
        std::unique_ptr<cMessage> deadline;
        std::unique_ptr<cMessage> retryTimer;
        std::shared_ptr<DtlsHandshakeFlight> retryFlight;
        simtime_t retryTimeout = 1;
        size_t retryBurst = 0;
        size_t retryQueued = 0;
        bool retryGenerating = false;
        bool retryPumping = false;
        bool processingHello = false;
        size_t receivedBytes = 0;
        size_t sentBytes = 0;
        size_t reservedBytes = 0;
        uint64_t nextRecord = 0;
        std::vector<uint8_t> cookie;
        std::vector<uint8_t> initialHello;
        std::vector<uint8_t> pendingInitialHello;
        bool requestKeyShare = false;
        bool failing = false;
        explicit PrevalidationAttempt(size_t limit) : reassembly(limit), deadline(std::make_unique<cMessage>("dtlsPrevalidationDeadline")), retryTimer(std::make_unique<cMessage>("dtlsRetryRetransmission")) {}
    };

    UdpSocket udpSocket;
    ICallback *listener = nullptr;
    std::map<AssociationId, std::unique_ptr<Association>> associations;
    std::vector<std::unique_ptr<PrevalidationAttempt>> prevalidationAttempts;
    AssociationId nextAssociationId = 1;
    DtlsSessionStore sessions;
    uint64_t nextTicketSerial = 1;
    uint64_t nextConnectionId = 1;
    uint64_t nextHeartbeatToken = 1;
    uint64_t nextAuthenticationContext = 1;
    bool startHeartbeatProbe(AssociationId id, size_t udpBytes = 0);
    static bool sameTuple(const DtlsPath& a, const DtlsPath& b);
    int64_t initialPayloadBudget() const;
    void sendUdp(Packet *packet, const L3Address& peerAddress, int peerPort);
    std::map<std::vector<uint8_t>, AssociationId> cidIndex;
    std::vector<uint8_t> allocateConnectionId(AssociationId id);
    void removeConnectionIds(AssociationId id);
    size_t countConnectionIds(AssociationId id) const;
    std::unique_ptr<DtlsProcessingQueue> inputProcessing;
    std::unique_ptr<DtlsProcessingQueue> outputProcessing;

    static simsignal_t packetSentToLowerSignal;
    static simsignal_t packetReceivedFromLowerSignal;
    static simsignal_t packetSentToUpperSignal;
    static simsignal_t recordAcceptedSignal;
    static simsignal_t recordDroppedSignal;
    static simsignal_t associationEstablishedSignal;
    static simsignal_t associationFailedSignal;
    static simsignal_t establishmentTimeSignal;
    static simsignal_t recordOverheadSignal;
    static simsignal_t reassemblyBytesSignal;
    static simsignal_t handshakeRetransmissionSignal;
    static simsignal_t processingDelaySignal;

  protected:
    bool isInitializeStage(int stage) const override { return stage == INITSTAGE_APPLICATION_LAYER; }
    bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_APPLICATION_LAYER; }
    bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_APPLICATION_LAYER; }
    void handleMessageWhenUp(cMessage *message) override;
    void handleStartOperation(LifecycleOperation *operation) override;
    void handleStopOperation(LifecycleOperation *operation) override;
    void handleCrashOperation(LifecycleOperation *operation) override;
    void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    void socketClosed(UdpSocket *socket) override;
    void discardAll(const char *reason, bool notify);
    void parseDatagram(Packet *packet);
    bool receiveRecord12(Packet *packet, const DtlsPath& path, size_t datagramBytes, const std::shared_ptr<bool>& creditCounted);
    bool handleClientHello12(PrevalidationAttempt *attempt, const Ptr<const DtlsClientHello>& hello, uint16_t sequence, const std::vector<uint8_t>& encoded);
    bool isProcessedHello12(const DtlsPath& path, uint16_t sequence, const std::vector<uint8_t>& encoded) const;
    void sendRetry12(PrevalidationAttempt *attempt, const Ptr<const DtlsClientHello>& hello, const std::vector<uint8_t>& encoded);
    void requestCookieFromPrefix12(PrevalidationAttempt *attempt);
    static std::vector<uint8_t> makeCookie(uint64_t issued, uint64_t started, const L3Address& local, int localPort, const L3Address& peer, int peerPort, const Ptr<const DtlsClientHello>& hello);
    static uint64_t readUint64(const std::vector<uint8_t>& input, size_t offset);
    void sendClientHello(AssociationId id, const std::vector<uint8_t>& cookie = {});
    void sendRetry(const L3Address& localAddress, const L3Address& peerAddress, int peerPort, const std::vector<uint8_t>& cookie, bool requestKeyShare, size_t receivedBytes, PrevalidationAttempt *attempt, uint16_t messageSequence);
    void pumpRetry(uint64_t id);
    void acknowledgeRetry(uint64_t id, const DtlsAck& ack);
    PrevalidationAttempt *findPrevalidation(uint64_t id) const;
    size_t countHalfOpen() const;
    bool handleHandshakeDeadline(cMessage *message);
    void handlePlainHandshake(Packet *body, const L3Address& localAddress, int localPort, const L3Address& peerAddress, int peerPort, size_t datagramBytes, const std::shared_ptr<bool>& creditCounted, uint64_t recordSequence, bool acknowledgePartial = true, bool recordChargePending = false);
    void handlePlainHandshakeRecord(Packet *body, const L3Address& localAddress, int localPort, const L3Address& peerAddress, int peerPort, size_t datagramBytes, const std::shared_ptr<bool>& creditCounted, uint64_t recordSequence);
    void acknowledgePrevalidation(PrevalidationAttempt *attempt, uint64_t recordSequence);
    void failPrevalidation(PrevalidationAttempt *attempt, uint8_t alertDescription);
    void removePrevalidation(uint64_t id);
    void emitReassemblyBytes();
    void handleProtectedHandshake(AssociationId id, uint16_t epoch, uint16_t truncatedSequence, unsigned sequenceBits, Packet *body, size_t recordBytes);
    void handleAvailableProtectedHandshake(AssociationId id, uint16_t epoch, uint16_t truncatedSequence, unsigned sequenceBits, Packet *body, size_t recordBytes);
    void sendServerFlight(AssociationId id);
    Ptr<Chunk> makeCertificate(const std::vector<uint8_t>& context = {}) const;
    Ptr<Chunk> makeCertificateRequest(const std::vector<uint8_t>& context = {}) const;
    int checkCertificateRequest(Packet *body, bool postHandshake, std::vector<uint8_t>& context, uint16_t& signature) const;
    Ptr<Chunk> makeCertificateVerify(uint16_t scheme) const;
    int checkCertificate(Packet *body, const std::vector<uint8_t>& context = {}) const;
    int checkCertificateVerify(Packet *body, uint16_t& selectedScheme) const;
    void sendFinalAck(AssociationId id, uint64_t sequence);
    void sendAcknowledgements(AssociationId id, uint64_t sendEpoch, const std::vector<std::pair<uint64_t, uint64_t>>& records);
    void sendApplication(AssociationId id, Packet *packet, std::optional<simtime_t> sampledDelay = {}, bool paid = false);
    bool holdApplication(AssociationId id, std::unique_ptr<Packet> packet, std::optional<simtime_t> sampledDelay = {}, bool paid = false);
    void sendPlainHandshake(AssociationId id, uint8_t messageType, uint16_t messageSequence, const Ptr<Chunk>& body);
    void sendProtectedHandshake(AssociationId id, uint8_t messageType, uint16_t messageSequence, const Ptr<Chunk>& body, bool establishAfterSend = false, bool serverFinished = false);
    void transmitFlightFragment(AssociationId id, const std::shared_ptr<DtlsHandshakeFlight>& flight, size_t index);
    void establish(AssociationId id);
    void failAssociation(AssociationId id, const char *reason, int alertDescription = -1);
    void finishClose(AssociationId id, bool forced);
    void beginClose(AssociationId id);
    void resumeApplications(AssociationId id);
    void sendAlert(const L3Address& localAddress, const L3Address& peerAddress, int peerPort, uint8_t description, bool encrypted, uint64_t sequence = 0, uint64_t epoch = 2,
                   const std::vector<uint8_t>& cid = {}, int interfaceId = -1, bool legacy = false);
    bool enqueueProcessing(DtlsProcessingQueue *queue, AssociationId id, size_t bytes, simtime_t delay, DtlsProcessingQueue::Completion completion,
                           DtlsProcessingQueue::Eligibility eligible = {}, DtlsProcessingQueue::Completion skipped = {},
                           DtlsProcessingQueue::Completion started = {}, bool recordDelay = true);
    bool containsAssociation(AssociationId id) const { return associations.find(id) != associations.end(); }
    static bool hasConsistentEncoding(const Packet *packet);
    void reportEarlyData(AssociationId id, bool accepted);
    simtime_t processingDelay(AssociationId id, const char *operation);

  public:
    ~Dtls() override;
    void listen(ICallback *callback) override;
    AssociationId open(const L3Address& peerAddress, int peerPort, ICallback *callback) override;
    AssociationId open(const L3Address& peerAddress, int peerPort, ICallback *callback, bool resumeSession) override;
    AssociationId open(const L3Address& peerAddress, int peerPort, ICallback *callback, const DtlsOpenOptions& options) override;
    bool isResumed(AssociationId id) const override;
    void send(AssociationId id, Packet *packet, ICallback *requester) override;
    void sendEarly(AssociationId id, Packet *packet, ICallback *requester) override;
    void close(AssociationId id) override;
    bool requestKeyUpdate(AssociationId id, bool requestPeerUpdate) override;
    bool requestConnectionIds(AssociationId id, int count) override;
    bool provideConnectionIds(AssociationId id, int count, bool immediate) override;
    bool activateSpareConnectionId(AssociationId id) override;
    bool migrateLocal(AssociationId id, const L3Address& address, int interfaceId) override;
    bool rebindLocalPort(AssociationId id, int port) override;
    bool requestClientAuthentication(AssociationId id) override;
    bool getPeerAuthentication(AssociationId id, DtlsPeerAuthentication& result) const override;
    size_t getMaximumDatagramSize(AssociationId id) const override;
};

} // namespace inet::dtls
#endif
