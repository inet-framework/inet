//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
