//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_COAP_COAPSERVER_H
#define __INET_COAP_COAPSERVER_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "inet/applications/coap/CoapEndpointBase.h"

namespace inet {
namespace coap {

/**
 * In-memory resource state hosted by CoapServer.
 */
struct INET_API CoapResourceEntry
{
    std::string path;
    std::vector<uint8_t> payload;
    int32_t contentFormat = 0;
    uint32_t maxAge = 60;
    bool explicitMaxAge = false;
    std::vector<uint8_t> etag;
    std::string resourceType;
    std::string interfaceDescription;
    std::string title;
    bool observable = false;
};

/**
 * CoAP over UDP server application hosting configured XML resources.
 */
class INET_API CoapServer : public CoapEndpointBase
{
  protected:
    simtime_t responseDelay = SIMTIME_ZERO;
    uint32_t defaultMaxAgeSeconds = 60;
    std::map<std::string, CoapResourceEntry> resources;

    static simsignal_t requestHandledSignal;

  protected:
    virtual void parseAndValidateParameters() override;
    virtual void loadXmlResources(cXMLElement *root);
    static std::string normalizeResourcePath(const std::string& rawPath);
    static std::string extractPathFromHeader(const CoapHeader& header);

    virtual void handleInboundRequest(const CoapPeerEndpoint& peer, bool destIsMulticast,
            const Ptr<const CoapHeader>& header, const std::vector<uint8_t>& payload) override;
    virtual void onGetResourceHandled(const std::string& pathKey, const CoapResourceEntry& resource) {}

    void sendResponsePacket(const CoapPeerEndpoint& peer, bool requestWasConfirmable, uint16_t requestMid,
            const Ptr<const CoapHeader>& requestHeader, uint8_t responseCode, const Ptr<CoapHeader>& responseHeader,
            const std::vector<uint8_t>& payload);

  public:
    CoapServer() = default;
    virtual ~CoapServer() = default;

    const std::map<std::string, CoapResourceEntry>& getResources() const { return resources; }
};

} // namespace coap
} // namespace inet

#endif
