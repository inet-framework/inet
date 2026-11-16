//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_COAP_COAPENDPOINTBASE_H
#define __INET_COAP_COAPENDPOINTBASE_H

#include <cstdint>
#include <deque>
#include <map>
#include <set>
#include <vector>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/applications/coap/CoapHeader_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {
namespace coap {

/**
 * Comparable remote UDP transport endpoint key for CoAP exchanges.
 */
struct INET_API CoapPeerEndpoint
{
    L3Address address;
    int port = -1;

    bool operator==(const CoapPeerEndpoint& other) const {
        return port == other.port && address == other.address;
    }

    bool operator!=(const CoapPeerEndpoint& other) const {
        return !(*this == other);
    }

    bool operator<(const CoapPeerEndpoint& other) const {
        if (port != other.port)
            return port < other.port;
        return address < other.address;
    }
};

/**
 * Base module for CoAP over UDP endpoints (RFC 7252).
 * Handles UDP socket binding, raw/typed datagram validation, token & MID
 * allocation, duplicate suppression, request-response matching, and lifecycle.
 */
class INET_API CoapEndpointBase : public ApplicationBase, public UdpSocket::ICallback
{
  protected:
    struct CoapDuplicateKey {
        CoapPeerEndpoint peer;
        uint16_t messageId = 0;

        bool operator<(const CoapDuplicateKey& other) const {
            if (messageId != other.messageId)
                return messageId < other.messageId;
            return peer < other.peer;
        }
    };

    struct CoapDuplicateEntry {
        CoapDuplicateKey key;
        simtime_t expiry = SIMTIME_ZERO;
        bool isConfirmable = false;
        bool hasCachedResponse = false;
        std::vector<uint8_t> cachedWireBytes;
    };

    struct CoapPendingRequest {
        uint64_t requestId = 0;
        CoapPeerEndpoint peer;
        bool peerIsMulticast = false;
        bool confirmable = false;
        uint8_t requestCode = COAP_CODE_EMPTY;
        std::vector<uint8_t> token;
        uint16_t initialMid = 0;
        std::set<uint16_t> sentMids;
        simtime_t createdAt = SIMTIME_ZERO;
        simtime_t expiresAt = SIMTIME_ZERO;
    };

    struct CoapOutboundItem {
        uint64_t outboundId = 0;
        CoapPeerEndpoint peer;
        bool peerIsMulticast = false;
        bool confirmable = false;
        bool isRequest = false;
        uint64_t requestId = 0;
        std::vector<uint8_t> token;
        std::string packetName;
        Ptr<CoapHeader> header;
        std::vector<uint8_t> payload;
    };

    struct CoapOutboundExchange {
        uint64_t exchangeId = 0;
        CoapPeerEndpoint peer;
        uint16_t messageId = 0;
        bool isRequest = false;
        uint64_t requestId = 0;
        std::vector<uint8_t> token;
        std::string packetName;
        std::vector<uint8_t> wireBytes;
        simtime_t initialTimeout = SIMTIME_ZERO;
        simtime_t currentTimeout = SIMTIME_ZERO;
        simtime_t nextRetryAt = SIMTIME_ZERO;
        int retransmitCount = 0;
    };

    struct CoapPeerCongestionState {
        int activeConfirmables = 0;
        bool inProbingBackoff = false;
        simtime_t probingEarliestAt = SIMTIME_ZERO;
    };

  protected:
    // Cached NED parameters
    L3Address localAddress;
    int localPort = -1;
    int timeToLive = -1;
    int dscp = -1;
    int tos = -1;
    int rngStream = 0;

    simtime_t ackTimeout = 2.0;
    double ackRandomFactor = 1.5;
    int maxRetransmit = 4;
    int nstart = 1;
    double probingRateBytesPerSec = 1.0;
    simtime_t maxLatency = 100.0;
    simtime_t processingDelay = 2.0;

    simtime_t maxTransmitSpan = 45.0;
    simtime_t maxTransmitWait = 93.0;
    simtime_t exchangeLifetime = 247.0;
    simtime_t nonLifetime = 145.0;

    int maxPendingRequests = 64;
    int maxPendingOutboundPerEndpoint = 64;
    int maxIncomingBlockSize = 65536;
    int preferredBlockSize = 1024;
    int maxIncomingBlockTransfers = 16;
    int maxObservers = 64;

    // Socket & single reusable endpoint deadline timer
    UdpSocket socket;
    cMessage *deadlineTimer = nullptr;

    // Cryptographic / uniqueness counters surviving crash & restart
    bool generatorsInitialized = false;
    uint64_t tokenCounter = 0;
    uint64_t tokenRemaining = 0;
    uint16_t nextMessageId = 0;
    std::deque<std::pair<uint16_t, simtime_t>> midReservationOrder;
    std::map<uint16_t, simtime_t> reservedOutgoingMids;
    bool midPoolExhausted = false;

    // Transient operational state cleared on stop / crash
    uint64_t nextRequestId = 1;
    uint64_t nextOutboundId = 1;
    uint64_t nextExchangeId = 1;
    std::map<std::vector<uint8_t>, CoapPendingRequest> pendingRequests;
    std::map<std::pair<CoapPeerEndpoint, uint16_t>, std::vector<uint8_t>> outboundMidToToken;
    std::map<std::pair<CoapPeerEndpoint, uint16_t>, CoapOutboundExchange> activeExchanges;
    std::map<CoapPeerEndpoint, CoapPeerCongestionState> peerCongestion;
    std::deque<CoapOutboundItem> outboundQueue;
    std::deque<CoapDuplicateKey> conDuplicateOrder;
    std::deque<CoapDuplicateKey> nonDuplicateOrder;
    std::map<CoapDuplicateKey, CoapDuplicateEntry> duplicateEntries;

    // Statistics signals
    static simsignal_t retransmissionSignal;
    static simsignal_t duplicateReceivedSignal;
    static simsignal_t malformedReceivedSignal;
    static simsignal_t capacityRejectedSignal;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    // Lifecycle hooks
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    // UdpSocket::ICallback
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    // Initialization & socket setup
    virtual void parseAndValidateParameters();
    virtual void initializeCountersOnce();
    virtual void bindSocket();
    virtual void clearTransientState(bool clearDuplicates);

    // Timer management
    virtual void rescheduleDeadlineTimer();
    virtual simtime_t getNextSubclassDeadline() const { return SIMTIME_ZERO; }
    virtual void onDeadlineTimer();
    virtual void onSubclassTimer(simtime_t now) {}
    virtual void handleSubclassSelfMessage(cMessage *msg);
    virtual void onEndpointStart() {}
    virtual void onEndpointStop() {}
    virtual void onEndpointCrash() {}

    // Allocation helpers & test seams
    virtual bool allocateToken(std::vector<uint8_t>& tokenOut);
    virtual bool allocateMessageId(uint16_t& midOut, bool emitOnFailure = true);
    void configureTokenGeneratorForTest(uint64_t initialValue, uint64_t remainingCount);
    void configureMidGeneratorForTest(uint16_t initialMid, bool reserveAll = false, simtime_t reserveDuration = SIMTIME_ZERO);
    void purgeExpiredMidReservations(simtime_t now);

    // Packet construction, parsing, and transmission
    Packet *buildCoapPacket(const char *name, const Ptr<CoapHeader>& header, const std::vector<uint8_t>& payload) const;
    bool parseInboundDatagram(Packet *packet, Ptr<CoapHeader>& headerOut, std::vector<uint8_t>& payloadOut) const;
    virtual void sendCoapPacket(Packet *packet, const L3Address& destAddr, int destPort);
    void sendEmptyMessage(CoapMessageType type, uint16_t messageId, const L3Address& destAddr, int destPort);
    void sendEmptyMessageAndCacheDuplicate(CoapMessageType type, uint16_t messageId, const CoapPeerEndpoint& peer);
    bool submitOutboundMessage(const CoapPeerEndpoint& peer, bool peerIsMulticast, bool confirmable,
            bool isRequest, uint64_t requestId, const std::vector<uint8_t>& token, const char *packetName,
            const Ptr<CoapHeader>& header, const std::vector<uint8_t>& payload);
    bool sendConfirmablePing(const CoapPeerEndpoint& peer, uint64_t& exchangeIdOut);
    void pumpOutboundQueue(simtime_t now);
    void processConfirmableRetransmissions(simtime_t now);
    void markPeerReachable(const CoapPeerEndpoint& peer);

    // Deduplication
    void purgeExpiredDuplicates(simtime_t now);
    bool checkAndHandleDuplicate(const CoapPeerEndpoint& peer, uint16_t messageId, CoapMessageType type);
    void recordDuplicateEntry(const CoapPeerEndpoint& peer, uint16_t messageId, bool isConfirmable, simtime_t lifetime);
    void cacheDuplicateResponse(const CoapPeerEndpoint& peer, uint16_t messageId, Packet *responsePacket);

    // Request-response matching & virtual handlers
    bool registerPendingRequest(const CoapPeerEndpoint& peer, bool peerIsMulticast, bool confirmable,
            uint8_t requestCode, const std::vector<uint8_t>& token, uint64_t& requestIdOut);
    void removePendingRequest(const std::vector<uint8_t>& token);
    void expirePendingRequests(simtime_t now);
    void cancelConfirmableExchangesForRequest(uint64_t requestId);

    virtual bool isRecognizedRequestOption(uint16_t optionNumber) const;
    virtual bool isRecognizedResponseOption(uint16_t optionNumber) const;
    bool findUnrecognizedCriticalOption(const CoapHeader& header, bool isRequest, uint16_t& badOptionOut) const;

    virtual void handleInboundRequest(const CoapPeerEndpoint& peer, bool destIsMulticast,
            const Ptr<const CoapHeader>& header, const std::vector<uint8_t>& payload);
    virtual void handleMatchedResponse(const CoapPendingRequest& request, const CoapPeerEndpoint& peer,
            const Ptr<const CoapHeader>& header, const std::vector<uint8_t>& payload);
    virtual void handleEmptyAck(const CoapPeerEndpoint& peer, uint16_t messageId);
    virtual void handleReset(const CoapPeerEndpoint& peer, uint16_t messageId);
    virtual void onOutboundExchangeFailed(uint64_t exchangeId, const CoapPeerEndpoint& peer, uint16_t messageId, const char *reason) {}
    virtual void onRequestCompleted(uint64_t requestId, const CoapPeerEndpoint& peer,
            const Ptr<const CoapHeader>& responseHeader, const std::vector<uint8_t>& payload) {}
    virtual void onRequestFailed(uint64_t requestId, const char *reason, uint8_t coapCode = 0) {}
    virtual void onRequestAborted(uint64_t requestId) {}

  public:
    CoapEndpointBase() = default;
    virtual ~CoapEndpointBase();
};

} // namespace coap
} // namespace inet

#endif
