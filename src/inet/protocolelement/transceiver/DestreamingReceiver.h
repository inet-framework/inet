//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DESTREAMINGRECEIVER_H
#define __INET_DESTREAMINGRECEIVER_H

#include "inet/protocolelement/transceiver/base/StreamingReceiverBase.h"

namespace inet {

class INET_API DestreamingReceiver : public StreamingReceiverBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;

    virtual void sendToUpperLayer(Packet *packet);

    virtual void receivePacketStart(cPacket *packet, cGate *gate, bps datarate);
    virtual void receivePacketEnd(cPacket *packet, cGate *gate, bps datarate);
    virtual void receivePacketProgress(cPacket *packet, cGate *gate, bps datarate, b position, simtime_t timePosition, b extraProcessableLength, simtime_t extraProcessableDuration);

  public:
    virtual bool supportsPacketStreaming(cGate *gate) const override { return gate == inputGate; }
};

} // namespace inet

#endif

