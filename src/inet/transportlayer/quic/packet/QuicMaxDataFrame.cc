//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "QuicMaxDataFrame.h"

namespace inet {
namespace quic {

QuicMaxDataFrame::QuicMaxDataFrame(Connection *connection, uint64_t maxReceiveOffset)
{
    if(maxReceiveOffset == 0){
        throw cRuntimeError("It makes no sense to create a QuicMaxDataFrame with 0 FC limit");
    }

    this->maxReceiveOffset = maxReceiveOffset;
    this->setConnection(connection);

    auto header = createHeader();
    this->setHeader(header);
}

QuicMaxDataFrame::~QuicMaxDataFrame() {
}

void QuicMaxDataFrame::setHeader(Ptr<const FrameHeader> header)
{
    QuicFrame::setHeader(header);
    maxDataHeader = dynamicPtrCast<const MaxDataFrameHeader>(header);
}

void QuicMaxDataFrame::onFrameLost()
{
    connection->onMaxDataFrameLost();
}

Ptr<MaxDataFrameHeader> QuicMaxDataFrame::createHeader()
{
    Ptr<MaxDataFrameHeader> header = makeShared<MaxDataFrameHeader>();
    header->setMaximumData(maxReceiveOffset);
    header->calcChunkLength();
    return header;

}


} /* namespace quic */
} /* namespace inet */
