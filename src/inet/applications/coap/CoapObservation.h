//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_COAPOBSERVATION_H
#define __INET_COAPOBSERVATION_H

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "inet/applications/coap/CoapEndpointBase.h"

namespace inet {
namespace coap {

/**
 * Serial-number comparison and 1ms tick helpers for RFC 7641 Observe.
 */
namespace observe {

/**
 * Returns true if candidateVal arriving at candidateTime is strictly newer than
 * currentVal received at currentTime using RFC 7641 section 3.4 24-bit arithmetic
 * and the strict 128s elapsed-time rule (Tnew > Told + 128s).
 */
INET_API bool isObserveOrderNewer(uint32_t currentVal, simtime_t currentTime,
        uint32_t candidateVal, simtime_t candidateTime);

/**
 * Converts simulation time into conservative 1ms monotonic clock ticks.
 */
INET_API uint64_t currentObserveTick(simtime_t now);

/**
 * Masks a 64-bit millisecond tick to its 24-bit RFC 7641 wire value.
 */
inline uint32_t toWireObserveValue(uint64_t tick)
{
    return static_cast<uint32_t>(tick & 0xFFFFFFULL);
}

} // namespace observe

struct INET_API CoapObserverKey
{
    CoapPeerEndpoint peer;
    std::vector<uint8_t> token;

    bool operator<(const CoapObserverKey& other) const {
        if (peer != other.peer)
            return peer < other.peer;
        return token < other.token;
    }
};

struct INET_API CoapPendingNotification
{
    uint8_t responseCode = COAP_CODE_CONTENT;
    std::vector<uint8_t> payload;
    std::vector<uint8_t> etag;
    int32_t contentFormat = -1;
    uint32_t maxAge = 60;
    bool hasMaxAge = false;
    bool includeObserve = true;
    bool terminal = false;
    simtime_t earliestSendTime = SIMTIME_ZERO;
};

struct INET_API CoapServerObserverRelation
{
    CoapObserverKey key;
    std::string resourcePath;
    std::vector<std::string> pathSegments;
    int32_t negotiatedContentFormat = -1;
    int32_t acceptFormat = -1;
    bool inFlight = false;
    uint64_t inFlightOutboundId = 0;
    bool lastSentValid = false;
    uint64_t lastSentTick = 0;
    bool hasPendingNotification = false;
    CoapPendingNotification pending;
};

struct INET_API CoapClientObserveCacheKey
{
    CoapPeerEndpoint peer;
    std::vector<std::string> pathSegments;
    std::vector<std::string> querySegments;
    int32_t acceptFormat = -1;
    int32_t block2Szx = -1;

    bool operator<(const CoapClientObserveCacheKey& other) const {
        if (peer != other.peer)
            return peer < other.peer;
        if (acceptFormat != other.acceptFormat)
            return acceptFormat < other.acceptFormat;
        if (block2Szx != other.block2Szx)
            return block2Szx < other.block2Szx;
        if (pathSegments != other.pathSegments)
            return pathSegments < other.pathSegments;
        return querySegments < other.querySegments;
    }
};

struct INET_API CoapClientObservation
{
    uint64_t relationId = 0;
    CoapObserverKey key;
    CoapClientObserveCacheKey cacheKey;
    std::string uriStr;
    std::vector<CoapOption> templateExtraOptions;
    bool confirmable = true;

    bool established = false;
    bool complete = false;
    bool hasFreshnessTimer = false;
    simtime_t reregisterAt = SIMTIME_ZERO;
    uint64_t activeRegistrationRequestId = 0;

    uint32_t lastObserveValue = 0;
    simtime_t lastReceivedTime = SIMTIME_ZERO;
    uint32_t maxAgeSeconds = 60;
    uint8_t lastResponseCode = COAP_CODE_CONTENT;
    int32_t contentFormat = -1;
    std::vector<uint8_t> etag;
    std::vector<uint8_t> payload;

    std::set<uint64_t> consumerIds;
};

} // namespace coap
} // namespace inet

#endif
