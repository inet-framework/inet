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

#ifndef INET_TRANSPORTLAYER_QUIC_PACKET_QUICMAXDATAFRAME_H_
#define INET_TRANSPORTLAYER_QUIC_PACKET_QUICMAXDATAFRAME_H_

#include "QuicFrame.h"
#include "../Connection.h"

namespace inet {
namespace quic {

class QuicMaxDataFrame: public QuicFrame{

public:
    QuicMaxDataFrame(Connection *connection, uint64_t maxReceiveOffset);

    virtual ~QuicMaxDataFrame();

    virtual void setHeader(Ptr<const FrameHeader> header) override;
    virtual void onFrameLost() override;

    virtual void setConnection(Connection *connection) {
        this->connection = connection;
    }

private:
    uint64_t maxReceiveOffset = 0;
    Connection *connection;

    Ptr<const MaxDataFrameHeader> maxDataHeader;
    Ptr<MaxDataFrameHeader> createHeader();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_PACKET_QUICMAXDATAFRAME_H_ */
