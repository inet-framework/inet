// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_AODVEXTENSIONS_H
#define __INET_AODVEXTENSIONS_H

#include "inet/routing/aodv/AodvControlPackets_m.h"

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

} // namespace aodv
} // namespace inet

#endif
