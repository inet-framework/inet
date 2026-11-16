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
}

void CoapClient::onEndpointCrash()
{
    cancelEvent(sendTimer);
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
    return submitCoapRequest(destAddr, peerPort, requestMethodCode, confirmable, requestUri,
            requestBodyBytes, contentFormat, acceptFormat);
}

uint64_t CoapClient::submitCoapRequest(const L3Address& destAddr, int destPort, uint8_t methodCode,
        bool isConfirmable, const std::string& uriStr, const std::vector<uint8_t>& body,
        int reqContentFormat, int reqAccept, const std::vector<CoapOption>& extraOptions)
{
    CoapParsedUri parsedUri = parseCoapUri(uriStr);
    CoapPeerEndpoint peer{destAddr, destPort};
    bool peerMulticast = destAddr.isMulticast();
    if (peerMulticast && isConfirmable)
        throw cRuntimeError("Confirmable CoAP requests must not target multicast address");

    emit(requestStartedSignal, 1L);

    std::vector<uint8_t> token;
    if (!allocateToken(token)) {
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

void CoapClient::onRequestCompleted(uint64_t requestId, const CoapPeerEndpoint& peer,
        const Ptr<const CoapHeader>& responseHeader, const std::vector<uint8_t>& payload)
{
    simtime_t latency = SIMTIME_ZERO;
    auto startIt = activeRequestStarts.find(requestId);
    if (startIt != activeRequestStarts.end()) {
        latency = simTime() - startIt->second;
        activeRequestStarts.erase(startIt);
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
    ++failedCount;
    emit(requestFailedSignal, 1L);
}

void CoapClient::onRequestAborted(uint64_t requestId)
{
    activeRequestStarts.erase(requestId);
    ++abortedCount;
    emit(requestAbortedSignal, 1L);
}

} // namespace coap
} // namespace inet
