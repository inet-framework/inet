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
#include "inet/transportlayer/tls/TlsHandshakeParameters.h"

namespace inet::dtls {

class Dtls;
class DtlsClientHello;
class DtlsAck;

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
    static constexpr std::array<uint8_t, 32> HELLO_RETRY_RANDOM = {
        0xcf, 0x21, 0xad, 0x74, 0xe5, 0x9a, 0x61, 0x11, 0xbe, 0x1d, 0x8c, 0x02, 0x1e, 0x65, 0xb8, 0x91,
        0xc2, 0xa2, 0x11, 0x16, 0x7a, 0xbb, 0x8c, 0x5e, 0x07, 0x9e, 0x09, 0xe2, 0xc8, 0xa8, 0x33, 0x9c
    };
    Dtls& engine;
    DtlsHandshakeReassembly reassembly;
    std::shared_ptr<DtlsHandshakeFlight> flight;
    std::unique_ptr<cMessage> retransmissionTimer;
    std::unique_ptr<cMessage> delayedAckTimer;
    std::unique_ptr<cMessage> retentionTimer;
    std::unique_ptr<cMessage> closeDeadline;
    // RFC 9147 Section 5.8.4: independent recovery owners, keyed by handshake type.
    std::map<uint8_t, std::unique_ptr<DtlsPostHandshakeExchange>> postHandshakeExchanges;
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
    HandshakeState state = HandshakeState::CLIENT_WAIT_SERVER_HELLO;
    tls::TlsHandshakeParameters::Offer offer;
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
    bool certificateProfile = false;
    bool clientAuthRequested = false;
    bool ownFinishedHandedOff = false;
    bool peerFinishedProcessed = false;
    uint64_t peerFinishedRecord = 0;
    bool addressValidated = false;
    bool serverSide = false;
    bool retried = false;
    bool retryRequestedShare = false;
    std::vector<uint8_t> retryCookie;
    std::vector<uint8_t> pendingServerHello;
    uint16_t pendingServerHelloSequence = 0;
    std::array<uint8_t, 32> clientRandom{};
    std::array<uint8_t, 32> clientShare{};
    uint16_t serverMessageBase = 0;
    uint16_t nextPeerMessage = 0;
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
                                   tls::TlsHandshakeParameters::Offer& offer, tls::TlsHandshakeParameters::Selection& selection);
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
