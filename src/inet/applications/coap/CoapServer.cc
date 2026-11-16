//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/coap/CoapServer.h"

#include <cstdlib>
#include <cstring>

#include "inet/applications/coap/CoapHeaderSerializer.h"

namespace inet {
namespace coap {

Define_Module(CoapServer);

simsignal_t CoapServer::requestHandledSignal = cComponent::registerSignal("requestHandled");

namespace {

bool parseIntAttribute(cXMLElement *elem, const char *attrName, long& outVal)
{
    const char *str = elem->getAttribute(attrName);
    if (str == nullptr)
        return false;
    char *endPtr = nullptr;
    long v = std::strtol(str, &endPtr, 10);
    if (str[0] == '\0' || *endPtr != '\0')
        throw cRuntimeError("Invalid integer attribute %s='%s' in CoAP XML resource", attrName, str);
    outVal = v;
    return true;
}

bool parseBoolAttribute(cXMLElement *elem, const char *attrName, bool defaultValue)
{
    const char *str = elem->getAttribute(attrName);
    if (str == nullptr)
        return defaultValue;
    if (strcmp(str, "true") == 0 || strcmp(str, "1") == 0)
        return true;
    if (strcmp(str, "false") == 0 || strcmp(str, "0") == 0)
        return false;
    throw cRuntimeError("Invalid boolean attribute %s='%s' in CoAP XML resource", attrName, str);
}

} // namespace

std::string CoapServer::normalizeResourcePath(const std::string& rawPath)
{
    if (rawPath.empty())
        throw cRuntimeError("CoAP XML resource path must not be empty");
    std::string normalized = rawPath;
    if (normalized.front() == '/')
        normalized.erase(normalized.begin());
    while (!normalized.empty() && normalized.back() == '/')
        normalized.pop_back();
    return normalized;
}

std::string CoapServer::extractPathFromHeader(const CoapHeader& header)
{
    std::string result;
    bool first = true;
    for (size_t i = 0; i < header.getOptionsArraySize(); ++i) {
        const auto& opt = header.getOptions(i);
        if (opt.getNumber() == COAP_OPT_URI_PATH) {
            if (!first)
                result.push_back('/');
            first = false;
            for (size_t j = 0; j < opt.getValueArraySize(); ++j)
                result.push_back(static_cast<char>(opt.getValue(j)));
        }
    }
    return result;
}

void CoapServer::parseAndValidateParameters()
{
    CoapEndpointBase::parseAndValidateParameters();

    responseDelay = par("responseDelay");
    double maxAgeParam = par("defaultMaxAge");
    if (responseDelay < SIMTIME_ZERO)
        throw cRuntimeError("CoapServer responseDelay must be non-negative");
    if (maxAgeParam < 0.0 || maxAgeParam > 4294967295.0)
        throw cRuntimeError("CoapServer defaultMaxAge out of range: %g", maxAgeParam);
    defaultMaxAgeSeconds = static_cast<uint32_t>(maxAgeParam);

    loadXmlResources(par("resources").xmlValue());
}

void CoapServer::loadXmlResources(cXMLElement *root)
{
    resources.clear();
    if (root == nullptr)
        return;
    if (strcmp(root->getTagName(), "resources") != 0)
        throw cRuntimeError("CoapServer resources XML root must be <resources>, got <%s>", root->getTagName());

    for (cXMLElement *child : root->getChildren()) {
        if (strcmp(child->getTagName(), "resource") != 0)
            throw cRuntimeError("Unexpected XML child <%s> in CoapServer <resources>", child->getTagName());

        const char *pathAttr = child->getAttribute("path");
        if (pathAttr == nullptr)
            throw cRuntimeError("CoapServer <resource> requires a 'path' attribute");

        std::string key = normalizeResourcePath(pathAttr);
        if (resources.find(key) != resources.end())
            throw cRuntimeError("Duplicate CoAP resource path '%s'", pathAttr);

        CoapResourceEntry entry;
        entry.path = key;
        entry.maxAge = defaultMaxAgeSeconds;

        long cfVal = 0;
        if (parseIntAttribute(child, "contentFormat", cfVal) || parseIntAttribute(child, "ct", cfVal)) {
            if (cfVal < 0 || cfVal > 65535)
                throw cRuntimeError("Invalid CoAP contentFormat %ld on resource '%s'", cfVal, pathAttr);
            entry.contentFormat = static_cast<int32_t>(cfVal);
        }

        long maxAgeVal = 0;
        if (parseIntAttribute(child, "maxAge", maxAgeVal)) {
            if (maxAgeVal < 0 || maxAgeVal > 4294967295L)
                throw cRuntimeError("Invalid CoAP maxAge %ld on resource '%s'", maxAgeVal, pathAttr);
            entry.maxAge = static_cast<uint32_t>(maxAgeVal);
            entry.explicitMaxAge = true;
        }

        if (const char *rt = child->getAttribute("rt"))
            entry.resourceType = rt;
        if (const char *ifStr = child->getAttribute("if"))
            entry.interfaceDescription = ifStr;
        if (const char *title = child->getAttribute("title"))
            entry.title = title;
        entry.observable = parseBoolAttribute(child, "obs", false);

        const char *nodeVal = child->getNodeValue();
        if (nodeVal != nullptr)
            entry.payload.assign(nodeVal, nodeVal + std::strlen(nodeVal));

        resources[key] = std::move(entry);
    }
}

simtime_t CoapServer::getNextSubclassDeadline() const
{
    simtime_t earliest = SIMTIME_ZERO;
    for (const auto& sep : pendingSeparateResponses) {
        if (earliest == SIMTIME_ZERO || sep.dueAt < earliest)
            earliest = sep.dueAt;
    }
    return earliest;
}

void CoapServer::onSubclassTimer(simtime_t now)
{
    for (auto it = pendingSeparateResponses.begin(); it != pendingSeparateResponses.end();) {
        if (it->dueAt <= now) {
            CoapPendingSeparateResponse ready = std::move(*it);
            it = pendingSeparateResponses.erase(it);
            submitOutboundMessage(ready.peer, /*peerIsMulticast=*/false, ready.confirmable,
                    /*isRequest=*/false, /*requestId=*/0, ready.token, "CoapSeparateResponse",
                    ready.header, ready.payload);
        }
        else {
            ++it;
        }
    }
}

void CoapServer::onEndpointStop()
{
    pendingSeparateResponses.clear();
}

void CoapServer::onEndpointCrash()
{
    pendingSeparateResponses.clear();
}

void CoapServer::sendResponsePacket(const CoapPeerEndpoint& peer, bool requestWasConfirmable, uint16_t requestMid,
        const Ptr<const CoapHeader>& requestHeader, uint8_t responseCode, const Ptr<CoapHeader>& responseHeader,
        const std::vector<uint8_t>& payload)
{
    responseHeader->setVersion(1);
    responseHeader->setCode(responseCode);
    std::vector<uint8_t> tokenVec(requestHeader->getTokenArraySize());
    responseHeader->setTokenArraySize(tokenVec.size());
    for (size_t i = 0; i < tokenVec.size(); ++i) {
        uint8_t b = requestHeader->getToken(i);
        tokenVec[i] = b;
        responseHeader->setToken(i, b);
    }

    if (responseDelay > SIMTIME_ZERO) {
        if (requestWasConfirmable)
            sendEmptyMessageAndCacheDuplicate(COAP_TYPE_ACK, requestMid, peer);

        responseHeader->setMessageType(requestWasConfirmable ? COAP_TYPE_CON : COAP_TYPE_NON);
        CoapPendingSeparateResponse sep;
        sep.dueAt = simTime() + responseDelay;
        sep.peer = peer;
        sep.confirmable = requestWasConfirmable;
        sep.token = std::move(tokenVec);
        sep.header = responseHeader;
        sep.payload = payload;
        pendingSeparateResponses.push_back(std::move(sep));
        rescheduleDeadlineTimer();
        return;
    }

    if (requestWasConfirmable) {
        responseHeader->setMessageType(COAP_TYPE_ACK);
        responseHeader->setMessageId(requestMid);
        Packet *packet = buildCoapPacket("CoapResponse", responseHeader, payload);
        cacheDuplicateResponse(peer, requestMid, packet);
        sendCoapPacket(packet, peer.address, peer.port);
    }
    else {
        responseHeader->setMessageType(COAP_TYPE_NON);
        submitOutboundMessage(peer, /*peerIsMulticast=*/false, /*confirmable=*/false,
                /*isRequest=*/false, /*requestId=*/0, tokenVec, "CoapResponse",
                responseHeader, payload);
    }
}

void CoapServer::handleInboundRequest(const CoapPeerEndpoint& peer, bool destIsMulticast,
        const Ptr<const CoapHeader>& header, const std::vector<uint8_t>& payload)
{
    emit(requestHandledSignal, 1L);

    bool reqConfirmable = (header->getMessageType() == COAP_TYPE_CON) && !destIsMulticast;
    uint16_t reqMid = header->getMessageId();
    uint8_t method = header->getCode();

    if (method != COAP_CODE_GET) {
        auto respHeader = makeShared<CoapHeader>();
        std::vector<uint8_t> emptyPayload;
        sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_METHOD_NOT_ALLOWED, respHeader, emptyPayload);
        return;
    }

    int32_t requestedAccept = -1;
    for (size_t i = 0; i < header->getOptionsArraySize(); ++i) {
        const auto& opt = header->getOptions(i);
        if (opt.getNumber() == COAP_OPT_ACCEPT) {
            uint32_t acceptVal = 0;
            if (!decodeCoapUintValue(opt, acceptVal, 2)) {
                auto errHeader = makeShared<CoapHeader>();
                std::vector<uint8_t> emptyPayload;
                sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_BAD_OPTION, errHeader, emptyPayload);
                return;
            }
            requestedAccept = static_cast<int32_t>(acceptVal);
        }
    }

    std::string pathKey = extractPathFromHeader(*header);
    auto it = resources.find(pathKey);
    if (it == resources.end()) {
        auto respHeader = makeShared<CoapHeader>();
        std::vector<uint8_t> emptyPayload;
        sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_NOT_FOUND, respHeader, emptyPayload);
        return;
    }

    const CoapResourceEntry& entry = it->second;

    if (requestedAccept >= 0 && requestedAccept != entry.contentFormat) {
        auto respHeader = makeShared<CoapHeader>();
        std::vector<uint8_t> emptyPayload;
        sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_NOT_ACCEPTABLE, respHeader, emptyPayload);
        return;
    }

    onGetResourceHandled(pathKey, entry);

    auto respHeader = makeShared<CoapHeader>();
    if (entry.contentFormat >= 0)
        addCoapOptionUint(respHeader, COAP_OPT_CONTENT_FORMAT, static_cast<uint32_t>(entry.contentFormat));
    if (entry.explicitMaxAge || entry.maxAge != 60U)
        addCoapOptionUint(respHeader, COAP_OPT_MAX_AGE, entry.maxAge);

    sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_CONTENT, respHeader, entry.payload);
}

} // namespace coap
} // namespace inet
