//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
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
