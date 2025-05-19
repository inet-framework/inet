//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "QuicStreamFrame.h"

namespace inet {
namespace quic {

QuicStreamFrame::QuicStreamFrame(Stream *stream) {
    this->setStream(stream);
}

QuicStreamFrame::~QuicStreamFrame() { }

void QuicStreamFrame::setHeader(Ptr<const FrameHeader> header)
{
    QuicFrame::setHeader(header);
    streamHeader = dynamicPtrCast<const StreamFrameHeader>(header);
}

size_t QuicStreamFrame::getDataSize()
{
    return streamHeader->getLength();
}

Ptr<const Chunk> QuicStreamFrame::getData()
{
    return stream->getDataToSend(streamHeader->getOffset(), streamHeader->getLength());
}

void QuicStreamFrame::setData(Ptr<const Chunk> data)
{
    throw cRuntimeError("It makes no sense to set data to a QuicStreamFrame. It fetches the data from the stream.");
}

void QuicStreamFrame::onFrameLost()
{
    stream->streamDataLost(streamHeader->getOffset(), streamHeader->getLength());
}

void QuicStreamFrame::onFrameAcked()
{
    stream->streamDataAcked(streamHeader->getOffset(), streamHeader->getLength());
}

bool QuicStreamFrame::equals(const QuicFrame *other)
{
    const QuicStreamFrame *otherStreamFrame = dynamic_cast<const QuicStreamFrame*>(other);
    if (otherStreamFrame != nullptr
     && stream == otherStreamFrame->stream
     && streamHeader->getOffset() == otherStreamFrame->streamHeader->getOffset()
     && streamHeader->getLength() == otherStreamFrame->streamHeader->getLength()) {
        return true;
    }

    return false;
}

} /* namespace quic */
} /* namespace inet */
