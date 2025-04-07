//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OSCILLATORBASE_H
#define __INET_OSCILLATORBASE_H

#include "inet/clock/contract/IOscillator.h"
#include "inet/common/SimpleModule.h"

namespace inet {

class INET_API OscillatorBase : public SimpleModule, public IOscillator
{
  public:
    static simsignal_t driftRateChangedSignal;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
  public:
    virtual std::string resolveDirective(char directive) const override;
};

} // namespace inet

#endif
