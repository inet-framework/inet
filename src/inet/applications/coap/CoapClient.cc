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
    pendingObserveBlock2SubRequests.clear();
    ignoredObserveBlock2SubRequests.clear();
    for (auto& kv : queuedBlockTransfers) {
        for (auto& queued : kv.second)
            onRequestAborted(queued.transferId);
    }
    queuedBlockTransfers.clear();
    activeKeyToTransfer.clear();
    emit(activeObservationsSignal, 0L);
}

void CoapClient::onEndpointCrash()
{
    cancelEvent(sendTimer);
    observationsByPeerToken.clear();
    observationsByCacheKey.clear();
    pendingObserveRegistrations.clear();
    observeConsumerToRelation.clear();
    pendingObserveBlock2SubRequests.clear();
    ignoredObserveBlock2SubRequests.clear();
    for (auto& kv : queuedBlockTransfers) {
        for (auto& queued : kv.second)
            onRequestAborted(queued.transferId);
    }
    queuedBlockTransfers.clear();
    activeKeyToTransfer.clear();
    emit(activeObservationsSignal, 0L);
}

simtime_t CoapClient::getNextSubclassDeadline() const
{
    simtime_t earliest = SIMTIME_ZERO;
    for (const auto& kv : observationsByPeerToken) {
        const auto& obs = kv.second;
        if (obs.hasFreshnessTimer && obs.reregisterAt > SIMTIME_ZERO) {
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
        if (obs.hasFreshnessTimer && obs.reregisterAt <= now)
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

uint64_t CoapClient::submitWireExchange(const CoapPeerEndpoint& peer, bool peerMulticast, uint8_t methodCode,
        bool isConfirmable, const CoapParsedUri& parsedUri, const std::vector<uint8_t>& body,
        int reqContentFormat, int reqAccept, const std::vector<CoapOption>& extraOptions,
        const std::vector<uint8_t> *explicitToken)
{
    std::vector<uint8_t> token;
    if (explicitToken != nullptr) {
        token = *explicitToken;
    }
    else if (!allocateToken(token)) {
        return 0;
    }

    uint64_t subRequestId = 0;
    if (!registerPendingRequest(peer, peerMulticast, isConfirmable, methodCode, token, subRequestId))
        return 0;

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
        if (!isIpLiteral || parsedAddr != peer.address)
            addCoapOptionString(header, COAP_OPT_URI_HOST, parsedUri.host);
    }

    if (parsedUri.hasAuthority && parsedUri.port >= 0 && parsedUri.port != peer.port)
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

    if (!submitOutboundMessage(peer, peerMulticast, isConfirmable, /*isRequest=*/true, subRequestId,
            token, "CoapRequest", header, body)) {
        removePendingRequest(token);
        return 0;
    }

    return subRequestId;
}

bool CoapClient::sendNextTransferStep(blockwise::CoapClientBlockTransfer& transfer)
{
    CoapParsedUri parsedUri = parseCoapUri(transfer.uriStr);
    std::vector<CoapOption> stepOptions;
    bool hasExplicitBlock1 = false;
    bool hasExplicitBlock2 = false;
    for (const auto& opt : transfer.baseExtraOptions) {
        uint16_t num = opt.getNumber();
        if (transfer.uploadDone || transfer.uploadActive) {
            if (num == COAP_OPT_BLOCK1 || num == COAP_OPT_SIZE1)
                continue;
        }
        if (transfer.downloadActive && transfer.expectedNextOffset > 0) {
            if (num == COAP_OPT_BLOCK2 || num == COAP_OPT_SIZE2)
                continue;
        }
        if (num == COAP_OPT_BLOCK1)
            hasExplicitBlock1 = true;
        else if (num == COAP_OPT_BLOCK2)
            hasExplicitBlock2 = true;
        stepOptions.push_back(opt);
    }

    std::vector<uint8_t> stepBody;
    transfer.sentBlock1InLastExchange = false;

    if (transfer.uploadActive && !transfer.uploadDone) {
        uint32_t bs = 1U << (transfer.uploadSzx + 4U);
        size_t rem = transfer.uploadBody.size() - static_cast<size_t>(transfer.uploadOffset);
        size_t chunkLen = std::min<size_t>(bs, rem);
        bool more = (transfer.uploadOffset + chunkLen < transfer.uploadBody.size());
        uint32_t num = static_cast<uint32_t>(transfer.uploadOffset / bs);

        CoapOption b1Opt;
        b1Opt.setNumber(COAP_OPT_BLOCK1);
        std::vector<uint8_t> b1Val = encodeCoapUintValue(blockwise::encodeCoapBlockValue({num, more, transfer.uploadSzx}));
        b1Opt.setValueArraySize(b1Val.size());
        for (size_t i = 0; i < b1Val.size(); ++i)
            b1Opt.setValue(i, b1Val[i]);
        stepOptions.push_back(b1Opt);

        if (num == 0) {
            CoapOption s1Opt;
            s1Opt.setNumber(COAP_OPT_SIZE1);
            std::vector<uint8_t> s1Val = encodeCoapUintValue(static_cast<uint32_t>(transfer.uploadBody.size()));
            s1Opt.setValueArraySize(s1Val.size());
            for (size_t i = 0; i < s1Val.size(); ++i)
                s1Opt.setValue(i, s1Val[i]);
            stepOptions.push_back(s1Opt);
        }

        stepBody.assign(transfer.uploadBody.begin() + static_cast<std::ptrdiff_t>(transfer.uploadOffset),
                transfer.uploadBody.begin() + static_cast<std::ptrdiff_t>(transfer.uploadOffset + chunkLen));
        transfer.sentBlock1InLastExchange = true;
    }
    else if (transfer.downloadActive) {
        uint32_t bs = 1U << (transfer.downloadSzx + 4U);
        uint32_t num = static_cast<uint32_t>(transfer.expectedNextOffset / bs);
        if (transfer.expectedNextOffset > 0 || !hasExplicitBlock2) {
            CoapOption b2Opt;
            b2Opt.setNumber(COAP_OPT_BLOCK2);
            std::vector<uint8_t> b2Val = encodeCoapUintValue(blockwise::encodeCoapBlockValue({num, false, transfer.downloadSzx}));
            b2Opt.setValueArraySize(b2Val.size());
            for (size_t i = 0; i < b2Val.size(); ++i)
                b2Opt.setValue(i, b2Val[i]);
            stepOptions.push_back(b2Opt);
        }
    }
    else {
        stepBody = transfer.uploadBody;
        transfer.sentBlock1InLastExchange = hasExplicitBlock1;
    }

    uint64_t subReqId = submitWireExchange(transfer.peer, transfer.peerIsMulticast, transfer.methodCode,
            transfer.isConfirmable, parsedUri, stepBody, transfer.reqContentFormat, transfer.reqAccept,
            stepOptions, nullptr);
    if (subReqId == 0)
        return false;

    transfer.activeSubRequestId = subReqId;
    subRequestToTransfer[subReqId] = transfer.transferId;
    return true;
}

bool CoapClient::startClientBlockTransfer(blockwise::CoapClientBlockTransfer& transfer)
{
    uint64_t id = transfer.transferId;
    activeBlockTransfers[id] = transfer;
    auto& stored = activeBlockTransfers[id];
    if (!sendNextTransferStep(stored)) {
        activeBlockTransfers.erase(id);
        return false;
    }
    return true;
}

void CoapClient::releaseTransferSlotAndPumpQueue(const blockwise::CoapBlockRequestKey& key)
{
    activeKeyToTransfer.erase(key);
    auto qIt = queuedBlockTransfers.find(key);
    while (qIt != queuedBlockTransfers.end() && !qIt->second.empty()) {
        blockwise::CoapClientBlockTransfer next = std::move(qIt->second.front());
        qIt->second.pop_front();
        if (qIt->second.empty())
            queuedBlockTransfers.erase(qIt);

        next.serializedSlotHeld = true;
        activeKeyToTransfer[key] = next.transferId;
        if (startClientBlockTransfer(next))
            return;

        activeKeyToTransfer.erase(key);
        onRequestFailed(next.transferId, "Failed to start queued CoAP blockwise transfer");
        qIt = queuedBlockTransfers.find(key);
    }
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

    bool hasExplicitBlock1 = false;
    bool hasExplicitBlock2 = false;
    bool hasExplicitObserve = false;
    bool hasExplicitRequestTag = false;
    blockwise::CoapBlockOption explicitB2;
    std::vector<std::vector<uint8_t>> ifMatchValues;
    bool hasIfNoneMatch = false;
    std::vector<std::vector<uint8_t>> requestTags;

    for (const auto& opt : extraOptions) {
        uint16_t num = opt.getNumber();
        if (num == COAP_OPT_OBSERVE)
            hasExplicitObserve = true;
        else if (num == COAP_OPT_BLOCK1)
            hasExplicitBlock1 = true;
        else if (num == COAP_OPT_BLOCK2) {
            hasExplicitBlock2 = true;
            blockwise::decodeCoapBlockOption(opt, explicitB2);
        }
        else if (num == COAP_OPT_IF_MATCH) {
            std::vector<uint8_t> val(opt.getValueArraySize());
            for (size_t i = 0; i < val.size(); ++i)
                val[i] = opt.getValue(i);
            ifMatchValues.push_back(std::move(val));
        }
        else if (num == COAP_OPT_IF_NONE_MATCH)
            hasIfNoneMatch = true;
        else if (num == COAP_OPT_REQUEST_TAG) {
            hasExplicitRequestTag = true;
            std::vector<uint8_t> val(opt.getValueArraySize());
            for (size_t i = 0; i < val.size(); ++i)
                val[i] = opt.getValue(i);
            requestTags.push_back(std::move(val));
        }
    }

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

    // Direct single-exchange path for Observe registrations/cancellations or explicit-token requests.
    if (explicitToken != nullptr || hasExplicitObserve || bypassObserveReuse) {
        uint64_t subReqId = submitWireExchange(peer, peerMulticast, methodCode, isConfirmable, parsedUri,
                body, reqContentFormat, reqAccept, extraOptions, explicitToken);
        if (subReqId == 0) {
            ++failedCount;
            emit(requestFailedSignal, 1L);
            return 0;
        }
        activeRequestStarts[subReqId] = simTime();
        return subReqId;
    }

    uint8_t prefSzx = 6;
    blockwise::szxFromBlockSize(static_cast<uint32_t>(preferredBlockSize), prefSzx);

    blockwise::CoapClientBlockTransfer transfer;
    transfer.transferId = nextRequestId++;
    transfer.peer = peer;
    transfer.peerIsMulticast = peerMulticast;
    transfer.isConfirmable = isConfirmable;
    transfer.methodCode = methodCode;
    transfer.uriStr = uriStr;
    transfer.reqContentFormat = reqContentFormat;
    transfer.reqAccept = reqAccept;
    transfer.baseExtraOptions = extraOptions;
    transfer.logicalStartTime = simTime();
    transfer.hasUploadBody = !body.empty();
    transfer.uploadBody = body;
    transfer.uploadSzx = prefSzx;
    transfer.downloadSzx = hasExplicitBlock2 ? std::min(explicitB2.szx, prefSzx) : prefSzx;

    bool needsBlock1 = (methodCode == COAP_CODE_PUT || methodCode == COAP_CODE_POST)
            && !hasExplicitBlock1 && !peerMulticast
            && body.size() > static_cast<size_t>(preferredBlockSize);

    if (useRequestTag && !hasExplicitRequestTag && !peerMulticast
            && (methodCode == COAP_CODE_PUT || methodCode == COAP_CODE_POST)) {
        std::vector<uint8_t> autoTag = encodeCoapUintValue(static_cast<uint32_t>(nextRequestTagCounter++));
        CoapOption tagOpt;
        tagOpt.setNumber(COAP_OPT_REQUEST_TAG);
        tagOpt.setValueArraySize(autoTag.size());
        for (size_t i = 0; i < autoTag.size(); ++i)
            tagOpt.setValue(i, autoTag[i]);
        transfer.baseExtraOptions.push_back(tagOpt);
        requestTags.push_back(std::move(autoTag));
    }

    transfer.uploadActive = needsBlock1;
    transfer.downloadActive = hasExplicitBlock2;

    transfer.serializationKey.peer = peer;
    transfer.serializationKey.methodCode = methodCode;
    transfer.serializationKey.pathSegments = parsedUri.pathSegments;
    transfer.serializationKey.uriQueries = parsedUri.querySegments;
    transfer.serializationKey.contentFormat = reqContentFormat;
    transfer.serializationKey.acceptFormat = reqAccept;
    transfer.serializationKey.ifMatchValues = std::move(ifMatchValues);
    transfer.serializationKey.hasIfNoneMatch = hasIfNoneMatch;
    transfer.serializationKey.requestTags = std::move(requestTags);

    activeRequestStarts[transfer.transferId] = simTime();

    bool alreadySerialized = (activeKeyToTransfer.find(transfer.serializationKey) != activeKeyToTransfer.end());
    if (alreadySerialized) {
        uint64_t queuedId = transfer.transferId;
        auto& q = queuedBlockTransfers[transfer.serializationKey];
        q.push_back(std::move(transfer));
        return queuedId;
    }

    bool holdSlot = needsBlock1 || hasExplicitBlock2
            || (!peerMulticast && (methodCode == COAP_CODE_POST || methodCode == COAP_CODE_PUT));
    if (holdSlot) {
        activeKeyToTransfer[transfer.serializationKey] = transfer.transferId;
        transfer.serializedSlotHeld = true;
    }

    uint64_t transferId = transfer.transferId;
    if (!startClientBlockTransfer(transfer)) {
        activeRequestStarts.erase(transferId);
        if (transfer.serializedSlotHeld)
            activeKeyToTransfer.erase(transfer.serializationKey);
        ++failedCount;
        emit(requestFailedSignal, 1L);
        return 0;
    }

    return transferId;
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

    cancelInFlightObserveBlock2(obs);
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

void CoapClient::scheduleObserveBlock2Reregister(CoapClientObservation& obs)
{
    double jitterSec = 5.0 + getRNG(rngStream)->doubleRand() * 10.0;
    obs.hasFreshnessTimer = true;
    obs.reregisterAt = simTime() + SimTime(jitterSec);
    rescheduleDeadlineTimer();
}

void CoapClient::cancelInFlightObserveBlock2(CoapClientObservation& obs)
{
    if (obs.block2ActiveSubRequestId != 0) {
        pendingObserveBlock2SubRequests.erase(obs.block2ActiveSubRequestId);
        ignoredObserveBlock2SubRequests.insert(obs.block2ActiveSubRequestId);
        obs.block2ActiveSubRequestId = 0;
    }
    obs.block2InProgress = false;
    obs.block2ExpectedOffset = 0;
    obs.block2Buffer.clear();
    obs.block2PendingEtag.clear();
}

bool CoapClient::sendNextObserveBlock2Request(CoapClientObservation& obs)
{
    CoapParsedUri parsedUri = parseCoapUri(obs.uriStr);
    std::vector<CoapOption> stepOptions;
    for (const auto& opt : obs.templateExtraOptions) {
        uint16_t num = opt.getNumber();
        if (num == COAP_OPT_OBSERVE || num == COAP_OPT_BLOCK2 || num == COAP_OPT_SIZE2
                || num == COAP_OPT_ETAG || num == COAP_OPT_IF_MATCH || num == COAP_OPT_IF_NONE_MATCH) {
            continue;
        }
        stepOptions.push_back(opt);
    }

    uint32_t bs = 1U << (obs.block2Szx + 4U);
    uint32_t nextNum = static_cast<uint32_t>(obs.block2ExpectedOffset / bs);
    CoapOption b2Opt;
    b2Opt.setNumber(COAP_OPT_BLOCK2);
    std::vector<uint8_t> b2Val = encodeCoapUintValue(blockwise::encodeCoapBlockValue({nextNum, false, obs.block2Szx}));
    b2Opt.setValueArraySize(b2Val.size());
    for (size_t i = 0; i < b2Val.size(); ++i)
        b2Opt.setValue(i, b2Val[i]);
    stepOptions.push_back(b2Opt);

    std::vector<uint8_t> emptyBody;
    uint64_t subReqId = submitWireExchange(obs.key.peer, /*peerMulticast=*/false, COAP_CODE_GET,
            obs.confirmable, parsedUri, emptyBody, -1, obs.cacheKey.acceptFormat, stepOptions, nullptr);
    if (subReqId == 0)
        return false;

    obs.block2ActiveSubRequestId = subReqId;
    pendingObserveBlock2SubRequests[subReqId] = obs.key;
    return true;
}

bool CoapClient::handleObservePayloadResponse(CoapClientObservation& obs, const Ptr<const CoapHeader>& responseHeader,
        const std::vector<uint8_t>& payload, uint32_t observeVal, bool fromRegistration, uint64_t originRequestId)
{
    bool hasBlock2 = false;
    blockwise::CoapBlockOption b2;
    uint32_t maxAge = 60;
    int32_t cf = obs.contentFormat;
    std::vector<std::vector<uint8_t>> etags;

    for (size_t i = 0; i < responseHeader->getOptionsArraySize(); ++i) {
        const auto& opt = responseHeader->getOptions(i);
        uint16_t num = opt.getNumber();
        uint32_t v = 0;
        if (num == COAP_OPT_BLOCK2) {
            if (hasBlock2 || blockwise::decodeCoapBlockOption(opt, b2) != blockwise::CoapBlockDecodeStatus::OK) {
                cancelInFlightObserveBlock2(obs);
                if (obs.established)
                    scheduleObserveBlock2Reregister(obs);
                return true;
            }
            hasBlock2 = true;
        }
        else if (num == COAP_OPT_MAX_AGE && decodeCoapUintValue(opt, v, 4)) {
            maxAge = v;
        }
        else if (num == COAP_OPT_CONTENT_FORMAT && decodeCoapUintValue(opt, v, 2)) {
            cf = static_cast<int32_t>(v);
        }
        else if (num == COAP_OPT_ETAG) {
            std::vector<uint8_t> val(opt.getValueArraySize());
            for (size_t j = 0; j < val.size(); ++j)
                val[j] = opt.getValue(j);
            etags.push_back(std::move(val));
        }
    }

    if (responseHeader->getCode() == COAP_CODE_VALID || !hasBlock2 || !b2.more) {
        if (obs.block2InProgress && obs.block2FromRegistration && obs.block2OriginRequestId != 0
                && !fromRegistration) {
            fromRegistration = true;
            originRequestId = obs.block2OriginRequestId;
        }
        cancelInFlightObserveBlock2(obs);
        obs.block2FromRegistration = false;
        obs.block2OriginRequestId = 0;
        bool wasEstablished = obs.established;
        acceptObserveUpdate(obs, responseHeader, payload, observeVal);
        if (!wasEstablished)
            emit(activeObservationsSignal, static_cast<long>(observationsByPeerToken.size()));
        if (fromRegistration && originRequestId != 0) {
            simtime_t latency = SIMTIME_ZERO;
            auto startIt = activeRequestStarts.find(originRequestId);
            if (startIt != activeRequestStarts.end()) {
                latency = simTime() - startIt->second;
                activeRequestStarts.erase(startIt);
            }
            emit(requestCompletedSignal, 1L);
            emit(requestLatencySignal, latency);
        }
        return true;
    }

    if (b2.num != 0 || etags.empty() || payload.size() != b2.blockSize()
            || payload.size() > static_cast<size_t>(maxIncomingBlockSize)) {
        cancelInFlightObserveBlock2(obs);
        if (payload.size() > static_cast<size_t>(maxIncomingBlockSize))
            emit(capacityRejectedSignal, 1L);
        scheduleObserveBlock2Reregister(obs);
        return true;
    }

    bool carriedFromRegistration = obs.block2InProgress && obs.block2FromRegistration && obs.block2OriginRequestId != 0 && !fromRegistration;
    uint64_t carriedOriginId = carriedFromRegistration ? obs.block2OriginRequestId : originRequestId;
    bool effectiveFromReg = fromRegistration || carriedFromRegistration;

    cancelInFlightObserveBlock2(obs);

    uint8_t prefSzx = 6;
    blockwise::szxFromBlockSize(static_cast<uint32_t>(preferredBlockSize), prefSzx);

    obs.lastObserveValue = observeVal & 0xFFFFFFU;
    obs.lastReceivedTime = simTime();
    obs.complete = false;
    obs.block2InProgress = true;
    obs.block2FromRegistration = effectiveFromReg;
    obs.block2OriginRequestId = carriedOriginId;
    obs.block2PendingObserveVal = obs.lastObserveValue;
    obs.block2PendingReceivedAt = obs.lastReceivedTime;
    obs.block2PendingCode = responseHeader->getCode();
    obs.block2PendingContentFormat = cf;
    obs.block2PendingMaxAge = maxAge;
    obs.block2PendingEtag = etags[0];
    obs.block2Szx = std::min(b2.szx, prefSzx);
    obs.block2ExpectedOffset = payload.size();
    obs.block2Buffer = payload;

    if (!sendNextObserveBlock2Request(obs)) {
        uint64_t abortOriginId = obs.block2FromRegistration ? obs.block2OriginRequestId : 0;
        obs.block2FromRegistration = false;
        obs.block2OriginRequestId = 0;
        cancelInFlightObserveBlock2(obs);
        if (abortOriginId != 0)
            activeRequestStarts.erase(abortOriginId);
        scheduleObserveBlock2Reregister(obs);
    }
    return false;
}

void CoapClient::handleObserveBlock2Response(uint64_t subRequestId, const CoapPeerEndpoint& peer,
        const Ptr<const CoapHeader>& responseHeader, const std::vector<uint8_t>& payload)
{
    auto mapIt = pendingObserveBlock2SubRequests.find(subRequestId);
    if (mapIt == pendingObserveBlock2SubRequests.end())
        return;
    CoapObserverKey key = mapIt->second;
    pendingObserveBlock2SubRequests.erase(mapIt);

    auto obsIt = observationsByPeerToken.find(key);
    if (obsIt == observationsByPeerToken.end())
        return;
    CoapClientObservation& obs = obsIt->second;
    if (!obs.block2InProgress || obs.block2ActiveSubRequestId != subRequestId)
        return;

    obs.block2ActiveSubRequestId = 0;
    uint8_t code = responseHeader->getCode();
    if (code >= 128) {
        bool fromReg = obs.block2FromRegistration;
        uint64_t originId = obs.block2OriginRequestId;
        cancelInFlightObserveBlock2(obs);
        obs.block2FromRegistration = false;
        obs.block2OriginRequestId = 0;
        if (fromReg && originId != 0) {
            activeRequestStarts.erase(originId);
            ++failedCount;
            emit(requestFailedSignal, 1L);
        }
        terminateObservation(key, code, "Observe Block2 follow-up returned error");
        return;
    }

    bool hasBlock2 = false;
    blockwise::CoapBlockOption b2;
    int32_t cf = obs.block2PendingContentFormat;
    uint32_t maxAge = obs.block2PendingMaxAge;
    bool hasMaxAge = false;
    std::vector<std::vector<uint8_t>> etags;

    for (size_t i = 0; i < responseHeader->getOptionsArraySize(); ++i) {
        const auto& opt = responseHeader->getOptions(i);
        uint16_t num = opt.getNumber();
        uint32_t v = 0;
        if (num == COAP_OPT_BLOCK2) {
            if (hasBlock2 || blockwise::decodeCoapBlockOption(opt, b2) != blockwise::CoapBlockDecodeStatus::OK) {
                hasBlock2 = false;
                break;
            }
            hasBlock2 = true;
        }
        else if (num == COAP_OPT_CONTENT_FORMAT && decodeCoapUintValue(opt, v, 2)) {
            cf = static_cast<int32_t>(v);
        }
        else if (num == COAP_OPT_MAX_AGE && decodeCoapUintValue(opt, v, 4)) {
            maxAge = v;
            hasMaxAge = true;
        }
        else if (num == COAP_OPT_ETAG) {
            std::vector<uint8_t> val(opt.getValueArraySize());
            for (size_t j = 0; j < val.size(); ++j)
                val[j] = opt.getValue(j);
            etags.push_back(std::move(val));
        }
    }

    bool validShape = hasBlock2 && b2.byteOffset() == obs.block2ExpectedOffset
            && (!b2.more || payload.size() == b2.blockSize());
    bool etagMatches = !etags.empty() && etags[0] == obs.block2PendingEtag;

    if (!validShape || !etagMatches) {
        bool fromReg = obs.block2FromRegistration;
        uint64_t originId = obs.block2OriginRequestId;
        cancelInFlightObserveBlock2(obs);
        obs.block2FromRegistration = false;
        obs.block2OriginRequestId = 0;
        obs.complete = false;
        if (fromReg && originId != 0) {
            activeRequestStarts.erase(originId);
            ++failedCount;
            emit(requestFailedSignal, 1L);
        }
        scheduleObserveBlock2Reregister(obs);
        return;
    }

    if (obs.block2Buffer.size() + payload.size() > static_cast<size_t>(maxIncomingBlockSize)) {
        emit(capacityRejectedSignal, 1L);
        bool fromReg = obs.block2FromRegistration;
        uint64_t originId = obs.block2OriginRequestId;
        cancelInFlightObserveBlock2(obs);
        obs.block2FromRegistration = false;
        obs.block2OriginRequestId = 0;
        if (fromReg && originId != 0) {
            activeRequestStarts.erase(originId);
            ++failedCount;
            emit(requestFailedSignal, 1L);
        }
        terminateObservation(key, COAP_CODE_REQUEST_ENTITY_TOO_LARGE, "Assembled Observe Block2 payload exceeds limit");
        return;
    }

    obs.block2Buffer.insert(obs.block2Buffer.end(), payload.begin(), payload.end());
    obs.block2ExpectedOffset += payload.size();
    obs.block2Szx = std::min(obs.block2Szx, b2.szx);
    obs.block2PendingContentFormat = cf;
    if (hasMaxAge)
        obs.block2PendingMaxAge = maxAge;

    if (b2.more) {
        if (!sendNextObserveBlock2Request(obs)) {
            bool fromReg = obs.block2FromRegistration;
            uint64_t originId = obs.block2OriginRequestId;
            cancelInFlightObserveBlock2(obs);
            obs.block2FromRegistration = false;
            obs.block2OriginRequestId = 0;
            if (fromReg && originId != 0) {
                activeRequestStarts.erase(originId);
                ++failedCount;
                emit(requestFailedSignal, 1L);
            }
            scheduleObserveBlock2Reregister(obs);
        }
        return;
    }

    bool wasEstablished = obs.established;
    bool fromReg = obs.block2FromRegistration;
    uint64_t originId = obs.block2OriginRequestId;
    std::vector<uint8_t> assembled = std::move(obs.block2Buffer);
    std::vector<uint8_t> finalEtag = obs.block2PendingEtag;
    uint32_t finalObsVal = obs.block2PendingObserveVal;
    simtime_t finalRecvAt = obs.block2PendingReceivedAt;
    uint8_t finalCode = obs.block2PendingCode;
    int32_t finalCf = obs.block2PendingContentFormat;
    uint32_t finalMaxAge = obs.block2PendingMaxAge;

    obs.block2InProgress = false;
    obs.block2FromRegistration = false;
    obs.block2OriginRequestId = 0;
    obs.block2ActiveSubRequestId = 0;
    obs.block2ExpectedOffset = 0;
    obs.block2Buffer.clear();
    obs.block2PendingEtag.clear();

    obs.established = true;
    obs.complete = true;
    obs.lastResponseCode = finalCode;
    obs.lastObserveValue = finalObsVal;
    obs.lastReceivedTime = finalRecvAt;
    obs.maxAgeSeconds = finalMaxAge;
    obs.contentFormat = finalCf;
    obs.etag = std::move(finalEtag);
    obs.payload = std::move(assembled);

    scheduleObserveFreshnessTimer(obs);

    CoapClientObserveNotification notif;
    notif.relationId = obs.relationId;
    notif.code = finalCode;
    notif.observeValue = finalObsVal;
    notif.contentFormat = obs.contentFormat;
    notif.etag = obs.etag;
    notif.payload = obs.payload;
    notif.receivedAt = simTime();
    receivedNotifications.push_back(notif);
    emit(notificationReceivedSignal, 1L);
    onObserveNotification(notif);

    if (!wasEstablished)
        emit(activeObservationsSignal, static_cast<long>(observationsByPeerToken.size()));

    if (fromReg && originId != 0) {
        simtime_t latency = SIMTIME_ZERO;
        auto startIt = activeRequestStarts.find(originId);
        if (startIt != activeRequestStarts.end()) {
            latency = simTime() - startIt->second;
            activeRequestStarts.erase(startIt);
        }
        emit(requestCompletedSignal, 1L);
        emit(requestLatencySignal, latency);
    }
}

void CoapClient::handleObserveBlock2FailureOrAbort(uint64_t subRequestId, bool aborted, const char *reason, uint8_t coapCode)
{
    auto mapIt = pendingObserveBlock2SubRequests.find(subRequestId);
    if (mapIt == pendingObserveBlock2SubRequests.end())
        return;
    CoapObserverKey key = mapIt->second;
    pendingObserveBlock2SubRequests.erase(mapIt);

    auto obsIt = observationsByPeerToken.find(key);
    if (obsIt == observationsByPeerToken.end())
        return;
    CoapClientObservation& obs = obsIt->second;
    if (!obs.block2InProgress || obs.block2ActiveSubRequestId != subRequestId)
        return;

    bool fromReg = obs.block2FromRegistration;
    uint64_t originId = obs.block2OriginRequestId;
    cancelInFlightObserveBlock2(obs);
    obs.block2FromRegistration = false;
    obs.block2OriginRequestId = 0;

    if (fromReg && originId != 0) {
        activeRequestStarts.erase(originId);
        if (aborted) {
            ++abortedCount;
            emit(requestAbortedSignal, 1L);
        }
        else {
            ++failedCount;
            emit(requestFailedSignal, 1L);
        }
    }
    terminateObservation(key, coapCode, reason);
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

    cancelInFlightObserveBlock2(obsIt->second);
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
    if ((obs.established || obs.block2InProgress)
            && !observe::isObserveOrderNewer(obs.lastObserveValue, obs.lastReceivedTime, obsVal, simTime()))
        return true;

    handleObservePayloadResponse(obs, header, payload, obsVal, /*fromRegistration=*/false, /*originRequestId=*/0);
    return true;
}

void CoapClient::failLogicalTransfer(uint64_t transferId, const char *reason, uint8_t coapCode)
{
    auto trIt = activeBlockTransfers.find(transferId);
    if (trIt == activeBlockTransfers.end())
        return;

    blockwise::CoapBlockRequestKey key = trIt->second.serializationKey;
    bool slotHeld = trIt->second.serializedSlotHeld;
    activeBlockTransfers.erase(trIt);

    onRequestFailed(transferId, reason, coapCode);

    if (slotHeld)
        releaseTransferSlotAndPumpQueue(key);
}

void CoapClient::completeLogicalTransfer(uint64_t transferId, const CoapPeerEndpoint& peer,
        const Ptr<const CoapHeader>& finalHeader, const std::vector<uint8_t>& finalPayload)
{
    auto trIt = activeBlockTransfers.find(transferId);
    if (trIt == activeBlockTransfers.end())
        return;

    blockwise::CoapBlockRequestKey key = trIt->second.serializationKey;
    bool slotHeld = trIt->second.serializedSlotHeld;
    activeBlockTransfers.erase(trIt);

    onRequestCompleted(transferId, peer, finalHeader, finalPayload);

    if (slotHeld)
        releaseTransferSlotAndPumpQueue(key);
}

void CoapClient::dispatchMatchedResponse(uint64_t requestId, const CoapPeerEndpoint& peer,
        const Ptr<const CoapHeader>& responseHeader, const std::vector<uint8_t>& payload)
{
    if (ignoredObserveBlock2SubRequests.erase(requestId) > 0)
        return;

    if (pendingObserveBlock2SubRequests.find(requestId) != pendingObserveBlock2SubRequests.end()) {
        handleObserveBlock2Response(requestId, peer, responseHeader, payload);
        return;
    }

    auto mapIt = subRequestToTransfer.find(requestId);
    if (mapIt == subRequestToTransfer.end()) {
        onRequestCompleted(requestId, peer, responseHeader, payload);
        return;
    }

    uint64_t transferId = mapIt->second;
    subRequestToTransfer.erase(mapIt);

    auto trIt = activeBlockTransfers.find(transferId);
    if (trIt == activeBlockTransfers.end())
        return;

    blockwise::CoapClientBlockTransfer& t = trIt->second;
    uint8_t code = responseHeader->getCode();

    bool hasBlock1 = false;
    blockwise::CoapBlockOption b1;
    bool hasBlock2 = false;
    blockwise::CoapBlockOption b2;
    bool hasSize1 = false;
    uint32_t size1Val = 0;
    std::vector<std::vector<uint8_t>> etags;

    for (size_t i = 0; i < responseHeader->getOptionsArraySize(); ++i) {
        const auto& opt = responseHeader->getOptions(i);
        uint16_t num = opt.getNumber();
        if (num == COAP_OPT_BLOCK1) {
            if (hasBlock1 || blockwise::decodeCoapBlockOption(opt, b1) != blockwise::CoapBlockDecodeStatus::OK) {
                failLogicalTransfer(transferId, "Invalid Block1 option in server response");
                return;
            }
            hasBlock1 = true;
        }
        else if (num == COAP_OPT_BLOCK2) {
            if (hasBlock2 || blockwise::decodeCoapBlockOption(opt, b2) != blockwise::CoapBlockDecodeStatus::OK) {
                failLogicalTransfer(transferId, "Invalid Block2 option in server response");
                return;
            }
            hasBlock2 = true;
        }
        else if (num == COAP_OPT_SIZE1) {
            if (!decodeCoapUintValue(opt, size1Val, 4)) {
                failLogicalTransfer(transferId, "Invalid Size1 option in server response");
                return;
            }
            hasSize1 = true;
        }
        else if (num == COAP_OPT_ETAG) {
            std::vector<uint8_t> val(opt.getValueArraySize());
            for (size_t j = 0; j < val.size(); ++j)
                val[j] = opt.getValue(j);
            etags.push_back(std::move(val));
        }
    }

    if (code == COAP_CODE_CONTINUE) {
        if (!t.uploadActive || t.uploadDone || !hasBlock1 || b1.szx > t.uploadSzx || b1.byteOffset() != t.uploadOffset) {
            failLogicalTransfer(transferId, "Unexpected or inconsistent 2.31 Continue response", code);
            return;
        }

        uint32_t sentBs = 1U << (t.uploadSzx + 4U);
        size_t sentLen = std::min<size_t>(sentBs, t.uploadBody.size() - static_cast<size_t>(t.uploadOffset));
        t.uploadOffset += sentLen;
        t.uploadSzx = b1.szx;

        if (t.uploadOffset >= t.uploadBody.size()) {
            failLogicalTransfer(transferId, "2.31 Continue received after final Block1 segment", code);
            return;
        }

        if (!t.serializedSlotHeld) {
            t.serializedSlotHeld = true;
            activeKeyToTransfer[t.serializationKey] = transferId;
        }

        if (!sendNextTransferStep(t))
            failLogicalTransfer(transferId, "Failed to transmit next Block1 chunk");
        return;
    }

    if (code == COAP_CODE_REQUEST_ENTITY_TOO_LARGE) {
        uint8_t prefSzx = 6;
        blockwise::szxFromBlockSize(static_cast<uint32_t>(preferredBlockSize), prefSzx);
        uint8_t curSzx = t.sentBlock1InLastExchange ? t.uploadSzx : prefSzx;
        bool acceptableSize = (!hasSize1 || size1Val >= t.uploadBody.size());
        bool validSmallerSzx = hasBlock1 && ((t.sentBlock1InLastExchange && b1.szx < curSzx)
                || (!t.sentBlock1InLastExchange && b1.szx <= curSzx
                        && t.uploadBody.size() > (1ULL << (b1.szx + 4U))));
        if (!t.uploadDone && !t.uploadRestartedOn413 && t.hasUploadBody && acceptableSize && validSmallerSzx) {
            t.uploadRestartedOn413 = true;
            t.uploadActive = true;
            t.uploadOffset = 0;
            t.uploadSzx = b1.szx;
            if (!t.serializedSlotHeld) {
                t.serializedSlotHeld = true;
                activeKeyToTransfer[t.serializationKey] = transferId;
            }
            if (!sendNextTransferStep(t))
                failLogicalTransfer(transferId, "Failed to restart Block1 upload after 4.13");
            return;
        }
        completeLogicalTransfer(transferId, peer, responseHeader, payload);
        return;
    }

    if (code < 128 && hasBlock2) {
        t.uploadDone = true;
        if (b2.byteOffset() != t.expectedNextOffset) {
            failLogicalTransfer(transferId, "Unexpected Block2 offset from server");
            return;
        }

        if (t.downloadActive && t.expectedNextOffset > 0) {
            if (etags.empty()) {
                failLogicalTransfer(transferId, "Multi-block Block2 response missing ETag");
                return;
            }
            if (etags[0] != t.downloadEtag) {
                if (!t.restartedOnEtagMismatch && t.methodCode == COAP_CODE_GET) {
                    t.restartedOnEtagMismatch = true;
                    t.downloadBuffer.clear();
                    t.expectedNextOffset = 0;
                    t.hasDownloadEtag = false;
                    t.downloadEtag.clear();
                    t.downloadActive = true;
                    if (!t.serializedSlotHeld) {
                        t.serializedSlotHeld = true;
                        activeKeyToTransfer[t.serializationKey] = transferId;
                    }
                    if (!sendNextTransferStep(t))
                        failLogicalTransfer(transferId, "Failed to restart Block2 download on ETag change");
                    return;
                }
                failLogicalTransfer(transferId, "Block2 representation ETag changed repeatedly during download");
                return;
            }
        }
        else {
            if (b2.more && etags.empty()) {
                failLogicalTransfer(transferId, "Multi-block Block2 response missing ETag");
                return;
            }
            if (!etags.empty()) {
                t.hasDownloadEtag = true;
                t.downloadEtag = etags[0];
            }
        }

        if (b2.more && payload.size() != b2.blockSize()) {
            failLogicalTransfer(transferId, "Intermediate Block2 chunk size mismatch");
            return;
        }
        if (t.downloadBuffer.size() + payload.size() > static_cast<size_t>(maxIncomingBlockSize)) {
            emit(capacityRejectedSignal, 1L);
            failLogicalTransfer(transferId, "Assembled Block2 payload exceeds maxIncomingBlockSize");
            return;
        }

        t.downloadBuffer.insert(t.downloadBuffer.end(), payload.begin(), payload.end());
        t.expectedNextOffset += payload.size();
        t.downloadSzx = std::min(t.downloadSzx, b2.szx);

        if (b2.more) {
            t.downloadActive = true;
            if (!t.serializedSlotHeld) {
                t.serializedSlotHeld = true;
                activeKeyToTransfer[t.serializationKey] = transferId;
            }
            if (!sendNextTransferStep(t))
                failLogicalTransfer(transferId, "Failed to request next Block2 chunk");
            return;
        }

        std::vector<uint8_t> assembled = std::move(t.downloadBuffer);
        completeLogicalTransfer(transferId, peer, responseHeader, assembled);
        return;
    }

    completeLogicalTransfer(transferId, peer, responseHeader, payload);
}

void CoapClient::dispatchRequestFailed(uint64_t requestId, const char *reason, uint8_t coapCode)
{
    if (ignoredObserveBlock2SubRequests.erase(requestId) > 0)
        return;

    if (pendingObserveBlock2SubRequests.find(requestId) != pendingObserveBlock2SubRequests.end()) {
        handleObserveBlock2FailureOrAbort(requestId, false, reason, coapCode);
        return;
    }

    auto mapIt = subRequestToTransfer.find(requestId);
    if (mapIt == subRequestToTransfer.end()) {
        onRequestFailed(requestId, reason, coapCode);
        return;
    }
    uint64_t transferId = mapIt->second;
    subRequestToTransfer.erase(mapIt);
    failLogicalTransfer(transferId, reason, coapCode);
}

void CoapClient::dispatchRequestAborted(uint64_t requestId)
{
    if (ignoredObserveBlock2SubRequests.erase(requestId) > 0)
        return;

    if (pendingObserveBlock2SubRequests.find(requestId) != pendingObserveBlock2SubRequests.end()) {
        handleObserveBlock2FailureOrAbort(requestId, true, "Observe Block2 request aborted", 0);
        return;
    }

    auto mapIt = subRequestToTransfer.find(requestId);
    if (mapIt == subRequestToTransfer.end()) {
        onRequestAborted(requestId);
        return;
    }
    uint64_t transferId = mapIt->second;
    subRequestToTransfer.erase(mapIt);

    auto trIt = activeBlockTransfers.find(transferId);
    if (trIt != activeBlockTransfers.end()) {
        blockwise::CoapBlockRequestKey key = trIt->second.serializationKey;
        bool slotHeld = trIt->second.serializedSlotHeld;
        activeBlockTransfers.erase(trIt);
        if (slotHeld)
            activeKeyToTransfer.erase(key);
        onRequestAborted(transferId);
    }
}

void CoapClient::onRequestCompleted(uint64_t requestId, const CoapPeerEndpoint& peer,
        const Ptr<const CoapHeader>& responseHeader, const std::vector<uint8_t>& payload)
{
    simtime_t latency = SIMTIME_ZERO;
    simtime_t startedAt = simTime();
    bool hasStart = false;
    auto startIt = activeRequestStarts.find(requestId);
    if (startIt != activeRequestStarts.end()) {
        startedAt = startIt->second;
        latency = simTime() - startedAt;
        hasStart = true;
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
                if ((!wasEstablished && !obs.block2InProgress)
                        || observe::isObserveOrderNewer(obs.lastObserveValue, obs.lastReceivedTime, obsVal, simTime())) {
                    if (hasStart)
                        activeRequestStarts[requestId] = startedAt;
                    handleObservePayloadResponse(obs, responseHeader, payload, obsVal,
                            /*fromRegistration=*/true, /*originRequestId=*/requestId);
                    return;
                }
                emit(requestCompletedSignal, 1L);
                emit(requestLatencySignal, latency);
                return;
            }
            if (obs.established || obs.block2InProgress) {
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
