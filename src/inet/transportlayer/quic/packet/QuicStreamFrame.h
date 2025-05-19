//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
