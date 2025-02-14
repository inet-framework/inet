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
    return streamHeader->getLength().getIntValue();
}

Ptr<const Chunk> QuicStreamFrame::getData()
{
    return stream->getDataToSend(streamHeader->getOffset().getIntValue(), streamHeader->getLength().getIntValue());
}

void QuicStreamFrame::setData(Ptr<const Chunk> data)
{
    throw cRuntimeError("It makes no sense to set data to a QuicStreamFrame. It fetches the data from the stream.");
}

void QuicStreamFrame::onFrameLost()
{
    stream->streamDataLost(streamHeader->getOffset().getIntValue(), streamHeader->getLength().getIntValue());
}

void QuicStreamFrame::onFrameAcked()
{
    stream->streamDataAcked(streamHeader->getOffset().getIntValue(), streamHeader->getLength().getIntValue());
}

bool QuicStreamFrame::equals(const QuicFrame *other)
{
    const QuicStreamFrame *otherStreamFrame = dynamic_cast<const QuicStreamFrame*>(other);
    if (otherStreamFrame != nullptr
     && stream == otherStreamFrame->stream
     && streamHeader->getOffset().getIntValue() == otherStreamFrame->streamHeader->getOffset().getIntValue()
     && streamHeader->getLength().getIntValue() == otherStreamFrame->streamHeader->getLength().getIntValue()) {
        return true;
    }

    return false;
}

} /* namespace quic */
} /* namespace inet */
