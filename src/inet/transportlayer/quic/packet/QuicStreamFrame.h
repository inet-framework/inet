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

#ifndef INET_APPLICATIONS_QUIC_PACKET_QUICSTREAMFRAME_H_
#define INET_APPLICATIONS_QUIC_PACKET_QUICSTREAMFRAME_H_

#include "QuicFrame.h"
#include "../stream/Stream.h"

namespace inet {
namespace quic {

class QuicStreamFrame: public QuicFrame {
public:
    QuicStreamFrame(Stream *stream);
    virtual ~QuicStreamFrame();

    virtual void setHeader(Ptr<const FrameHeader> header) override;
    virtual size_t getDataSize() override;
    virtual Ptr<const Chunk> getData() override;
    virtual void setData(Ptr<const Chunk> data) override;
    virtual void onFrameLost() override;
    virtual void onFrameAcked() override;
    virtual bool equals(const QuicFrame *other) override;
    virtual bool hasData() override {
        return true;
    }
    virtual Ptr<const StreamFrameHeader> getStreamHeader() {
        return streamHeader;
    }
    virtual void setStream(Stream *stream) {
        this->stream = stream;
    }
    virtual Stream *getStream() {
        return this->stream;
    }

private:
    Stream *stream;
    Ptr<const StreamFrameHeader> streamHeader;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_PACKET_QUICSTREAMFRAME_H_ */
