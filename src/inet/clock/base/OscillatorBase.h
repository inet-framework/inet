//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OSCILLATORBASE_H
#define __INET_OSCILLATORBASE_H

#include "inet/clock/contract/IClock.h"
#include "inet/clock/contract/IOscillator.h"
#include "inet/common/SimpleModule.h"

namespace inet {

class INET_API OscillatorBase : public SimpleModule, public IOscillator
{
  protected:
    cMessage *tickTimer = nullptr;

    uint64_t numTicks = -1;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void handleTickTimer();
    virtual void scheduleTickTimer() = 0;

  public:
    virtual ~OscillatorBase() { cancelAndDelete(tickTimer); }
    virtual std::string resolveDirective(char directive) const override;
};

} // namespace inet

#endif
