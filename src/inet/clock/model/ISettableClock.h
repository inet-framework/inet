//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_ISETTABLECLOCK_H
#define __INET_ISETTABLECLOCK_H

#include "inet/clock/model/OscillatorBasedClock.h"
#include "inet/common/scenario/IScriptable.h"

#include "inet/clock/base/ClockBase.h"

namespace inet {

class ISettableClock : public ClockBase {

protected:
    virtual void initialize(int stage) override;

public:
    virtual void setClockTime(clocktime_t newClockTime) = 0;
};

} /* namespace inet */

#endif
