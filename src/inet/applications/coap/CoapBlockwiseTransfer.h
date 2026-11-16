//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_COAP_COAPBLOCKWISETRANSFER_H
#define __INET_COAP_COAPBLOCKWISETRANSFER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "inet/applications/coap/CoapEndpointBase.h"
#include "inet/applications/coap/CoapHeader_m.h"
#include "inet/common/INETDefs.h"

namespace inet {
namespace coap {
namespace blockwise {

enum class CoapBlockDecodeStatus {
    OK,
    BAD_FORMAT,
    RESERVED_SZX_7
};

struct INET_API CoapBlockOption
{
    uint32_t num = 0;
    bool more = false;
    uint8_t szx = 0;

    uint32_t blockSize() const { return (szx <= 6U) ? (1U << (szx + 4U)) : 0U; }
    uint64_t byteOffset() const { return static_cast<uint64_t>(num) * blockSize(); }
};

struct INET_API CoapBlockRequestKey
{
    CoapPeerEndpoint peer;
    uint8_t methodCode = COAP_CODE_GET;
    std::vector<std::string> pathSegments;
    std::vector<std::string> uriQueries;
    int32_t contentFormat = -1;
    int32_t acceptFormat = -1;
    std::vector<std::vector<uint8_t>> ifMatchValues;
    bool hasIfNoneMatch = false;
    std::vector<std::vector<uint8_t>> requestTags;

    bool operator<(const CoapBlockRequestKey& other) const;
};

struct INET_API CoapServerBlock1Context
{
    CoapBlockRequestKey key;
    std::vector<uint8_t> buffer;
    bool hasSize1 = false;
    uint32_t size1 = 0;
    bool hasContentFormat = false;
    int32_t contentFormat = -1;
    simtime_t expiresAt = SIMTIME_ZERO;
};

struct INET_API CoapServerRetainedBlock2Context
{
    CoapBlockRequestKey key;
    uint8_t responseCode = COAP_CODE_CHANGED;
    int32_t contentFormat = -1;
    uint32_t maxAge = 60;
    bool explicitMaxAge = false;
    std::vector<uint8_t> etag;
    std::vector<uint8_t> payload;
    simtime_t expiresAt = SIMTIME_ZERO;
};

struct INET_API CoapClientBlockTransfer
{
    uint64_t transferId = 0;
    CoapPeerEndpoint peer;
    bool peerIsMulticast = false;
    bool isConfirmable = true;
    uint8_t methodCode = COAP_CODE_GET;
    std::string uriStr;
    int reqContentFormat = -1;
    int reqAccept = -1;
    std::vector<CoapOption> baseExtraOptions;
    CoapBlockRequestKey serializationKey;
    bool serializedSlotHeld = false;
    simtime_t logicalStartTime = SIMTIME_ZERO;

    // Block1 upload tracking
    bool hasUploadBody = false;
    std::vector<uint8_t> uploadBody;
    bool uploadActive = false;
    bool uploadDone = false;
    uint64_t uploadOffset = 0;
    uint8_t uploadSzx = 6;
    bool sentBlock1InLastExchange = false;
    bool uploadRestartedOn413 = false;

    // Block2 download tracking
    bool downloadActive = false;
    uint8_t downloadSzx = 6;
    uint64_t expectedNextOffset = 0;
    bool hasDownloadEtag = false;
    std::vector<uint8_t> downloadEtag;
    bool restartedOnEtagMismatch = false;
    std::vector<uint8_t> downloadBuffer;

    // Current sub-exchange tracking
    uint64_t activeSubRequestId = 0;
};

INET_API CoapBlockDecodeStatus decodeCoapBlockOption(const CoapOption& opt, CoapBlockOption& out);
INET_API uint32_t encodeCoapBlockValue(const CoapBlockOption& opt);
INET_API bool szxFromBlockSize(uint32_t blockSize, uint8_t& szxOut);

} // namespace blockwise
} // namespace coap
} // namespace inet

#endif
