//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMTHROUGHRECEIVER_H
#define __INET_STREAMTHROUGHRECEIVER_H

#include "inet/protocolelement/transceiver/base/StreamingReceiverBase.h"

namespace inet {

class INET_API StreamThroughReceiver : public StreamingReceiverBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;

    virtual void receivePacketStart(cPacket *packet, cGate *gate, bps datarate);
    virtual void receivePacketProgress(cPacket *packet, cGate *gate, bps datarate, b position, simtime_t timePosition, b extraProcessableLength, simtime_t extraProcessableDuration);
    virtual void receivePacketEnd(cPacket *packet, cGate *gate, bps datarate);

  public:
    virtual bool supportsPacketStreaming(cGate *gate) const override { return true; }
};

} // namespace inet

#endif

