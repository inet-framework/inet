//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMINGTRANSMITTER_H
#define __INET_STREAMINGTRANSMITTER_H

#include "inet/protocolelement/transceiver/base/StreamingTransmitterBase.h"

namespace inet {

class INET_API StreamingTransmitter : public StreamingTransmitterBase
{
  protected:
    virtual void handleMessageWhenUp(cMessage *message) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void startTx(Packet *packet);
    virtual void endTx();
    virtual void abortTx() override;

  public:
    virtual bool supportsPacketStreaming(cGate *gate) const override { return gate == outputGate; }

    virtual void pushPacket(Packet *packet, cGate *gate) override;
};

} // namespace inet

#endif

