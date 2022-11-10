//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BACKPRESSUREBARRIER_H
#define __INET_BACKPRESSUREBARRIER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {
namespace queueing {

class INET_API BackPressureBarrier : public PacketFlowBase, public TransparentProtocolRegistrationListener
{
  protected:
    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

  public:
    virtual void processPacket(Packet *packet) override {}

    virtual bool canPushSomePacket(cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return true; }

    virtual bool canPullSomePacket(cGate *gate) const override { return true; }
    virtual Packet* canPullPacket(cGate *gate) const override;
};

} // namespace queueing
} // namespace inet

#endif

