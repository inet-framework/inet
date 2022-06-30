//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TIMEBASEDTOKENGENERATOR_H
#define __INET_TIMEBASEDTOKENGENERATOR_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/TokenGeneratorBase.h"

namespace inet {

extern template class ClockUserModuleMixin<queueing::TokenGeneratorBase>;

namespace queueing {

class INET_API TimeBasedTokenGenerator : public ClockUserModuleMixin<TokenGeneratorBase>
{
  protected:
    cPar *generationIntervalParameter = nullptr;
    cPar *numTokensParameter = nullptr;

    ClockEvent *generationTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void scheduleGenerationTimer();

  public:
    virtual ~TimeBasedTokenGenerator() { cancelAndDeleteClockEvent(generationTimer); }

    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }
};

} // namespace queueing
} // namespace inet

#endif

