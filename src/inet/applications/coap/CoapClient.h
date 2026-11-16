//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_COAP_COAPCLIENT_H
#define __INET_COAP_COAPCLIENT_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "inet/applications/coap/CoapBlockwiseTransfer.h"
#include "inet/applications/coap/CoapEndpointBase.h"
#include "inet/applications/coap/CoapObservation.h"

namespace inet {
namespace coap {

/**
 * Parsed CoAP URI components derived per RFC 7252 Section 6.4.
 */
struct INET_API CoapParsedUri
{
    bool hasAuthority = false;
    std::string host;
    int port = -1;
    std::vector<std::string> pathSegments;
    std::vector<std::string> querySegments;
};

/**
 * Last observed client response record for diagnostics and protocol test inspection.
 */
struct INET_API CoapClientResponseRecord
{
    uint64_t requestId = 0;
    uint8_t code = 0;
    CoapMessageType messageType = COAP_TYPE_NON;
    int32_t contentFormat = -1;
    int64_t maxAge = -1;
    std::vector<uint8_t> etag;
    std::vector<std::string> locationPath;
    std::vector<std::string> locationQuery;
    std::vector<uint8_t> payload;
    simtime_t latency = SIMTIME_ZERO;
};

struct INET_API CoapClientObserveNotification
{
    uint64_t relationId = 0;
    uint8_t code = 0;
    uint32_t observeValue = 0;
    int32_t contentFormat = -1;
    std::vector<uint8_t> etag;
    std::vector<uint8_t> payload;
    simtime_t receivedAt = SIMTIME_ZERO;
};

/**
 * CoAP over UDP client application issuing configured requests and exposing
 * a protected submission seam for test harnesses and compound transfers.
 */
class INET_API CoapClient : public CoapEndpointBase
{
  protected:
    std::string peerAddressStr;
    int peerPort = 5683;
    std::string requestUri;
    uint8_t requestMethodCode = COAP_CODE_GET;
    bool confirmable = true;
    std::vector<uint8_t> requestBodyBytes;
    int contentFormat = -1;
    int acceptFormat = -1;
    bool observeOption = false;
    bool useRequestTag = false;

    simtime_t startTime = 1.0;
    simtime_t stopTime = -1.0;
    int numRequests = 1;

    cMessage *sendTimer = nullptr;
    int numRequestsSent = 0;

    std::map<uint64_t, simtime_t> activeRequestStarts;
    std::vector<CoapClientResponseRecord> completedResponses;
    int failedCount = 0;
    int abortedCount = 0;

    // RFC 7641 Observe state
    uint64_t nextObserveRelationId = 1;
    uint64_t nextObserveConsumerId = 1;
    std::map<CoapObserverKey, CoapClientObservation> observationsByPeerToken;
    std::map<CoapClientObserveCacheKey, CoapObserverKey> observationsByCacheKey;
    std::map<uint64_t, CoapObserverKey> pendingObserveRegistrations;
    std::map<uint64_t, CoapObserverKey> observeConsumerToRelation;
    std::vector<CoapClientObserveNotification> receivedNotifications;

    // RFC 7959 Blockwise & RFC 9175 Request-Tag state
    uint64_t nextRequestTagCounter = 1;
    std::map<uint64_t, blockwise::CoapClientBlockTransfer> activeBlockTransfers;
    std::map<uint64_t, uint64_t> subRequestToTransfer;
    std::map<blockwise::CoapBlockRequestKey, uint64_t> activeKeyToTransfer;
    std::map<blockwise::CoapBlockRequestKey, std::deque<blockwise::CoapClientBlockTransfer>> queuedBlockTransfers;

    static simsignal_t requestStartedSignal;
    static simsignal_t requestCompletedSignal;
    static simsignal_t requestFailedSignal;
    static simsignal_t requestAbortedSignal;
    static simsignal_t requestLatencySignal;
    static simsignal_t notificationReceivedSignal;
    static simsignal_t activeObservationsSignal;

  protected:
    virtual void parseAndValidateParameters() override;
    virtual simtime_t getNextSubclassDeadline() const override;
    virtual void onSubclassTimer(simtime_t now) override;
    virtual void handleSubclassSelfMessage(cMessage *msg) override;
    virtual void onEndpointStart() override;
    virtual void onEndpointStop() override;
    virtual void onEndpointCrash() override;

    virtual void dispatchMatchedResponse(uint64_t requestId, const CoapPeerEndpoint& peer,
            const Ptr<const CoapHeader>& responseHeader, const std::vector<uint8_t>& payload) override;
    virtual void dispatchRequestFailed(uint64_t requestId, const char *reason, uint8_t coapCode = 0) override;
    virtual void dispatchRequestAborted(uint64_t requestId) override;

    virtual bool handleUnsolicitedResponse(const CoapPeerEndpoint& peer, bool destIsMulticast,
            const Ptr<const CoapHeader>& header, const std::vector<uint8_t>& payload) override;
    virtual void onRequestCompleted(uint64_t requestId, const CoapPeerEndpoint& peer,
            const Ptr<const CoapHeader>& responseHeader, const std::vector<uint8_t>& payload) override;
    virtual void onRequestFailed(uint64_t requestId, const char *reason, uint8_t coapCode = 0) override;
    virtual void onRequestAborted(uint64_t requestId) override;

    /** Invoked when a complete initial response or Observe notification is assembled for an active subscription. */
    virtual void onObserveNotification(const CoapClientObserveNotification& notification) {}
    /** Invoked once when an active or pending Observe relation terminates due to error, reset, cancellation, or lifecycle stop/crash. */
    virtual void onObserveTerminated(uint64_t relationId, uint8_t code, const char *reason) {}

    void scheduleNextSend(simtime_t when);
    uint64_t submitWireExchange(const CoapPeerEndpoint& peer, bool peerMulticast, uint8_t methodCode,
            bool isConfirmable, const CoapParsedUri& parsedUri, const std::vector<uint8_t>& body,
            int reqContentFormat, int reqAccept, const std::vector<CoapOption>& extraOptions,
            const std::vector<uint8_t> *explicitToken);
    bool startClientBlockTransfer(blockwise::CoapClientBlockTransfer& transfer);
    bool sendNextTransferStep(blockwise::CoapClientBlockTransfer& transfer);
    void releaseTransferSlotAndPumpQueue(const blockwise::CoapBlockRequestKey& key);
    void failLogicalTransfer(uint64_t transferId, const char *reason, uint8_t coapCode = 0);
    void completeLogicalTransfer(uint64_t transferId, const CoapPeerEndpoint& peer,
            const Ptr<const CoapHeader>& finalHeader, const std::vector<uint8_t>& finalPayload);
    uint64_t submitCoapRequestInternal(const L3Address& destAddr, int destPort, uint8_t methodCode,
            bool isConfirmable, const std::string& uriStr, const std::vector<uint8_t>& body,
            int reqContentFormat, int reqAccept, const std::vector<CoapOption>& extraOptions,
            const std::vector<uint8_t> *explicitToken, bool bypassObserveReuse);
    uint64_t submitCoapRequest(const L3Address& destAddr, int destPort, uint8_t methodCode,
            bool isConfirmable, const std::string& uriStr, const std::vector<uint8_t>& body,
            int reqContentFormat = -1, int reqAccept = -1, const std::vector<CoapOption>& extraOptions = {});
    uint64_t submitConfiguredRequest();

    static CoapClientObserveCacheKey buildObserveCacheKey(const CoapPeerEndpoint& peer,
            const CoapParsedUri& parsedUri, int reqAccept, const std::vector<CoapOption>& extraOptions);
    void scheduleObserveFreshnessTimer(CoapClientObservation& obs);
    void acceptObserveUpdate(CoapClientObservation& obs, const Ptr<const CoapHeader>& responseHeader,
            const std::vector<uint8_t>& payload, uint32_t observeVal);
    void terminateObservation(const CoapObserverKey& key, uint8_t code, const char *reason);

  public:
    CoapClient() = default;
    virtual ~CoapClient();

    uint64_t subscribeObserve(const L3Address& destAddr, int destPort, const std::string& uriStr,
            bool isConfirmable = true, int reqAccept = -1, const std::vector<CoapOption>& extraOptions = {});
    bool unsubscribeObserve(uint64_t consumerId, bool sendExplicitCancel = true);

    static uint8_t parseMethodName(const std::string& methodName);
    static CoapParsedUri parseCoapUri(const std::string& uriStr);
    static std::string percentDecodeUriComponent(const std::string& input);

    const std::vector<CoapClientResponseRecord>& getCompletedResponses() const { return completedResponses; }
    const std::vector<CoapClientObserveNotification>& getReceivedNotifications() const { return receivedNotifications; }
    const std::map<CoapObserverKey, CoapClientObservation>& getObservations() const { return observationsByPeerToken; }
    int getFailedCount() const { return failedCount; }
    int getAbortedCount() const { return abortedCount; }
};

} // namespace coap
} // namespace inet

#endif
