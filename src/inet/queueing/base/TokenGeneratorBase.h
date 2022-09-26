//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TOKENGENERATORBASE_H
#define __INET_TOKENGENERATORBASE_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IPacketProcessor.h"
#include "inet/queueing/contract/ITokenStorage.h"

namespace inet {
namespace queueing {

class INET_API TokenGeneratorBase : public PacketProcessorBase
{
  public:
    static simsignal_t tokensCreatedSignal;

  protected:
    ModuleRefByPar<ITokenStorage> storage;
    int numTokensGenerated = -1;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }
    virtual std::string resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet

#endif

