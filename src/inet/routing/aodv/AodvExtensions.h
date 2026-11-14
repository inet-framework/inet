// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_AODVEXTENSIONS_H
#define __INET_AODVEXTENSIONS_H

#include "inet/routing/aodv/AodvControlPackets_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"

namespace inet {
namespace aodv {

/** RFC 3561 sections 9 and 9.1. Unknown skippable records remain on the message. */
template<typename T>
bool readAodvExtensions(const T& message, uint32_t& helloIntervalMilliseconds, unsigned int subnetType = 0)
{
    helloIntervalMilliseconds = 0;
    size_t size = message.getExtensionsArraySize();
    if (size > 65535)
        return false;
    for (size_t offset = 0; offset < size;) {
        if (size - offset < 2)
            return false;
        unsigned int type = message.getExtensions(offset++);
        unsigned int length = message.getExtensions(offset++);
        if (type == 0 || (type >= 128 && type != subnetType) || length > size - offset)
            return false;
        if (type == 1) {
            if (length != 4 || helloIntervalMilliseconds != 0)
                return false;
            uint32_t interval = 0;
            for (unsigned int i = 0; i < 4; ++i)
                interval = (interval << 8) | message.getExtensions(offset + i);
            if (interval == 0)
                return false;
            helloIntervalMilliseconds = interval;
        }
        offset += length;
    }
    return true;
}

inline bool readAodvSubnetFloor(const Rreq& message, unsigned int type, int& prefixLength, uint32_t& sequence)
{
    prefixLength = -1;
    sequence = 0;
    if (!type)
        return true;
    for (size_t offset = 0; offset < message.getExtensionsArraySize();) {
        if (message.getExtensionsArraySize() - offset < 2)
            return false;
        unsigned int recordType = message.getExtensions(offset++);
        unsigned int length = message.getExtensions(offset++);
        if (length > message.getExtensionsArraySize() - offset)
            return false;
        if (recordType == type) {
            if (prefixLength != -1 || length != 5)
                return false;
            prefixLength = message.getExtensions(offset);
            if (prefixLength <= 0 || prefixLength >= message.getDestAddr().getAddressType()->getMaxPrefixLength() ||
                    !message.getUnknownSeqNumFlag() || !message.getDestOnlyFlag() || message.getDestSeqNum() != 0)
                return false;
            for (unsigned int i = 0; i < 4; ++i)
                sequence = (sequence << 8) | message.getExtensions(offset + 1 + i);
        }
        offset += length;
    }
    return true;
}

inline void writeAodvSubnetFloor(const Ptr<Rreq>& message, unsigned int type, int prefixLength, uint32_t sequence)
{
    size_t offset = 0;
    while (offset < message->getExtensionsArraySize()) {
        if (message->getExtensions(offset) == type)
            break;
        offset += 2 + message->getExtensions(offset + 1);
    }
    if (offset == message->getExtensionsArraySize()) {
        message->setExtensionsArraySize(offset + 7);
        message->setChunkLength(message->getChunkLength() + B(7));
    }
    message->setExtensions(offset, type);
    message->setExtensions(offset + 1, 5);
    message->setExtensions(offset + 2, prefixLength);
    for (unsigned int i = 0; i < 4; ++i)
        message->setExtensions(offset + 3 + i, sequence >> (24 - 8 * i));
    message->setUnknownSeqNumFlag(true);
    message->setDestOnlyFlag(true);
    message->setDestSeqNum(0);
}

} // namespace aodv
} // namespace inet

#endif
