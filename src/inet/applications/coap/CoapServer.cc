//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/coap/CoapServer.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <sstream>

#include "inet/applications/coap/CoapHeaderSerializer.h"

namespace inet {
namespace coap {

Define_Module(CoapServer);

simsignal_t CoapServer::requestHandledSignal = cComponent::registerSignal("requestHandled");
simsignal_t CoapServer::activeObserversSignal = cComponent::registerSignal("activeObservers");
simsignal_t CoapServer::notificationSentSignal = cComponent::registerSignal("notificationSent");

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

std::string trimAscii(const std::string& s)
{
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
        ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
        --end;
    return s.substr(start, end - start);
}

std::vector<std::string> splitWhitespaceTokens(const std::string& input)
{
    std::vector<std::string> out;
    std::istringstream iss(input);
    std::string tok;
    while (iss >> tok)
        out.push_back(tok);
    return out;
}

bool matchLinkPattern(const std::string& candidate, const std::string& pattern)
{
    if (!pattern.empty() && pattern.back() == '*') {
        std::string prefix = pattern.substr(0, pattern.size() - 1);
        return candidate.compare(0, prefix.size(), prefix) == 0;
    }
    return candidate == pattern;
}

} // namespace

std::vector<uint8_t> CoapServer::allocateNextEtag()
{
    if (etagExhausted)
        throw cRuntimeError("CoapServer ETag revision counter exhausted");
    uint64_t val = nextEtagRevision;
    if (nextEtagRevision == std::numeric_limits<uint64_t>::max())
        etagExhausted = true;
    else
        ++nextEtagRevision;
    std::vector<uint8_t> tag(8);
    for (int i = 7; i >= 0; --i) {
        tag[i] = static_cast<uint8_t>(val & 0xFFU);
        val >>= 8U;
    }
    return tag;
}

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

std::vector<std::string> CoapServer::splitPathSegments(const std::string& normalizedPath)
{
    std::vector<std::string> segs;
    if (normalizedPath.empty())
        return segs;
    size_t start = 0;
    while (start <= normalizedPath.size()) {
        size_t slash = normalizedPath.find('/', start);
        if (slash == std::string::npos) {
            segs.push_back(normalizedPath.substr(start));
            break;
        }
        segs.push_back(normalizedPath.substr(start, slash - start));
        start = slash + 1;
    }
    return segs;
}

std::vector<std::string> CoapServer::extractPathSegments(const CoapHeader& header)
{
    std::vector<std::string> segs;
    for (size_t i = 0; i < header.getOptionsArraySize(); ++i) {
        const auto& opt = header.getOptions(i);
        if (opt.getNumber() == COAP_OPT_URI_PATH) {
            std::string seg(opt.getValueArraySize(), '\0');
            for (size_t j = 0; j < opt.getValueArraySize(); ++j)
                seg[j] = static_cast<char>(opt.getValue(j));
            segs.push_back(std::move(seg));
        }
    }
    return segs;
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

namespace {

bool checkPreconditions(bool exists, const std::vector<uint8_t>& currentEtag,
        const std::vector<std::vector<uint8_t>>& ifMatchValues, bool hasIfNoneMatch)
{
    if (!ifMatchValues.empty()) {
        if (!exists)
            return false;
        bool matched = false;
        for (const auto& candidate : ifMatchValues) {
            if (candidate.empty() || candidate == currentEtag) {
                matched = true;
                break;
            }
        }
        if (!matched)
            return false;
    }

    if (hasIfNoneMatch && exists)
        return false;

    return true;
}

void parseMethodsAttribute(const char *methodsAttr, CoapResourceEntry& entry, const char *pathAttr)
{
    if (methodsAttr == nullptr) {
        entry.allowGet = true;
        entry.allowPost = false;
        entry.allowPut = false;
        entry.allowDelete = false;
        return;
    }

    entry.allowGet = false;
    entry.allowPost = false;
    entry.allowPut = false;
    entry.allowDelete = false;

    std::string raw(methodsAttr);
    size_t i = 0;
    while (i < raw.size()) {
        while (i < raw.size() && (raw[i] == ',' || std::isspace(static_cast<unsigned char>(raw[i]))))
            ++i;
        if (i >= raw.size())
            break;
        size_t j = i;
        while (j < raw.size() && raw[j] != ',' && !std::isspace(static_cast<unsigned char>(raw[j])))
            ++j;
        std::string token = raw.substr(i, j - i);
        for (char& ch : token)
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        if (token == "GET")
            entry.allowGet = true;
        else if (token == "POST")
            entry.allowPost = true;
        else if (token == "PUT")
            entry.allowPut = true;
        else if (token == "DELETE")
            entry.allowDelete = true;
        else
            throw cRuntimeError("Unknown CoAP method '%s' on resource '%s'", token.c_str(), pathAttr);
        i = j;
    }
}

} // namespace

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
        entry.pathSegments = splitPathSegments(key);
        entry.maxAge = defaultMaxAgeSeconds;
        entry.exists = parseBoolAttribute(child, "exists", true);
        entry.creatable = parseBoolAttribute(child, "creatable", false);
        parseMethodsAttribute(child->getAttribute("methods"), entry, pathAttr);

        long cfVal = 0;
        if (parseIntAttribute(child, "contentFormat", cfVal) || parseIntAttribute(child, "ct", cfVal)) {
            if (cfVal < -1 || cfVal > 65535)
                throw cRuntimeError("Invalid CoAP contentFormat %ld on resource '%s'", cfVal, pathAttr);
            entry.contentFormat = static_cast<int32_t>(cfVal);
        }
        else {
            entry.contentFormat = entry.exists ? 0 : -1;
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
        if (entry.exists && nodeVal != nullptr)
            entry.payload.assign(nodeVal, nodeVal + std::strlen(nodeVal));

        if (entry.exists)
            entry.etag = allocateNextEtag();

        resources[key] = std::move(entry);
    }
}

std::string CoapServer::buildCoreLinkFormatPayload(const std::vector<std::string>& queries) const
{
    std::string result;
    for (const auto& kv : resources) {
        const CoapResourceEntry& entry = kv.second;
        if (!entry.exists)
            continue;

        std::string fullHref = "/" + entry.path;
        bool matched = true;
        for (const auto& q : queries) {
            size_t eqPos = q.find('=');
            std::string key = (eqPos == std::string::npos) ? q : q.substr(0, eqPos);
            std::string val = (eqPos == std::string::npos) ? std::string() : q.substr(eqPos + 1);
            if (key == "href") {
                if (eqPos == std::string::npos || !matchLinkPattern(fullHref, val)) {
                    matched = false;
                    break;
                }
            }
            else if (key == "rt") {
                auto tokens = splitWhitespaceTokens(entry.resourceType);
                bool any = false;
                for (const auto& tok : tokens) {
                    if (eqPos != std::string::npos && matchLinkPattern(tok, val)) {
                        any = true;
                        break;
                    }
                }
                if (!any) {
                    matched = false;
                    break;
                }
            }
            else if (key == "if") {
                auto tokens = splitWhitespaceTokens(entry.interfaceDescription);
                bool any = false;
                for (const auto& tok : tokens) {
                    if (eqPos != std::string::npos && matchLinkPattern(tok, val)) {
                        any = true;
                        break;
                    }
                }
                if (!any) {
                    matched = false;
                    break;
                }
            }
            else if (key == "ct") {
                if (eqPos == std::string::npos || entry.contentFormat < 0 || !matchLinkPattern(std::to_string(entry.contentFormat), val)) {
                    matched = false;
                    break;
                }
            }
            else if (key == "title") {
                if (eqPos == std::string::npos || entry.title.empty() || !matchLinkPattern(entry.title, val)) {
                    matched = false;
                    break;
                }
            }
            else if (key == "obs") {
                if (!entry.observable) {
                    matched = false;
                    break;
                }
            }
            else {
                matched = false;
                break;
            }
        }

        if (!matched)
            continue;

        if (!result.empty())
            result.push_back(',');
        result.append("<");
        result.append(fullHref);
        result.append(">");
        if (!entry.resourceType.empty())
            result.append(";rt=\"").append(entry.resourceType).append("\"");
        if (!entry.interfaceDescription.empty())
            result.append(";if=\"").append(entry.interfaceDescription).append("\"");
        if (entry.contentFormat >= 0)
            result.append(";ct=").append(std::to_string(entry.contentFormat));
        if (!entry.title.empty())
            result.append(";title=\"").append(entry.title).append("\"");
        if (entry.observable)
            result.append(";obs");
    }
    return result;
}

simtime_t CoapServer::getNextSubclassDeadline() const
{
    simtime_t earliest = SIMTIME_ZERO;
    for (const auto& sep : pendingSeparateResponses) {
        if (earliest == SIMTIME_ZERO || sep.dueAt < earliest)
            earliest = sep.dueAt;
    }
    for (const auto& kv : observers) {
        const auto& obs = kv.second;
        if (obs.hasPendingNotification && !obs.inFlight && obs.pending.earliestSendTime > SIMTIME_ZERO) {
            if (earliest == SIMTIME_ZERO || obs.pending.earliestSendTime < earliest)
                earliest = obs.pending.earliestSendTime;
        }
    }
    return earliest;
}

void CoapServer::onSubclassTimer(simtime_t now)
{
    for (auto it = pendingSeparateResponses.begin(); it != pendingSeparateResponses.end();) {
        if (it->dueAt <= now) {
            CoapPendingSeparateResponse ready = std::move(*it);
            it = pendingSeparateResponses.erase(it);
            if (ready.refreshObserveTick && ready.header != nullptr) {
                uint64_t tick = observe::currentObserveTick(now);
                auto obsIt = observers.find(ready.observeKey);
                if (obsIt != observers.end()) {
                    if (obsIt->second.lastSentValid && tick <= obsIt->second.lastSentTick)
                        tick = obsIt->second.lastSentTick + 1;
                    obsIt->second.lastSentValid = true;
                    obsIt->second.lastSentTick = tick;
                }
                setCoapOptionUint(ready.header, COAP_OPT_OBSERVE, observe::toWireObserveValue(tick));
            }
            submitOutboundMessage(ready.peer, /*peerIsMulticast=*/false, ready.confirmable,
                    /*isRequest=*/false, /*requestId=*/0, ready.token, "CoapSeparateResponse",
                    ready.header, ready.payload);
        }
        else {
            ++it;
        }
    }

    pumpObserverNotifications(now);
}

void CoapServer::onEndpointStop()
{
    pendingSeparateResponses.clear();
    observers.clear();
    emit(activeObserversSignal, 0L);
}

void CoapServer::onEndpointCrash()
{
    pendingSeparateResponses.clear();
    observers.clear();
    emit(activeObserversSignal, 0L);
    loadXmlResources(par("resources").xmlValue());
}

void CoapServer::removeObserverAndEmit(std::map<CoapObserverKey, CoapServerObserverRelation>::iterator it)
{
    if (it == observers.end())
        return;
    observers.erase(it);
    emit(activeObserversSignal, static_cast<long>(observers.size()));
}

void CoapServer::notifyObserversResourceMutated(const std::string& pathKey, const CoapResourceEntry& resource)
{
    for (auto& kv : observers) {
        auto& obs = kv.second;
        if (obs.resourcePath != pathKey)
            continue;

        CoapPendingNotification notif;
        bool formatIncompatible = (resource.contentFormat != obs.negotiatedContentFormat)
                || (obs.acceptFormat >= 0 && resource.contentFormat != obs.acceptFormat);
        if (formatIncompatible) {
            notif.responseCode = COAP_CODE_NOT_ACCEPTABLE;
            notif.includeObserve = false;
            notif.terminal = true;
            notif.contentFormat = -1;
            notif.hasMaxAge = false;
        }
        else {
            notif.responseCode = COAP_CODE_CONTENT;
            notif.payload = resource.payload;
            notif.etag = resource.etag;
            notif.contentFormat = resource.contentFormat;
            notif.maxAge = resource.maxAge;
            notif.hasMaxAge = resource.explicitMaxAge || resource.maxAge != 60U;
            notif.includeObserve = true;
            notif.terminal = false;
        }
        obs.pending = std::move(notif);
        obs.hasPendingNotification = true;
    }

    pumpObserverNotifications(simTime());
}

void CoapServer::notifyObserversResourceDeleted(const std::string& pathKey)
{
    for (auto& kv : observers) {
        auto& obs = kv.second;
        if (obs.resourcePath != pathKey)
            continue;

        CoapPendingNotification notif;
        notif.responseCode = COAP_CODE_NOT_FOUND;
        notif.includeObserve = false;
        notif.terminal = true;
        notif.contentFormat = -1;
        notif.hasMaxAge = false;
        obs.pending = std::move(notif);
        obs.hasPendingNotification = true;
    }

    pumpObserverNotifications(simTime());
}

void CoapServer::pumpObserverNotifications(simtime_t now)
{
    std::vector<CoapObserverKey> keys;
    keys.reserve(observers.size());
    for (const auto& kv : observers)
        keys.push_back(kv.first);

    for (const auto& key : keys) {
        auto obsIt = observers.find(key);
        if (obsIt == observers.end())
            continue;
        CoapServerObserverRelation& obs = obsIt->second;
        if (!obs.hasPendingNotification || obs.inFlight)
            continue;

        if (obs.lastSentValid && obs.pending.includeObserve) {
            uint64_t nowTick = observe::currentObserveTick(now);
            if (nowTick <= obs.lastSentTick) {
                obs.pending.earliestSendTime = SimTime(obs.lastSentTick + 1, SIMTIME_MS);
                continue;
            }
        }

        CoapPendingNotification ready = std::move(obs.pending);
        obs.hasPendingNotification = false;

        auto hdr = makeShared<CoapHeader>();
        hdr->setVersion(1);
        hdr->setMessageType(COAP_TYPE_CON);
        hdr->setCode(ready.responseCode);
        hdr->setTokenArraySize(obs.key.token.size());
        for (size_t i = 0; i < obs.key.token.size(); ++i)
            hdr->setToken(i, obs.key.token[i]);

        if (ready.includeObserve) {
            uint64_t tick = observe::currentObserveTick(now);
            obs.lastSentValid = true;
            obs.lastSentTick = tick;
            addCoapOptionUint(hdr, COAP_OPT_OBSERVE, observe::toWireObserveValue(tick));
        }
        if (!ready.etag.empty())
            addCoapOption(hdr, COAP_OPT_ETAG, ready.etag);
        if (ready.contentFormat >= 0)
            addCoapOptionUint(hdr, COAP_OPT_CONTENT_FORMAT, static_cast<uint32_t>(ready.contentFormat));
        if (ready.hasMaxAge)
            addCoapOptionUint(hdr, COAP_OPT_MAX_AGE, ready.maxAge);

        uint64_t outboundId = 0;
        bool queued = submitOutboundMessage(obs.key.peer, /*peerIsMulticast=*/false, /*confirmable=*/true,
                /*isRequest=*/false, /*requestId=*/0, obs.key.token,
                ready.terminal ? "CoapTerminalNotification" : "CoapNotification",
                hdr, ready.payload, /*refreshObserveOnRetry=*/ready.includeObserve, &outboundId);
        if (!queued) {
            removeObserverAndEmit(obsIt);
            continue;
        }

        emit(notificationSentSignal, 1L);
        if (ready.terminal) {
            removeObserverAndEmit(obsIt);
        }
        else {
            obs.inFlight = true;
            obs.inFlightOutboundId = outboundId;
        }
    }

    rescheduleDeadlineTimer();
}

void CoapServer::onOutboundItemAcknowledged(uint64_t outboundId)
{
    bool matched = false;
    for (auto& kv : observers) {
        if (kv.second.inFlight && kv.second.inFlightOutboundId == outboundId) {
            kv.second.inFlight = false;
            kv.second.inFlightOutboundId = 0;
            matched = true;
            break;
        }
    }
    if (matched)
        pumpObserverNotifications(simTime());
}

void CoapServer::onOutboundItemFailed(uint64_t outboundId, const char *reason)
{
    for (auto it = observers.begin(); it != observers.end(); ++it) {
        if (it->second.inFlight && it->second.inFlightOutboundId == outboundId) {
            removeObserverAndEmit(it);
            return;
        }
    }
}

bool CoapServer::updateResourceValue(const std::string& path, const std::vector<uint8_t>& newPayload, int32_t newContentFormat)
{
    std::string pathKey = normalizeResourcePath(path);
    auto it = resources.find(pathKey);
    if (it == resources.end() || !it->second.exists)
        return false;

    CoapResourceEntry& entry = it->second;
    entry.payload = newPayload;
    if (newContentFormat != -2)
        entry.contentFormat = newContentFormat;
    entry.etag = allocateNextEtag();
    notifyObserversResourceMutated(pathKey, entry);
    return true;
}

bool CoapServer::deleteResourceValue(const std::string& path)
{
    std::string pathKey = normalizeResourcePath(path);
    auto it = resources.find(pathKey);
    if (it == resources.end() || !it->second.exists)
        return false;

    CoapResourceEntry& entry = it->second;
    entry.payload.clear();
    entry.exists = false;
    entry.etag = allocateNextEtag();
    notifyObserversResourceDeleted(pathKey);
    return true;
}

void CoapServer::sendResponsePacket(const CoapPeerEndpoint& peer, bool requestWasConfirmable, uint16_t requestMid,
        const Ptr<const CoapHeader>& requestHeader, uint8_t responseCode, const Ptr<CoapHeader>& responseHeader,
        const std::vector<uint8_t>& payload, simtime_t minDelay, bool refreshObserveTick, const CoapObserverKey *observeKey)
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

    simtime_t delay = std::max(responseDelay, minDelay);
    if (delay > SIMTIME_ZERO) {
        if (requestWasConfirmable)
            sendEmptyMessageAndCacheDuplicate(COAP_TYPE_ACK, requestMid, peer);

        responseHeader->setMessageType(requestWasConfirmable ? COAP_TYPE_CON : COAP_TYPE_NON);
        CoapPendingSeparateResponse sep;
        sep.dueAt = simTime() + delay;
        sep.peer = peer;
        sep.confirmable = requestWasConfirmable;
        sep.token = std::move(tokenVec);
        sep.header = responseHeader;
        sep.payload = payload;
        sep.refreshObserveTick = refreshObserveTick;
        if (observeKey != nullptr)
            sep.observeKey = *observeKey;
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
    std::vector<uint8_t> emptyPayload;

    bool hasAccept = false;
    uint32_t requestedAccept = 0;
    bool hasContentFormat = false;
    uint32_t reqContentFormat = 0;
    bool hasObserve = false;
    uint32_t observeVal = 0;
    bool hasIfNoneMatch = false;
    std::vector<std::vector<uint8_t>> ifMatchValues;
    std::vector<std::vector<uint8_t>> reqEtags;
    std::vector<std::string> uriQueries;

    for (size_t i = 0; i < header->getOptionsArraySize(); ++i) {
        const auto& opt = header->getOptions(i);
        uint16_t optNum = opt.getNumber();
        size_t valSize = opt.getValueArraySize();

        if (optNum == COAP_OPT_PROXY_URI || optNum == COAP_OPT_PROXY_SCHEME) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_PROXYING_NOT_SUPPORTED, respHeader, emptyPayload);
            return;
        }
        else if (optNum == COAP_OPT_OBSERVE) {
            if (hasObserve || !decodeCoapUintValue(opt, observeVal, 3)) {
                auto errHeader = makeShared<CoapHeader>();
                sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_BAD_OPTION, errHeader, emptyPayload);
                return;
            }
            hasObserve = true;
        }
        else if (optNum == COAP_OPT_ACCEPT) {
            if (hasAccept || !decodeCoapUintValue(opt, requestedAccept, 2)) {
                auto errHeader = makeShared<CoapHeader>();
                sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_BAD_OPTION, errHeader, emptyPayload);
                return;
            }
            hasAccept = true;
        }
        else if (optNum == COAP_OPT_CONTENT_FORMAT) {
            if (hasContentFormat || !decodeCoapUintValue(opt, reqContentFormat, 2)) {
                auto errHeader = makeShared<CoapHeader>();
                sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_BAD_OPTION, errHeader, emptyPayload);
                return;
            }
            hasContentFormat = true;
        }
        else if (optNum == COAP_OPT_IF_MATCH) {
            if (valSize > 8) {
                auto errHeader = makeShared<CoapHeader>();
                sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_BAD_OPTION, errHeader, emptyPayload);
                return;
            }
            std::vector<uint8_t> val(valSize);
            for (size_t j = 0; j < valSize; ++j)
                val[j] = opt.getValue(j);
            ifMatchValues.push_back(std::move(val));
        }
        else if (optNum == COAP_OPT_IF_NONE_MATCH) {
            if (hasIfNoneMatch || valSize != 0) {
                auto errHeader = makeShared<CoapHeader>();
                sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_BAD_OPTION, errHeader, emptyPayload);
                return;
            }
            hasIfNoneMatch = true;
        }
        else if (optNum == COAP_OPT_ETAG) {
            if (valSize < 1 || valSize > 8) {
                auto errHeader = makeShared<CoapHeader>();
                sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_BAD_OPTION, errHeader, emptyPayload);
                return;
            }
            std::vector<uint8_t> val(valSize);
            for (size_t j = 0; j < valSize; ++j)
                val[j] = opt.getValue(j);
            reqEtags.push_back(std::move(val));
        }
        else if (optNum == COAP_OPT_URI_QUERY) {
            std::string q;
            q.reserve(valSize);
            for (size_t j = 0; j < valSize; ++j)
                q.push_back(static_cast<char>(opt.getValue(j)));
            uriQueries.push_back(std::move(q));
        }
    }

    std::vector<std::string> pathSegments = extractPathSegments(*header);
    bool isWellKnownCore = (pathSegments.size() == 2 && pathSegments[0] == ".well-known" && pathSegments[1] == "core");
    if (isWellKnownCore) {
        if (method != COAP_CODE_GET) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_METHOD_NOT_ALLOWED, respHeader, emptyPayload);
            return;
        }
        if (!checkPreconditions(true, {}, ifMatchValues, hasIfNoneMatch)) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_PRECONDITION_FAILED, respHeader, emptyPayload);
            return;
        }
        if (hasAccept && requestedAccept != 40U) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_NOT_ACCEPTABLE, respHeader, emptyPayload);
            return;
        }

        std::string linkText = buildCoreLinkFormatPayload(uriQueries);
        std::vector<uint8_t> linkBytes(linkText.begin(), linkText.end());
        auto respHeader = makeShared<CoapHeader>();
        addCoapOptionUint(respHeader, COAP_OPT_CONTENT_FORMAT, 40U);
        if (defaultMaxAgeSeconds != 60U)
            addCoapOptionUint(respHeader, COAP_OPT_MAX_AGE, defaultMaxAgeSeconds);
        sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_CONTENT, respHeader, linkBytes);
        return;
    }

    std::string pathKey;
    for (size_t i = 0; i < pathSegments.size(); ++i) {
        if (i > 0)
            pathKey.push_back('/');
        pathKey.append(pathSegments[i]);
    }

    auto it = resources.find(pathKey);
    bool foundConfigured = (it != resources.end() && it->second.pathSegments == pathSegments);

    if (method == COAP_CODE_GET) {
        if (!foundConfigured || !it->second.exists) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_NOT_FOUND, respHeader, emptyPayload);
            return;
        }

        const CoapResourceEntry& entry = it->second;
        if (!entry.allowGet) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_METHOD_NOT_ALLOWED, respHeader, emptyPayload);
            return;
        }

        if (hasAccept && (entry.contentFormat < 0 || static_cast<int32_t>(requestedAccept) != entry.contentFormat)) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_NOT_ACCEPTABLE, respHeader, emptyPayload);
            return;
        }

        if (!checkPreconditions(entry.exists, entry.etag, ifMatchValues, hasIfNoneMatch)) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_PRECONDITION_FAILED, respHeader, emptyPayload);
            return;
        }

        bool includeObserve = false;
        simtime_t minObserveDelay = SIMTIME_ZERO;
        CoapObserverKey obsKey;
        obsKey.peer = peer;
        obsKey.token.resize(header->getTokenArraySize());
        for (size_t i = 0; i < obsKey.token.size(); ++i)
            obsKey.token[i] = header->getToken(i);

        if (hasObserve && observeVal == 0U && entry.observable) {
            auto obsIt = observers.find(obsKey);
            bool alreadyPresent = (obsIt != observers.end());
            if (alreadyPresent || static_cast<int>(observers.size()) < maxObservers) {
                CoapServerObserverRelation& rel = observers[obsKey];
                rel.key = obsKey;
                rel.resourcePath = pathKey;
                rel.pathSegments = entry.pathSegments;
                rel.negotiatedContentFormat = entry.contentFormat;
                rel.acceptFormat = hasAccept ? static_cast<int32_t>(requestedAccept) : -1;
                rel.hasPendingNotification = false;
                includeObserve = true;
                if (!alreadyPresent)
                    emit(activeObserversSignal, static_cast<long>(observers.size()));

                simtime_t now = simTime();
                uint64_t nowTick = observe::currentObserveTick(now);
                if (rel.lastSentValid && nowTick <= rel.lastSentTick && responseDelay == SIMTIME_ZERO) {
                    minObserveDelay = SimTime(rel.lastSentTick + 1, SIMTIME_MS) - now;
                }
            }
        }
        else if (hasObserve && observeVal == 1U) {
            auto obsIt = observers.find(obsKey);
            if (obsIt != observers.end())
                removeObserverAndEmit(obsIt);
        }

        auto attachInitialObserve = [&](const Ptr<CoapHeader>& respHeader, bool& delayedObserveRefresh) {
            if (!includeObserve)
                return;
            if (responseDelay > SIMTIME_ZERO || minObserveDelay > SIMTIME_ZERO) {
                addCoapOptionUint(respHeader, COAP_OPT_OBSERVE, 0U);
                delayedObserveRefresh = true;
                return;
            }
            uint64_t tick = observe::currentObserveTick(simTime());
            auto obsIt = observers.find(obsKey);
            if (obsIt != observers.end()) {
                obsIt->second.lastSentValid = true;
                obsIt->second.lastSentTick = tick;
            }
            addCoapOptionUint(respHeader, COAP_OPT_OBSERVE, observe::toWireObserveValue(tick));
        };

        bool etagMatched = false;
        for (const auto& candidate : reqEtags) {
            if (!entry.etag.empty() && candidate == entry.etag) {
                etagMatched = true;
                break;
            }
        }
        if (etagMatched) {
            auto respHeader = makeShared<CoapHeader>();
            bool delayedObs = false;
            attachInitialObserve(respHeader, delayedObs);
            addCoapOption(respHeader, COAP_OPT_ETAG, entry.etag);
            if (entry.explicitMaxAge || entry.maxAge != 60U)
                addCoapOptionUint(respHeader, COAP_OPT_MAX_AGE, entry.maxAge);
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_VALID, respHeader, emptyPayload,
                    minObserveDelay, delayedObs, delayedObs ? &obsKey : nullptr);
            return;
        }

        onGetResourceHandled(pathKey, entry);

        auto respHeader = makeShared<CoapHeader>();
        bool delayedObs = false;
        attachInitialObserve(respHeader, delayedObs);
        if (!entry.etag.empty())
            addCoapOption(respHeader, COAP_OPT_ETAG, entry.etag);
        if (entry.contentFormat >= 0)
            addCoapOptionUint(respHeader, COAP_OPT_CONTENT_FORMAT, static_cast<uint32_t>(entry.contentFormat));
        if (entry.explicitMaxAge || entry.maxAge != 60U)
            addCoapOptionUint(respHeader, COAP_OPT_MAX_AGE, entry.maxAge);

        sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_CONTENT, respHeader, entry.payload,
                minObserveDelay, delayedObs, delayedObs ? &obsKey : nullptr);
        return;
    }

    if (method == COAP_CODE_PUT) {
        if (!foundConfigured) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_NOT_FOUND, respHeader, emptyPayload);
            return;
        }

        CoapResourceEntry& entry = it->second;
        if (!entry.allowPut || (!entry.exists && !entry.creatable)) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_METHOD_NOT_ALLOWED, respHeader, emptyPayload);
            return;
        }

        if (!checkPreconditions(entry.exists, entry.etag, ifMatchValues, hasIfNoneMatch)) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_PRECONDITION_FAILED, respHeader, emptyPayload);
            return;
        }

        bool wasCreated = !entry.exists;
        entry.exists = true;
        entry.payload = payload;
        if (hasContentFormat)
            entry.contentFormat = static_cast<int32_t>(reqContentFormat);
        else if (wasCreated)
            entry.contentFormat = -1;
        entry.etag = allocateNextEtag();

        notifyObserversResourceMutated(pathKey, entry);

        auto respHeader = makeShared<CoapHeader>();
        addCoapOption(respHeader, COAP_OPT_ETAG, entry.etag);
        sendResponsePacket(peer, reqConfirmable, reqMid, header,
                wasCreated ? COAP_CODE_CREATED : COAP_CODE_CHANGED, respHeader, emptyPayload);
        return;
    }

    if (method == COAP_CODE_POST) {
        if (!foundConfigured || !it->second.exists) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_NOT_FOUND, respHeader, emptyPayload);
            return;
        }

        CoapResourceEntry& entry = it->second;
        if (!entry.allowPost) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_METHOD_NOT_ALLOWED, respHeader, emptyPayload);
            return;
        }

        if (!checkPreconditions(entry.exists, entry.etag, ifMatchValues, hasIfNoneMatch)) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_PRECONDITION_FAILED, respHeader, emptyPayload);
            return;
        }

        if (hasContentFormat && static_cast<int32_t>(reqContentFormat) != entry.contentFormat) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_UNSUPPORTED_CONTENT_FORMAT, respHeader, emptyPayload);
            return;
        }

        entry.payload.insert(entry.payload.end(), payload.begin(), payload.end());
        entry.etag = allocateNextEtag();

        notifyObserversResourceMutated(pathKey, entry);

        auto respHeader = makeShared<CoapHeader>();
        addCoapOption(respHeader, COAP_OPT_ETAG, entry.etag);
        if (entry.contentFormat >= 0)
            addCoapOptionUint(respHeader, COAP_OPT_CONTENT_FORMAT, static_cast<uint32_t>(entry.contentFormat));
        if (entry.explicitMaxAge || entry.maxAge != 60U)
            addCoapOptionUint(respHeader, COAP_OPT_MAX_AGE, entry.maxAge);
        sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_CHANGED, respHeader, entry.payload);
        return;
    }

    if (method == COAP_CODE_DELETE) {
        if (foundConfigured) {
            CoapResourceEntry& entry = it->second;
            if (!entry.allowDelete) {
                auto respHeader = makeShared<CoapHeader>();
                sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_METHOD_NOT_ALLOWED, respHeader, emptyPayload);
                return;
            }
            if (!checkPreconditions(entry.exists, entry.etag, ifMatchValues, hasIfNoneMatch)) {
                auto respHeader = makeShared<CoapHeader>();
                sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_PRECONDITION_FAILED, respHeader, emptyPayload);
                return;
            }
            if (entry.exists) {
                entry.payload.clear();
                entry.exists = false;
                entry.etag = allocateNextEtag();
                notifyObserversResourceDeleted(pathKey);
            }
        }
        else if (!checkPreconditions(false, {}, ifMatchValues, hasIfNoneMatch)) {
            auto respHeader = makeShared<CoapHeader>();
            sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_PRECONDITION_FAILED, respHeader, emptyPayload);
            return;
        }

        auto respHeader = makeShared<CoapHeader>();
        sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_DELETED, respHeader, emptyPayload);
        return;
    }

    auto respHeader = makeShared<CoapHeader>();
    sendResponsePacket(peer, reqConfirmable, reqMid, header, COAP_CODE_METHOD_NOT_ALLOWED, respHeader, emptyPayload);
}

} // namespace coap
} // namespace inet
