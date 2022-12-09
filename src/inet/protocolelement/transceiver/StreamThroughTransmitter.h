//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMTHROUGHTRANSMITTER_H
#define __INET_STREAMTHROUGHTRANSMITTER_H

#include "inet/protocolelement/transceiver/base/StreamingTransmitterBase.h"

namespace inet {

class INET_API StreamThroughTransmitter : public StreamingTransmitterBase
{
  protected:
    // parameters of last sent transmission progress report
    simtime_t lastTxProgressTime = -1;
    b lastTxProgressPosition = b(-1);

    // parameters of last received input progress report
    bps lastInputDatarate = bps(NaN);
    simtime_t lastInputProgressTime = -1;
    b lastInputProgressPosition = b(-1);

    cMessage *bufferUnderrunTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void startTx(Packet *packet, bps datarate, b position);
    virtual void progressTx(Packet *packet, bps datarate, b position);
    virtual void endTx(Packet *packet);
    virtual void abortTx() override;

    virtual void scheduleBufferUnderrunTimer();

  public:
    virtual ~StreamThroughTransmitter() { cancelAndDelete(bufferUnderrunTimer); }

    virtual bool supportsPacketStreaming(cGate *gate) const override { return true; }

    virtual void pushPacket(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;
};

} // namespace inet

#endif

