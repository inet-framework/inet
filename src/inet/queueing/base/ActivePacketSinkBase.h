//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ACTIVEPACKETSINKBASE_H
#define __INET_ACTIVEPACKETSINKBASE_H

#include "inet/common/ModuleRefByGate.h"
#include "inet/queueing/base/PacketSinkBase.h"
#include "inet/queueing/contract/IActivePacketSink.h"

namespace inet {
namespace queueing {

class INET_API ActivePacketSinkBase : public PacketSinkBase, public virtual IActivePacketSink
{
  protected:
    cGate *inputGate = nullptr;
    ModuleRefByGate<IPassivePacketSource> provider;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual IPassivePacketSource *getProvider(const cGate *gate) override { return provider; }

    virtual bool supportsPacketPushing(const cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return inputGate == gate; }
};

} // namespace queueing
} // namespace inet

#endif

