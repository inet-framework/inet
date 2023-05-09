//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETSERVERBASE_H
#define __INET_PACKETSERVERBASE_H

#include "inet/common/ModuleRefByGate.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IActivePacketSource.h"

namespace inet {
namespace queueing {

class INET_API PacketServerBase : public PacketProcessorBase, public virtual IActivePacketSink, public virtual IActivePacketSource
{
  protected:
    cGate *inputGate = nullptr;
    ModuleRefByGate<IPassivePacketSource> provider;

    cGate *outputGate = nullptr;
    ModuleRefByGate<IPassivePacketSink> consumer;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual IPassivePacketSource *getProvider(const cGate *gate) override { return provider; }
    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { return consumer; }

    virtual bool supportsPacketPushing(const cGate *gate) const override { return outputGate == gate; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return inputGate == gate; }

    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;
    virtual void handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;
};

} // namespace queueing
} // namespace inet

#endif

