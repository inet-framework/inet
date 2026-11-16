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

#include "inet/applications/coap/CoapEndpointBase.h"

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

    static simsignal_t requestStartedSignal;
    static simsignal_t requestCompletedSignal;
    static simsignal_t requestFailedSignal;
    static simsignal_t requestAbortedSignal;
    static simsignal_t requestLatencySignal;

  protected:
    virtual void parseAndValidateParameters() override;
    virtual void handleSubclassSelfMessage(cMessage *msg) override;
    virtual void onEndpointStart() override;
    virtual void onEndpointStop() override;
    virtual void onEndpointCrash() override;

    virtual void onRequestCompleted(uint64_t requestId, const CoapPeerEndpoint& peer,
            const Ptr<const CoapHeader>& responseHeader, const std::vector<uint8_t>& payload) override;
    virtual void onRequestFailed(uint64_t requestId, const char *reason, uint8_t coapCode = 0) override;
    virtual void onRequestAborted(uint64_t requestId) override;

    void scheduleNextSend(simtime_t when);
    uint64_t submitCoapRequest(const L3Address& destAddr, int destPort, uint8_t methodCode,
            bool isConfirmable, const std::string& uriStr, const std::vector<uint8_t>& body,
            int reqContentFormat = -1, int reqAccept = -1, const std::vector<CoapOption>& extraOptions = {});
    uint64_t submitConfiguredRequest();

  public:
    CoapClient() = default;
    virtual ~CoapClient();

    static uint8_t parseMethodName(const std::string& methodName);
    static CoapParsedUri parseCoapUri(const std::string& uriStr);
    static std::string percentDecodeUriComponent(const std::string& input);

    const std::vector<CoapClientResponseRecord>& getCompletedResponses() const { return completedResponses; }
    int getFailedCount() const { return failedCount; }
    int getAbortedCount() const { return abortedCount; }
};

} // namespace coap
} // namespace inet

#endif
