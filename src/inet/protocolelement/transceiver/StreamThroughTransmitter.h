//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
    virtual void endTx();
    virtual void abortTx() override;

    virtual void scheduleBufferUnderrunTimer();
    virtual void scheduleTxEndTimer(Signal *signal);

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

