//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMINGTRANSMITTERBASE_H
#define __INET_STREAMINGTRANSMITTERBASE_H

#include "inet/protocolelement/transceiver/base/PacketTransmitterBase.h"

namespace inet {

class INET_API StreamingTransmitterBase : public PacketTransmitterBase, public cListener
{
  protected:
    cChannel *transmissionChannel = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void scheduleAt(simtime_t t, cMessage *message) override;

    virtual void abortTx() = 0;

    virtual void scheduleTxEndTimer(Signal *signal);

  public:
    virtual bool canPushSomePacket(cGate *gate) const override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace inet

#endif

