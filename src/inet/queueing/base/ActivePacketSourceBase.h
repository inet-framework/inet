//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ACTIVEPACKETSOURCEBASE_H
#define __INET_ACTIVEPACKETSOURCEBASE_H

#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/base/PacketSourceBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API ActivePacketSourceBase : public PacketSourceBase, public virtual IActivePacketSource
{
  protected:
    cGate *outputGate = nullptr;
    PassivePacketSinkRef consumer;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { return consumer; }

    virtual bool supportsPacketPushing(const cGate *gate) const override { return outputGate == gate; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return false; }
};

} // namespace queueing
} // namespace inet

#endif

