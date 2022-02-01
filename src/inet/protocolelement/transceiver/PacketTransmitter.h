//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETTRANSMITTER_H
#define __INET_PACKETTRANSMITTER_H

#include "inet/protocolelement/transceiver/base/PacketTransmitterBase.h"

namespace inet {

class INET_API PacketTransmitter : public PacketTransmitterBase
{
  protected:
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void startTx(Packet *packet);
    virtual void endTx();

    virtual void scheduleTxEndTimer(Signal *signal);

  public:
    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }
};

} // namespace inet

#endif

