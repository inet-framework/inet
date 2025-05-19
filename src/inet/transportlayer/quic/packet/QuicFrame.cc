//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "QuicFrame.h"

namespace inet {
namespace quic {

QuicFrame::QuicFrame() {

}

QuicFrame::QuicFrame(Ptr<const FrameHeader> header)
{
    this->setHeader(header);
}

QuicFrame::QuicFrame(Ptr<const FrameHeader> header, Ptr<const Chunk> data)
{
    this->setHeader(header);
    this->setData(data);
}

QuicFrame::~QuicFrame() { }

void QuicFrame::setHeader(Ptr<const FrameHeader> header)
{
    this->header = header;

    FrameHeaderType type = getType();
    if (type == FRAME_HEADER_TYPE_ACK) {
        ackEliciting = false;
        countsInFlight = false;
    } else if (type == FRAME_HEADER_TYPE_PADDING) {
        ackEliciting = false;
        countsInFlight = true;
    } else {
        ackEliciting = true;
        countsInFlight = true;
    }
}

size_t QuicFrame::getSize()
{
    size_t size = 0;
    if (header != nullptr) {
        size += B(header->getChunkLength()).get();
    }
    size += this->getDataSize();
    return size;
}

size_t QuicFrame::getDataSize()
{
    size_t size = 0;
    if (hasdata) {
        size += B(data->getChunkLength()).get();
    }
    return size;
}

FrameHeaderType QuicFrame::getType()
{
    return header->getFrameType();
}

bool QuicFrame::equals(const QuicFrame *other)
{
    // TODO: Implement
    return false;
}

int QuicFrame::getMemorySize()
{
    int size = sizeof(QuicFrame);
    if (header != nullptr) {
        size += sizeof(*header);
    }
    if (data != nullptr) {
        size += sizeof(*data);
    }
    return size;
}

} /* namespace quic */
} /* namespace inet */
