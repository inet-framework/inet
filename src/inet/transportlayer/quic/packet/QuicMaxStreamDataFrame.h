//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_PACKET_QUICMAXSTREAMDATAFRAME_H_
#define INET_TRANSPORTLAYER_QUIC_PACKET_QUICMAXSTREAMDATAFRAME_H_

#include "QuicFrame.h"
#include "../stream/Stream.h"

namespace inet {
namespace quic {

class QuicMaxStreamDataFrame: public QuicFrame {
public:

    QuicMaxStreamDataFrame(Stream *stream, uint64_t maxReceiveOffset);

    virtual ~QuicMaxStreamDataFrame();

    virtual void setHeader(Ptr<const FrameHeader> header) override;
    virtual void onFrameLost() override;


    virtual void setStream(Stream *stream) {
        this->stream = stream;
    }

private:
    uint64_t streamId = 0;
    uint64_t maxReceiveOffset = 0;

    Stream *stream;
    Ptr<const MaxStreamDataFrameHeader> maxStreamDataHeader;
    Ptr<MaxStreamDataFrameHeader> createHeader();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_PACKET_QUICMAXSTREAMDATAFRAME_H_ */
