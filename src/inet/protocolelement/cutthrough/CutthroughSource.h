//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CUTTHROUGHSOURCE_H
#define __INET_CUTTHROUGHSOURCE_H

#include "inet/common/packet/chunk/StreamBufferChunk.h"
#include "inet/protocolelement/common/PacketDestreamer.h"

namespace inet {

using namespace inet::queueing;

class INET_API CutthroughSource : public PacketDestreamer
{
  protected:
    b cutthroughPosition;
    cMessage *cutthroughTimer = nullptr;

    Ptr<StreamBufferChunk> cutthroughBuffer = nullptr;
  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

  public:
    virtual ~CutthroughSource();

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif

