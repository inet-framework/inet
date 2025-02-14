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
