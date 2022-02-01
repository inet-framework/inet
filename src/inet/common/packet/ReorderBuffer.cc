//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/ReorderBuffer.h"

namespace inet {

b ReorderBuffer::getAvailableDataLength() const
{
    if (regions.size() > 0 && regions[0].offset == expectedOffset)
        return regions[0].data->getChunkLength();
    else
        return b(0);
}

const Ptr<const Chunk> ReorderBuffer::popAvailableData(b length)
{
    if (regions.size() > 0 && regions[0].offset == expectedOffset) {
        auto data = regions[0].data;
        b dataLength = data->getChunkLength();
        if (length != b(-1)) {
            data = data->peek(Chunk::ForwardIterator(b(0)), length);
            dataLength = length;
        }
        clear(expectedOffset, dataLength);
        expectedOffset += dataLength;
        return data;
    }
    else
        return nullptr;
}

} // namespace inet

