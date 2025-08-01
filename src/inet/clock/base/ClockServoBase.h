//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CLOCKSERVOBASE_H
#define __INET_CLOCKSERVOBASE_H

#include "inet/clock/contract/IClockServo.h"
#include "inet/clock/model/SettableClock.h"
#include "inet/common/SimpleModule.h"

namespace inet {

class INET_API ClockServoBase : public SimpleModule, public IClockServo
{
  protected:
    SettableClock *clock = nullptr;

  protected:
    virtual void initialize(int stage) override;
};

} // namespace inet

#endif

