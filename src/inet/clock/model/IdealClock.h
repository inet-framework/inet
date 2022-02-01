//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDEALCLOCK_H
#define __INET_IDEALCLOCK_H

#include "inet/clock/base/ClockBase.h"

namespace inet {

class INET_API IdealClock : public ClockBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;

  public:
    virtual clocktime_t computeClockTimeFromSimTime(simtime_t t) const override;
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t t) const override;
};

} // namespace inet

#endif

