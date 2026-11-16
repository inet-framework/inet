//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/coap/CoapClient.h"

#include <cctype>
#include <cstdlib>

#include "inet/applications/coap/CoapHeaderSerializer.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {
namespace coap {

Define_Module(CoapClient);

simsignal_t CoapClient::requestStartedSignal = cComponent::registerSignal("requestStarted");
simsignal_t CoapClient::requestCompletedSignal = cComponent::registerSignal("requestCompleted");
simsignal_t CoapClient::requestFailedSignal = cComponent::registerSignal("requestFailed");
simsignal_t CoapClient::requestAbortedSignal = cComponent::registerSignal("requestAborted");
simsignal_t CoapClient::requestLatencySignal = cComponent::registerSignal("requestLatency");
simsignal_t CoapClient::notificationReceivedSignal = cComponent::registerSignal("notificationReceived");
simsignal_t CoapClient::activeObservationsSignal = cComponent::registerSignal("activeObservations");

namespace {

int hexDigitValue(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'a' && ch <= 'f')
        return 10 + (ch - 'a');
    if (ch >= 'A' && ch <= 'F')
        return 10 + (ch - 'A');
    return -1;
}

bool equalsIgnoreCase(const std::string& a, const std::string& b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
            return false;
    }
    return true;
}

std::vector<std::string> splitNonEmptyByChar(const std::string& input, char delim, bool preserveInnerEmpty)
{
    std::vector<std::string> parts;
    if (input.empty())
        return parts;
    size_t start = 0;
    while (start <= input.size()) {
        size_t pos = input.find(delim, start);
        if (pos == std::string::npos) {
            std::string seg = input.substr(start);
            if (!seg.empty() || preserveInnerEmpty)
                parts.push_back(seg);
            break;
        }
        std::string seg = input.substr(start, pos - start);
        if (!seg.empty() || preserveInnerEmpty)
            parts.push_back(seg);
        start = pos + 1;
        if (start == input.size()) {
            if (preserveInnerEmpty)
                parts.emplace_back();
            break;
        }
    }
    return parts;
}

} // namespace

CoapClient::~CoapClient()
{
    cancelAndDelete(sendTimer);
    sendTimer = nullptr;
}

uint8_t CoapClient::parseMethodName(const std::string& methodName)
{
    if (methodName == "GET")
        return COAP_CODE_GET;
    if (methodName == "POST")
        return COAP_CODE_POST;
    if (methodName == "PUT")
        return COAP_CODE_PUT;
    if (methodName == "DELETE")
        return COAP_CODE_DELETE;
    throw cRuntimeError("Invalid CoAP requestMethod '%s'", methodName.c_str());
}

std::string CoapClient::percentDecodeUriComponent(const std::string& input)
{
    std::string result;
    result.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '%') {
            if (i + 2 >= input.size())
                throw cRuntimeError("Truncated percent-encoding in CoAP URI component '%s'", input.c_str());
            int hi = hexDigitValue(input[i + 1]);
            int lo = hexDigitValue(input[i + 2]);
            if (hi < 0 || lo < 0)
                throw cRuntimeError("Invalid percent-encoding in CoAP URI component '%s'", input.c_str());
            result.push_back(static_cast<char>((hi << 4) | lo));
            i += 2;
        }
        else {
            result.push_back(input[i]);
        }
    }
    return result;
}

CoapParsedUri CoapClient::parseCoapUri(const std::string& uriStr)
{
    CoapParsedUri parsed;
    if (uriStr.empty())
        return parsed;

    std::string rest = uriStr;
    size_t schemeSep = rest.find("://");
    if (schemeSep != std::string::npos) {
        std::string scheme = rest.substr(0, schemeSep);
        if (!equalsIgnoreCase(scheme, "coap"))
            throw cRuntimeError("Unsupported CoAP URI scheme '%s'", scheme.c_str());
        parsed.hasAuthority = true;
        rest = rest.substr(schemeSep + 3);

        size_t pathOrQuery = rest.find_first_of("/?");
        std::string authority = (pathOrQuery == std::string::npos) ? rest : rest.substr(0, pathOrQuery);
        rest = (pathOrQuery == std::string::npos) ? "" : rest.substr(pathOrQuery);

        if (!authority.empty() && authority.front() == '[') {
            size_t closeBracket = authority.find(']');
            if (closeBracket == std::string::npos)
                throw cRuntimeError("Malformed IPv6 host in CoAP URI '%s'", uriStr.c_str());
            parsed.host = authority.substr(1, closeBracket - 1);
            if (closeBracket + 1 < authority.size()) {
                if (authority[closeBracket + 1] != ':')
                    throw cRuntimeError("Malformed authority port in CoAP URI '%s'", uriStr.c_str());
                std::string portStr = authority.substr(closeBracket + 2);
                char *endPtr = nullptr;
                long p = std::strtol(portStr.c_str(), &endPtr, 10);
                if (portStr.empty() || *endPtr != '\0' || p < 0 || p > 65535)
                    throw cRuntimeError("Invalid port '%s' in CoAP URI", portStr.c_str());
                parsed.port = static_cast<int>(p);
            }
        }
        else {
            size_t colon = authority.rfind(':');
            if (colon != std::string::npos) {
                parsed.host = authority.substr(0, colon);
                std::string portStr = authority.substr(colon + 1);
                char *endPtr = nullptr;
                long p = std::strtol(portStr.c_str(), &endPtr, 10);
                if (portStr.empty() || *endPtr != '\0' || p < 0 || p > 65535)
                    throw cRuntimeError("Invalid port '%s' in CoAP URI", portStr.c_str());
                parsed.port = static_cast<int>(p);
            }
            else {
                parsed.host = authority;
            }
        }
        parsed.host = percentDecodeUriComponent(parsed.host);
    }

    size_t queryPos = rest.find('?');
    std::string pathPart = (queryPos == std::string::npos) ? rest : rest.substr(0, queryPos);
    std::string queryPart = (queryPos == std::string::npos) ? "" : rest.substr(queryPos + 1);

    if (!pathPart.empty() && pathPart.front() == '/')
        pathPart.erase(pathPart.begin());

    if (!pathPart.empty()) {
        std::vector<std::string> rawPathSegs = splitNonEmptyByChar(pathPart, '/', true);
        for (const auto& seg : rawPathSegs)
            parsed.pathSegments.push_back(percentDecodeUriComponent(seg));
    }

    if (!queryPart.empty()) {
        std::vector<std::string> rawQuerySegs = splitNonEmptyByChar(queryPart, '&', false);
        for (const auto& seg : rawQuerySegs)
            parsed.querySegments.push_back(percentDecodeUriComponent(seg));
    }

    return parsed;
}

void CoapClient::parseAndValidateParameters()
{
    CoapEndpointBase::parseAndValidateParameters();

    peerAddressStr = par("peerAddress").stdstringValue();
    peerPort = par("peerPort");
    requestUri = par("requestUri").stdstringValue();
    requestMethodCode = parseMethodName(par("requestMethod").stdstringValue());
    confirmable = par("confirmable");

    std::string bodyStr = par("requestBody").stdstringValue();
    requestBodyBytes.assign(bodyStr.begin(), bodyStr.end());

    contentFormat = par("contentFormat");
    acceptFormat = par("accept");
    observeOption = par("observe");
    useRequestTag = par("useRequestTag");

    startTime = par("startTime");
    stopTime = par("stopTime");
    numRequests = par("numRequests");

    if (peerPort < 1 || peerPort > 65535)
        throw cRuntimeError("CoapClient peerPort must be in [1, 65535], got %d", peerPort);
    if (contentFormat < -1 || contentFormat > 65535)
        throw cRuntimeError("CoapClient contentFormat out of range: %d", contentFormat);
    if (acceptFormat < -1 || acceptFormat > 65535)
        throw cRuntimeError("CoapClient accept out of range: %d", acceptFormat);
    if (startTime < SIMTIME_ZERO)
        throw cRuntimeError("CoapClient startTime must be non-negative");
    if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
        throw cRuntimeError("CoapClient stopTime must be >= startTime");

    // Eagerly validate configured URI syntax at startup.
    parseCoapUri(requestUri);

    if (sendTimer == nullptr)
        sendTimer = new cMessage("coapClientSendTimer");
}

void CoapClient::onEndpointStart()
{
    numRequestsSent = 0;
    if (!peerAddressStr.empty() && numRequests != 0) {
        simtime_t firstSend = std::max(simTime(), startTime);
        if (stopTime < SIMTIME_ZERO || firstSend < stopTime)
            scheduleNextSend(firstSend);
    }
}

void CoapClient::onEndpointStop()
{
    cancelEvent(sendTimer);
    observationsByPeerToken.clear();
    observationsByCacheKey.clear();
    pendingObserveRegistrations.clear();
    observeConsumerToRelation.clear();
    emit(activeObservationsSignal, 0L);
}

void CoapClient::onEndpointCrash()
{
    cancelEvent(sendTimer);
    observationsByPeerToken.clear();
    observationsByCacheKey.clear();
    pendingObserveRegistrations.clear();
    observeConsumerToRelation.clear();
    emit(activeObservationsSignal, 0L);
}

simtime_t CoapClient::getNextSubclassDeadline() const
{
    simtime_t earliest = SIMTIME_ZERO;
    for (const auto& kv : observationsByPeerToken) {
        const auto& obs = kv.second;
        if (obs.established && obs.hasFreshnessTimer && obs.reregisterAt > SIMTIME_ZERO) {
            if (earliest == SIMTIME_ZERO || obs.reregisterAt < earliest)
                earliest = obs.reregisterAt;
        }
    }
    return earliest;
}

void CoapClient::onSubclassTimer(simtime_t now)
{
    std::vector<CoapObserverKey> dueKeys;
    for (const auto& kv : observationsByPeerToken) {
        const auto& obs = kv.second;
        if (obs.established && obs.hasFreshnessTimer && obs.reregisterAt <= now)
            dueKeys.push_back(kv.first);
    }

    for (const auto& key : dueKeys) {
        auto it = observationsByPeerToken.find(key);
        if (it == observationsByPeerToken.end())
            continue;
        CoapClientObservation& obs = it->second;
        obs.hasFreshnessTimer = false;
        if (obs.activeRegistrationRequestId != 0)
            continue;

        std::vector<CoapOption> opts = obs.templateExtraOptions;
        CoapOption obsOpt;
        obsOpt.setNumber(COAP_OPT_OBSERVE);
        obsOpt.setValueArraySize(0);
        opts.push_back(obsOpt);

        std::vector<uint8_t> emptyBody;
        uint64_t reqId = submitCoapRequestInternal(obs.key.peer.address, obs.key.peer.port, COAP_CODE_GET,
                obs.confirmable, obs.uriStr, emptyBody, -1, obs.cacheKey.acceptFormat, opts,
                &obs.key.token, /*bypassObserveReuse=*/true);
        if (reqId != 0) {
            obs.activeRegistrationRequestId = reqId;
            pendingObserveRegistrations[reqId] = key;
        }
    }
    rescheduleDeadlineTimer();
}

void CoapClient::scheduleNextSend(simtime_t when)
{
    if (sendTimer == nullptr || operationalState != State::OPERATING)
        return;
    cancelEvent(sendTimer);
    scheduleAt(std::max(when, simTime()), sendTimer);
}

void CoapClient::handleSubclassSelfMessage(cMessage *msg)
{
    if (msg == sendTimer) {
        if (stopTime >= SIMTIME_ZERO && simTime() >= stopTime)
            return;

        submitConfiguredRequest();
        ++numRequestsSent;

        if (numRequests < 0 || numRequestsSent < numRequests) {
            simtime_t interval = par("sendInterval");
            if (interval < SIMTIME_ZERO)
                throw cRuntimeError("CoapClient sendInterval must be non-negative");
            simtime_t nextSend = simTime() + interval;
            if (stopTime < SIMTIME_ZERO || nextSend < stopTime)
                scheduleNextSend(nextSend);
        }
        return;
    }

    CoapEndpointBase::handleSubclassSelfMessage(msg);
}

uint64_t CoapClient::submitConfiguredRequest()
{
    if (peerAddressStr.empty())
        throw cRuntimeError("CoapClient peerAddress is empty");

    L3Address destAddr = L3AddressResolver().resolve(peerAddressStr.c_str());
    if (observeOption && requestMethodCode == COAP_CODE_GET)
        return subscribeObserve(destAddr, peerPort, requestUri, confirmable, acceptFormat);
    return submitCoapRequest(destAddr, peerPort, requestMethodCode, confirmable, requestUri,
            requestBodyBytes, contentFormat, acceptFormat);
}

CoapClientObserveCacheKey CoapClient::buildObserveCacheKey(const CoapPeerEndpoint& peer,
        const CoapParsedUri& parsedUri, int reqAccept, const std::vector<CoapOption>& extraOptions)
{
    CoapClientObserveCacheKey key;
    key.peer = peer;
    key.pathSegments = parsedUri.pathSegments;
    key.querySegments = parsedUri.querySegments;
    key.acceptFormat = reqAccept;
    for (const auto& opt : extraOptions) {
        uint32_t v = 0;
        if (opt.getNumber() == COAP_OPT_BLOCK2 && decodeCoapUintValue(opt, v, 3))
            key.block2Szx = static_cast<int32_t>(v & 0x7U);
    }
    return key;
}

uint64_t CoapClient::submitCoapRequest(const L3Address& destAddr, int destPort, uint8_t methodCode,
        bool isConfirmable, const std::string& uriStr, const std::vector<uint8_t>& body,
        int reqContentFormat, int reqAccept, const std::vector<CoapOption>& extraOptions)
{
    return submitCoapRequestInternal(destAddr, destPort, methodCode, isConfirmable, uriStr,
            body, reqContentFormat, reqAccept, extraOptions, nullptr, /*bypassObserveReuse=*/false);
}

uint64_t CoapClient::submitCoapRequestInternal(const L3Address& destAddr, int destPort, uint8_t methodCode,
        bool isConfirmable, const std::string& uriStr, const std::vector<uint8_t>& body,
        int reqContentFormat, int reqAccept, const std::vector<CoapOption>& extraOptions,
        const std::vector<uint8_t> *explicitToken, bool bypassObserveReuse)
{
    CoapParsedUri parsedUri = parseCoapUri(uriStr);
    CoapPeerEndpoint peer{destAddr, destPort};
    bool peerMulticast = destAddr.isMulticast();
    if (peerMulticast && isConfirmable)
        throw cRuntimeError("Confirmable CoAP requests must not target multicast address");

    if (methodCode == COAP_CODE_GET && !bypassObserveReuse && explicitToken == nullptr && !peerMulticast) {
        bool hasBypassOption = false;
        for (const auto& opt : extraOptions) {
            uint16_t num = opt.getNumber();
            if (num == COAP_OPT_OBSERVE || num == COAP_OPT_IF_MATCH || num == COAP_OPT_IF_NONE_MATCH
                    || num == COAP_OPT_ETAG || num == COAP_OPT_BLOCK1 || num == COAP_OPT_BLOCK2) {
                hasBypassOption = true;
                break;
            }
        }
        if (!hasBypassOption) {
            CoapClientObserveCacheKey ck = buildObserveCacheKey(peer, parsedUri, reqAccept, extraOptions);
            auto ckIt = observationsByCacheKey.find(ck);
            if (ckIt != observationsByCacheKey.end()) {
                auto obsIt = observationsByPeerToken.find(ckIt->second);
                if (obsIt != observationsByPeerToken.end()) {
                    const CoapClientObservation& obs = obsIt->second;
                    simtime_t now = simTime();
                    simtime_t freshnessDeadline = obs.lastReceivedTime + SimTime(static_cast<int64_t>(obs.maxAgeSeconds), SIMTIME_S);
                    if (obs.established && obs.complete && now <= freshnessDeadline) {
                        emit(requestStartedSignal, 1L);
                        uint64_t syntheticId = nextRequestId++;
                        activeRequestStarts[syntheticId] = now;
                        auto synthHdr = makeShared<CoapHeader>();
                        synthHdr->setVersion(1);
                        synthHdr->setMessageType(isConfirmable ? COAP_TYPE_ACK : COAP_TYPE_NON);
                        synthHdr->setCode(obs.lastResponseCode);
                        if (!obs.etag.empty())
                            addCoapOption(synthHdr, COAP_OPT_ETAG, obs.etag);
                        if (obs.contentFormat >= 0)
                            addCoapOptionUint(synthHdr, COAP_OPT_CONTENT_FORMAT, static_cast<uint32_t>(obs.contentFormat));
                        uint32_t remainingAge = 0;
                        if (freshnessDeadline > now)
                            remainingAge = static_cast<uint32_t>((freshnessDeadline - now).inUnit(SIMTIME_S));
                        addCoapOptionUint(synthHdr, COAP_OPT_MAX_AGE, remainingAge);
                        onRequestCompleted(syntheticId, obs.key.peer, synthHdr, obs.payload);
                        return syntheticId;
                    }
                }
            }
        }
    }

    emit(requestStartedSignal, 1L);

    std::vector<uint8_t> token;
    if (explicitToken != nullptr) {
        token = *explicitToken;
    }
    else if (!allocateToken(token)) {
        ++failedCount;
        emit(requestFailedSignal, 1L);
        return 0;
    }

    uint64_t requestId = 0;
    if (!registerPendingRequest(peer, peerMulticast, isConfirmable, methodCode, token, requestId)) {
        ++failedCount;
        emit(requestFailedSignal, 1L);
        return 0;
    }

    activeRequestStarts[requestId] = simTime();

    auto header = makeShared<CoapHeader>();
    header->setVersion(1);
    header->setMessageType(isConfirmable ? COAP_TYPE_CON : COAP_TYPE_NON);
    header->setCode(methodCode);
    header->setTokenArraySize(token.size());
    for (size_t i = 0; i < token.size(); ++i)
        header->setToken(i, token[i]);

    if (parsedUri.hasAuthority && !parsedUri.host.empty()) {
        L3Address parsedAddr;
        bool isIpLiteral = parsedAddr.tryParse(parsedUri.host.c_str());
        if (!isIpLiteral || parsedAddr != destAddr)
            addCoapOptionString(header, COAP_OPT_URI_HOST, parsedUri.host);
    }

    if (parsedUri.hasAuthority && parsedUri.port >= 0 && parsedUri.port != destPort)
        addCoapOptionUint(header, COAP_OPT_URI_PORT, static_cast<uint32_t>(parsedUri.port));

    for (const auto& seg : parsedUri.pathSegments)
        addCoapOptionString(header, COAP_OPT_URI_PATH, seg);

    if (reqContentFormat >= 0)
        addCoapOptionUint(header, COAP_OPT_CONTENT_FORMAT, static_cast<uint32_t>(reqContentFormat));

    for (const auto& q : parsedUri.querySegments)
        addCoapOptionString(header, COAP_OPT_URI_QUERY, q);

    if (reqAccept >= 0)
        addCoapOptionUint(header, COAP_OPT_ACCEPT, static_cast<uint32_t>(reqAccept));

    for (const auto& opt : extraOptions) {
        std::vector<uint8_t> val(opt.getValueArraySize());
        for (size_t i = 0; i < opt.getValueArraySize(); ++i)
            val[i] = opt.getValue(i);
        addCoapOption(header, opt.getNumber(), val);
    }

    if (!submitOutboundMessage(peer, peerMulticast, isConfirmable, /*isRequest=*/true, requestId,
            token, "CoapRequest", header, body)) {
        activeRequestStarts.erase(requestId);
        removePendingRequest(token);
        ++failedCount;
        emit(requestFailedSignal, 1L);
        return 0;
    }

    return requestId;
}

uint64_t CoapClient::subscribeObserve(const L3Address& destAddr, int destPort, const std::string& uriStr,
        bool isConfirmable, int reqAccept, const std::vector<CoapOption>& extraOptions)
{
    CoapParsedUri parsedUri = parseCoapUri(uriStr);
    CoapPeerEndpoint peer{destAddr, destPort};
    std::vector<CoapOption> filteredTemplate;
    for (const auto& opt : extraOptions) {
        if (opt.getNumber() != COAP_OPT_OBSERVE && opt.getNumber() != COAP_OPT_ETAG)
            filteredTemplate.push_back(opt);
    }

    CoapClientObserveCacheKey ck = buildObserveCacheKey(peer, parsedUri, reqAccept, filteredTemplate);
    uint64_t consumerId = nextObserveConsumerId++;

    auto ckIt = observationsByCacheKey.find(ck);
    if (ckIt != observationsByCacheKey.end()) {
        auto obsIt = observationsByPeerToken.find(ckIt->second);
        if (obsIt != observationsByPeerToken.end()) {
            CoapClientObservation& existing = obsIt->second;
            existing.consumerIds.insert(consumerId);
            observeConsumerToRelation[consumerId] = existing.key;
            if (existing.established && existing.complete
                    && simTime() <= existing.lastReceivedTime + SimTime(static_cast<int64_t>(existing.maxAgeSeconds), SIMTIME_S)) {
                CoapClientObserveNotification notif;
                notif.relationId = existing.relationId;
                notif.code = existing.lastResponseCode;
                notif.observeValue = existing.lastObserveValue;
                notif.contentFormat = existing.contentFormat;
                notif.etag = existing.etag;
                notif.payload = existing.payload;
                notif.receivedAt = simTime();
                receivedNotifications.push_back(notif);
                onObserveNotification(notif);
            }
            return consumerId;
        }
        observationsByCacheKey.erase(ckIt);
    }

    std::vector<uint8_t> token;
    if (!allocateToken(token))
        return 0;

    CoapObserverKey key{peer, token};
    CoapClientObservation obs;
    obs.relationId = nextObserveRelationId++;
    obs.key = key;
    obs.cacheKey = ck;
    obs.uriStr = uriStr;
    obs.templateExtraOptions = filteredTemplate;
    obs.confirmable = isConfirmable;
    obs.consumerIds.insert(consumerId);

    std::vector<CoapOption> regOptions = filteredTemplate;
    CoapOption obsOpt;
    obsOpt.setNumber(COAP_OPT_OBSERVE);
    obsOpt.setValueArraySize(0);
    regOptions.push_back(obsOpt);

    std::vector<uint8_t> emptyBody;
    uint64_t reqId = submitCoapRequestInternal(destAddr, destPort, COAP_CODE_GET, isConfirmable, uriStr,
            emptyBody, -1, reqAccept, regOptions, &token, /*bypassObserveReuse=*/true);
    if (reqId == 0)
        return 0;

    obs.activeRegistrationRequestId = reqId;
    observationsByPeerToken[key] = std::move(obs);
    observationsByCacheKey[ck] = key;
    observeConsumerToRelation[consumerId] = key;
    pendingObserveRegistrations[reqId] = key;
    return consumerId;
}

bool CoapClient::unsubscribeObserve(uint64_t consumerId, bool sendExplicitCancel)
{
    auto mapIt = observeConsumerToRelation.find(consumerId);
    if (mapIt == observeConsumerToRelation.end())
        return false;
    CoapObserverKey key = mapIt->second;
    observeConsumerToRelation.erase(mapIt);

    auto obsIt = observationsByPeerToken.find(key);
    if (obsIt == observationsByPeerToken.end())
        return false;

    CoapClientObservation& obs = obsIt->second;
    obs.consumerIds.erase(consumerId);
    if (!obs.consumerIds.empty())
        return true;

    CoapClientObservation removed = std::move(obs);
    observationsByCacheKey.erase(removed.cacheKey);
    observationsByPeerToken.erase(obsIt);
    if (removed.activeRegistrationRequestId != 0)
        pendingObserveRegistrations.erase(removed.activeRegistrationRequestId);
    emit(activeObservationsSignal, static_cast<long>(observationsByPeerToken.size()));
    rescheduleDeadlineTimer();

    if (sendExplicitCancel && removed.established) {
        removePendingRequest(removed.key.token);
        std::vector<CoapOption> cancelOpts = removed.templateExtraOptions;
        CoapOption obsOpt;
        obsOpt.setNumber(COAP_OPT_OBSERVE);
        std::vector<uint8_t> cancelBytes = encodeCoapUintValue(1U);
        obsOpt.setValueArraySize(cancelBytes.size());
        for (size_t i = 0; i < cancelBytes.size(); ++i)
            obsOpt.setValue(i, cancelBytes[i]);
        cancelOpts.push_back(obsOpt);

        std::vector<uint8_t> emptyBody;
        submitCoapRequestInternal(removed.key.peer.address, removed.key.peer.port, COAP_CODE_GET,
                removed.confirmable, removed.uriStr, emptyBody, -1, removed.cacheKey.acceptFormat,
                cancelOpts, &removed.key.token, /*bypassObserveReuse=*/true);
    }
    return true;
}

void CoapClient::scheduleObserveFreshnessTimer(CoapClientObservation& obs)
{
    double jitterSec = 5.0 + getRNG(rngStream)->doubleRand() * 10.0;
    obs.hasFreshnessTimer = true;
    obs.reregisterAt = simTime() + SimTime(static_cast<int64_t>(obs.maxAgeSeconds), SIMTIME_S) + SimTime(jitterSec);
    rescheduleDeadlineTimer();
}

void CoapClient::acceptObserveUpdate(CoapClientObservation& obs, const Ptr<const CoapHeader>& responseHeader,
        const std::vector<uint8_t>& payload, uint32_t observeVal)
{
    obs.established = true;
    obs.complete = true;
    obs.lastObserveValue = observeVal & 0xFFFFFFU;
    obs.lastReceivedTime = simTime();

    uint32_t maxAge = 60;
    int32_t cf = obs.contentFormat;
    std::vector<uint8_t> etag = obs.etag;
    for (size_t i = 0; i < responseHeader->getOptionsArraySize(); ++i) {
        const auto& opt = responseHeader->getOptions(i);
        uint32_t v = 0;
        if (opt.getNumber() == COAP_OPT_MAX_AGE && decodeCoapUintValue(opt, v, 4))
            maxAge = v;
        else if (opt.getNumber() == COAP_OPT_CONTENT_FORMAT && decodeCoapUintValue(opt, v, 2))
            cf = static_cast<int32_t>(v);
        else if (opt.getNumber() == COAP_OPT_ETAG) {
            etag.resize(opt.getValueArraySize());
            for (size_t j = 0; j < opt.getValueArraySize(); ++j)
                etag[j] = opt.getValue(j);
        }
    }
    obs.maxAgeSeconds = maxAge;

    if (responseHeader->getCode() != COAP_CODE_VALID) {
        obs.lastResponseCode = responseHeader->getCode();
        obs.contentFormat = cf;
        obs.etag = std::move(etag);
        obs.payload = payload;
    }

    scheduleObserveFreshnessTimer(obs);

    CoapClientObserveNotification notif;
    notif.relationId = obs.relationId;
    notif.code = responseHeader->getCode();
    notif.observeValue = obs.lastObserveValue;
    notif.contentFormat = obs.contentFormat;
    notif.etag = obs.etag;
    notif.payload = obs.payload;
    notif.receivedAt = obs.lastReceivedTime;
    receivedNotifications.push_back(notif);
    emit(notificationReceivedSignal, 1L);
    onObserveNotification(notif);
}

void CoapClient::terminateObservation(const CoapObserverKey& key, uint8_t code, const char *reason)
{
    auto obsIt = observationsByPeerToken.find(key);
    if (obsIt == observationsByPeerToken.end())
        return;

    CoapClientObservation removed = std::move(obsIt->second);
    observationsByCacheKey.erase(removed.cacheKey);
    observationsByPeerToken.erase(obsIt);
    if (removed.activeRegistrationRequestId != 0)
        pendingObserveRegistrations.erase(removed.activeRegistrationRequestId);
    for (uint64_t cid : removed.consumerIds)
        observeConsumerToRelation.erase(cid);
    emit(activeObservationsSignal, static_cast<long>(observationsByPeerToken.size()));
    rescheduleDeadlineTimer();

    onObserveTerminated(removed.relationId, code, reason);
}

bool CoapClient::handleUnsolicitedResponse(const CoapPeerEndpoint& peer, bool destIsMulticast,
        const Ptr<const CoapHeader>& header, const std::vector<uint8_t>& payload)
{
    std::vector<uint8_t> token(header->getTokenArraySize());
    for (size_t i = 0; i < token.size(); ++i)
        token[i] = header->getToken(i);

    CoapObserverKey key{peer, token};
    auto obsIt = observationsByPeerToken.find(key);
    if (obsIt == observationsByPeerToken.end())
        return false;

    uint16_t badOpt = 0;
    if (findUnrecognizedCriticalOption(*header, false, badOpt)) {
        if (header->getMessageType() == COAP_TYPE_CON && !destIsMulticast)
            return false;
        return true;
    }

    if (header->getMessageType() == COAP_TYPE_CON && !destIsMulticast)
        sendEmptyMessageAndCacheDuplicate(COAP_TYPE_ACK, header->getMessageId(), peer);

    bool hasObs = false;
    uint32_t obsVal = 0;
    for (size_t i = 0; i < header->getOptionsArraySize(); ++i) {
        const auto& opt = header->getOptions(i);
        if (opt.getNumber() == COAP_OPT_OBSERVE) {
            if (hasObs || !decodeCoapUintValue(opt, obsVal, 3)) {
                terminateObservation(key, COAP_CODE_BAD_OPTION, "Malformed Observe option in notification");
                return true;
            }
            hasObs = true;
        }
    }

    uint8_t code = header->getCode();
    if (code >= 128 || !hasObs) {
        terminateObservation(key, code, hasObs ? "Server terminated observation with error" : "Notification omitted Observe option");
        return true;
    }

    CoapClientObservation& obs = obsIt->second;
    if (obs.established && !observe::isObserveOrderNewer(obs.lastObserveValue, obs.lastReceivedTime, obsVal, simTime()))
        return true;

    acceptObserveUpdate(obs, header, payload, obsVal);
    return true;
}

void CoapClient::onRequestCompleted(uint64_t requestId, const CoapPeerEndpoint& peer,
        const Ptr<const CoapHeader>& responseHeader, const std::vector<uint8_t>& payload)
{
    simtime_t latency = SIMTIME_ZERO;
    auto startIt = activeRequestStarts.find(requestId);
    if (startIt != activeRequestStarts.end()) {
        latency = simTime() - startIt->second;
        activeRequestStarts.erase(startIt);
    }

    auto regIt = pendingObserveRegistrations.find(requestId);
    if (regIt != pendingObserveRegistrations.end()) {
        CoapObserverKey key = regIt->second;
        pendingObserveRegistrations.erase(regIt);

        bool hasObs = false;
        uint32_t obsVal = 0;
        for (size_t i = 0; i < responseHeader->getOptionsArraySize(); ++i) {
            const auto& opt = responseHeader->getOptions(i);
            if (opt.getNumber() == COAP_OPT_OBSERVE) {
                if (!hasObs && decodeCoapUintValue(opt, obsVal, 3))
                    hasObs = true;
                else {
                    hasObs = false;
                    break;
                }
            }
        }

        auto obsIt = observationsByPeerToken.find(key);
        if (obsIt != observationsByPeerToken.end()) {
            CoapClientObservation& obs = obsIt->second;
            obs.activeRegistrationRequestId = 0;
            uint8_t code = responseHeader->getCode();
            if (code < 128 && hasObs) {
                bool wasEstablished = obs.established;
                if (!wasEstablished || observe::isObserveOrderNewer(obs.lastObserveValue, obs.lastReceivedTime, obsVal, simTime()))
                    acceptObserveUpdate(obs, responseHeader, payload, obsVal);
                if (!wasEstablished)
                    emit(activeObservationsSignal, static_cast<long>(observationsByPeerToken.size()));
                emit(requestCompletedSignal, 1L);
                emit(requestLatencySignal, latency);
                return;
            }
            if (obs.established) {
                terminateObservation(key, code, hasObs ? "Observe re-registration failed" : "Server dropped Observe on refresh");
                return;
            }
            // Initial registration received normal response without Observe -> fall through to one-shot response
            observationsByCacheKey.erase(obs.cacheKey);
            for (uint64_t cid : obs.consumerIds)
                observeConsumerToRelation.erase(cid);
            observationsByPeerToken.erase(obsIt);
            emit(activeObservationsSignal, static_cast<long>(observationsByPeerToken.size()));
        }
        else {
            return;
        }
    }

    CoapClientResponseRecord record;
    record.requestId = requestId;
    record.code = responseHeader->getCode();
    record.messageType = responseHeader->getMessageType();
    record.payload = payload;
    record.latency = latency;

    for (size_t i = 0; i < responseHeader->getOptionsArraySize(); ++i) {
        const auto& opt = responseHeader->getOptions(i);
        uint32_t uintVal = 0;
        if (opt.getNumber() == COAP_OPT_CONTENT_FORMAT && decodeCoapUintValue(opt, uintVal, 2)) {
            record.contentFormat = static_cast<int32_t>(uintVal);
        }
        else if (opt.getNumber() == COAP_OPT_MAX_AGE && decodeCoapUintValue(opt, uintVal, 4)) {
            record.maxAge = static_cast<int64_t>(uintVal);
        }
        else if (opt.getNumber() == COAP_OPT_ETAG) {
            record.etag.resize(opt.getValueArraySize());
            for (size_t j = 0; j < opt.getValueArraySize(); ++j)
                record.etag[j] = opt.getValue(j);
        }
        else if (opt.getNumber() == COAP_OPT_LOCATION_PATH) {
            std::string seg(opt.getValueArraySize(), '\0');
            for (size_t j = 0; j < opt.getValueArraySize(); ++j)
                seg[j] = static_cast<char>(opt.getValue(j));
            record.locationPath.push_back(std::move(seg));
        }
        else if (opt.getNumber() == COAP_OPT_LOCATION_QUERY) {
            std::string seg(opt.getValueArraySize(), '\0');
            for (size_t j = 0; j < opt.getValueArraySize(); ++j)
                seg[j] = static_cast<char>(opt.getValue(j));
            record.locationQuery.push_back(std::move(seg));
        }
    }

    completedResponses.push_back(std::move(record));
    emit(requestCompletedSignal, 1L);
    emit(requestLatencySignal, latency);
}

void CoapClient::onRequestFailed(uint64_t requestId, const char *reason, uint8_t coapCode)
{
    activeRequestStarts.erase(requestId);
    auto regIt = pendingObserveRegistrations.find(requestId);
    if (regIt != pendingObserveRegistrations.end()) {
        CoapObserverKey key = regIt->second;
        pendingObserveRegistrations.erase(regIt);
        terminateObservation(key, coapCode, reason);
    }
    ++failedCount;
    emit(requestFailedSignal, 1L);
}

void CoapClient::onRequestAborted(uint64_t requestId)
{
    activeRequestStarts.erase(requestId);
    auto regIt = pendingObserveRegistrations.find(requestId);
    if (regIt != pendingObserveRegistrations.end()) {
        CoapObserverKey key = regIt->second;
        pendingObserveRegistrations.erase(regIt);
        terminateObservation(key, 0, "Request aborted");
    }
    ++abortedCount;
    emit(requestAbortedSignal, 1L);
}

} // namespace coap
} // namespace inet
