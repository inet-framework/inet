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
