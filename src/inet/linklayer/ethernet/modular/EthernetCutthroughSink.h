//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETCUTTHROUGHSINK_H
#define __INET_ETHERNETCUTTHROUGHSINK_H

#include "inet/protocolelement/common/PacketStreamer.h"

namespace inet {

using namespace inet::queueing;

class INET_API EthernetCutthroughSink : public PacketStreamer
{
  protected:
    cGate *cutthroughInputGate = nullptr;
    IActivePacketSource *cutthroughProducer = nullptr;

    bool cutthrough = false;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;

    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace inet

#endif

