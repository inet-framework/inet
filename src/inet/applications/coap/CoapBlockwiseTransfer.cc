//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/coap/CoapBlockwiseTransfer.h"
#include "inet/applications/coap/CoapHeaderSerializer.h"

namespace inet {
namespace coap {
namespace blockwise {

bool CoapBlockRequestKey::operator<(const CoapBlockRequestKey& other) const
{
    if (peer != other.peer)
        return peer < other.peer;
    if (methodCode != other.methodCode)
        return methodCode < other.methodCode;
    if (pathSegments != other.pathSegments)
        return pathSegments < other.pathSegments;
    if (uriQueries != other.uriQueries)
        return uriQueries < other.uriQueries;
    if (contentFormat != other.contentFormat)
        return contentFormat < other.contentFormat;
    if (acceptFormat != other.acceptFormat)
        return acceptFormat < other.acceptFormat;
    if (hasIfNoneMatch != other.hasIfNoneMatch)
        return hasIfNoneMatch < other.hasIfNoneMatch;
    if (ifMatchValues != other.ifMatchValues)
        return ifMatchValues < other.ifMatchValues;
    return requestTags < other.requestTags;
}

CoapBlockDecodeStatus decodeCoapBlockOption(const CoapOption& opt, CoapBlockOption& out)
{
    uint32_t raw = 0;
    if (!decodeCoapUintValue(opt, raw, 3))
        return CoapBlockDecodeStatus::BAD_FORMAT;

    out.szx = static_cast<uint8_t>(raw & 0x07U);
    out.more = (raw & 0x08U) != 0U;
    out.num = raw >> 4U;

    if (out.szx == 7U)
        return CoapBlockDecodeStatus::RESERVED_SZX_7;

    return CoapBlockDecodeStatus::OK;
}

uint32_t encodeCoapBlockValue(const CoapBlockOption& opt)
{
    return ((opt.num & 0xFFFFFU) << 4U) | (opt.more ? 0x08U : 0U) | (opt.szx & 0x07U);
}

bool szxFromBlockSize(uint32_t blockSize, uint8_t& szxOut)
{
    switch (blockSize) {
        case 16U:
            szxOut = 0;
            return true;
        case 32U:
            szxOut = 1;
            return true;
        case 64U:
            szxOut = 2;
            return true;
        case 128U:
            szxOut = 3;
            return true;
        case 256U:
            szxOut = 4;
            return true;
        case 512U:
            szxOut = 5;
            return true;
        case 1024U:
            szxOut = 6;
            return true;
        default:
            return false;
    }
}

} // namespace blockwise
} // namespace coap
} // namespace inet
