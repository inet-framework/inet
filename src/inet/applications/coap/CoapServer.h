//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_COAP_COAPSERVER_H
#define __INET_COAP_COAPSERVER_H

#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <vector>

#include "inet/applications/coap/CoapEndpointBase.h"
#include "inet/applications/coap/CoapObservation.h"

namespace inet {
namespace coap {

/**
 * In-memory resource state hosted by CoapServer.
 */
struct INET_API CoapResourceEntry
{
    std::string path;
    std::vector<std::string> pathSegments;
    std::vector<uint8_t> payload;
    int32_t contentFormat = 0;
    uint32_t maxAge = 60;
    bool explicitMaxAge = false;
    std::vector<uint8_t> etag;
    std::string resourceType;
    std::string interfaceDescription;
    std::string title;
    bool observable = false;
    bool exists = true;
    bool allowGet = true;
    bool allowPost = false;
    bool allowPut = false;
    bool allowDelete = false;
    bool creatable = false;
};

/**
 * Scheduled deferred separate response owned by CoapServer.
 */
struct INET_API CoapPendingSeparateResponse
{
    simtime_t dueAt = SIMTIME_ZERO;
    CoapPeerEndpoint peer;
    bool confirmable = true;
    std::vector<uint8_t> token;
    Ptr<CoapHeader> header;
    std::vector<uint8_t> payload;
    bool refreshObserveTick = false;
    CoapObserverKey observeKey;
};

/**
 * CoAP over UDP server application hosting configured XML resources.
 */
class INET_API CoapServer : public CoapEndpointBase
{
  protected:
    simtime_t responseDelay = SIMTIME_ZERO;
    uint32_t defaultMaxAgeSeconds = 60;
    uint64_t nextEtagRevision = 1;
    bool etagExhausted = false;
    std::map<std::string, CoapResourceEntry> resources;
    std::deque<CoapPendingSeparateResponse> pendingSeparateResponses;
    std::map<CoapObserverKey, CoapServerObserverRelation> observers;

    static simsignal_t requestHandledSignal;
    static simsignal_t activeObserversSignal;
    static simsignal_t notificationSentSignal;

  protected:
    virtual void parseAndValidateParameters() override;
    virtual simtime_t getNextSubclassDeadline() const override;
    virtual void onSubclassTimer(simtime_t now) override;
    virtual void onEndpointStop() override;
    virtual void onEndpointCrash() override;
    /** Parses initial XML resource configuration and populates initial entries using monotonic ETag revisions. */
    virtual void loadXmlResources(cXMLElement *root);
    std::vector<uint8_t> allocateNextEtag();
    static std::string normalizeResourcePath(const std::string& rawPath);
    static std::vector<std::string> splitPathSegments(const std::string& normalizedPath);
    static std::vector<std::string> extractPathSegments(const CoapHeader& header);
    static std::string extractPathFromHeader(const CoapHeader& header);
    std::string buildCoreLinkFormatPayload(const std::vector<std::string>& queries) const;

    virtual void handleInboundRequest(const CoapPeerEndpoint& peer, bool destIsMulticast,
            const Ptr<const CoapHeader>& header, const std::vector<uint8_t>& payload) override;
    virtual void onOutboundItemAcknowledged(uint64_t outboundId) override;
    virtual void onOutboundItemFailed(uint64_t outboundId, const char *reason) override;
    /** Invoked when a resource representation is served by a GET request; derived servers can override to update dynamic sensor readings or count accesses. */
    virtual void onGetResourceHandled(const std::string& pathKey, const CoapResourceEntry& resource) {}

    void notifyObserversResourceMutated(const std::string& pathKey, const CoapResourceEntry& resource);
    void notifyObserversResourceDeleted(const std::string& pathKey);
    void pumpObserverNotifications(simtime_t now);
    void removeObserverAndEmit(std::map<CoapObserverKey, CoapServerObserverRelation>::iterator it);

    void sendResponsePacket(const CoapPeerEndpoint& peer, bool requestWasConfirmable, uint16_t requestMid,
            const Ptr<const CoapHeader>& requestHeader, uint8_t responseCode, const Ptr<CoapHeader>& responseHeader,
            const std::vector<uint8_t>& payload, simtime_t minDelay = SIMTIME_ZERO,
            bool refreshObserveTick = false, const CoapObserverKey *observeKey = nullptr);

  public:
    CoapServer() = default;
    virtual ~CoapServer() = default;

    bool updateResourceValue(const std::string& path, const std::vector<uint8_t>& newPayload, int32_t newContentFormat = -2);
    bool deleteResourceValue(const std::string& path);
    const std::map<std::string, CoapResourceEntry>& getResources() const { return resources; }
    const std::map<CoapObserverKey, CoapServerObserverRelation>& getObservers() const { return observers; }
};

} // namespace coap
} // namespace inet

#endif
