//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QUERYPROBE_H
#define __INET_QUERYPROBE_H

#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

/**
 * Passes packets through, repeats the queries that pass through it, and checks
 * that a pulled packet is the packet that the provider offered before the pull.
 * See the NED documentation.
 */
class QueryProbe : public queueing::PacketFlowBase
{
  protected:
    int numRepeats = -1;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override;

    virtual bool canPullSomePacket(const cGate *gate) const override;
    virtual Packet *canPullPacket(const cGate *gate) const override;
    virtual Packet *pullPacket(const cGate *gate) override;
};

} // namespace inet

#endif

