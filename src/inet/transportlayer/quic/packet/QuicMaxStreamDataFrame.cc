//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "QuicMaxStreamDataFrame.h"

namespace inet {
namespace quic {

QuicMaxStreamDataFrame::QuicMaxStreamDataFrame(Stream *stream, uint64_t maxReceiveOffset)
{
    if(maxReceiveOffset == 0){
        throw cRuntimeError("It makes no sense to create a QuicMaxStreamDataFrame with 0 FC limit");
    }

    this->streamId = stream->id;
    this->maxReceiveOffset = maxReceiveOffset;
    this->setStream(stream);

    auto header = createHeader();
    this->setHeader(header);
}

QuicMaxStreamDataFrame::~QuicMaxStreamDataFrame() {
}

void QuicMaxStreamDataFrame::setHeader(Ptr<const FrameHeader> header)
{
    QuicFrame::setHeader(header);
    maxStreamDataHeader = dynamicPtrCast<const MaxStreamDataFrameHeader>(header);
}

void QuicMaxStreamDataFrame::onFrameLost()
{
    stream->onMaxStreamDataFrameLost();
}

Ptr<MaxStreamDataFrameHeader> QuicMaxStreamDataFrame::createHeader()
{
    Ptr<MaxStreamDataFrameHeader> header = makeShared<MaxStreamDataFrameHeader>();
    header->setStreamId(streamId);
    header->setMaximumStreamData(maxReceiveOffset);
    header->calcChunkLength();
    return header;

}

} /* namespace quic */
} /* namespace inet */
