//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SERVOCLOCKBASE_H
#define __INET_SERVOCLOCKBASE_H

#include "inet/clock/model/OscillatorBasedClock.h"
//#include "inet/common/scenario/IScriptable.h"

namespace inet {

class ServoClockBase : public OscillatorBasedClock {

protected:
    OverdueClockEventHandlingMode defaultOverdueClockEventHandlingMode = UNSPECIFIED;
    ppm oscillatorCompensation = ppm(0); // 0 means no compensation, higher value means faster clock, e.g. 100 ppm value means the clock compensates 100 microseconds for every second in clock time
    // 100 ppm value means the oscillator tick length is compensated to be smaller by a factor of (1 / (1 + 100 / 1E+6)) than the actual tick length measured in clock time


public:
    virtual void adjustClockTime(clocktime_t newClockTime) {};
    virtual void setOscillatorCompensation(ppm oscillatorCompensation);
    virtual void resetOscillatorOffset() const;
    virtual void rescheduleClockEvents(clocktime_t oldClockTime, clocktime_t newClockTime);

};

} /* namespace inet */

#endif
