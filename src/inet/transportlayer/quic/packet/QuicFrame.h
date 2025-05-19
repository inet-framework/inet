//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_QUICFRAME_H_
#define INET_APPLICATIONS_QUIC_QUICFRAME_H_

#include "FrameHeader_m.h"

namespace inet {
namespace quic {

class Stream;

class QuicFrame {
public:
    QuicFrame();
    QuicFrame(Ptr<const FrameHeader> header);
    QuicFrame(Ptr<const FrameHeader> header, Ptr<const Chunk> data);
    virtual ~QuicFrame();

    virtual void setHeader(Ptr<const FrameHeader> header);
    virtual FrameHeaderType getType();
    virtual size_t getSize();
    virtual size_t getDataSize();

    virtual bool equals(const QuicFrame *other);
    virtual int getMemorySize();

    virtual Ptr<const FrameHeader> getHeader() {
        return header;
    }
    virtual Ptr<const Chunk> getData() {
        return data;
    }
    virtual void setData(Ptr<const Chunk> data) {
        hasdata = true;
        this->data = data;
    }
    virtual bool hasData() {
        return hasdata;
    }
    virtual bool isAckEliciting() {
        return ackEliciting;
    }
    virtual bool countsAsInFlight() {
        return countsInFlight;
    }
    virtual void onFrameLost() { };
    virtual void onFrameAcked() { };

private:
    Ptr<const FrameHeader> header = nullptr;
    Ptr<const Chunk> data = nullptr;
    bool hasdata = false;
    bool ackEliciting = false;
    bool countsInFlight = false;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_QUICFRAME_H_ */
