//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SERVOCLOCKBASE_H
#define __INET_SERVOCLOCKBASE_H

#include "inet/clock/model/OscillatorBasedClock.h"
#include "inet/common/scenario/IScriptable.h"
#include "inet/common/XMLUtils.h"

#include "inet/clock/oscillator/ConstantDriftOscillator.h"

namespace inet {

class ServoClockBase : public OscillatorBasedClock, public IScriptable {

protected:
    OverdueClockEventHandlingMode defaultOverdueClockEventHandlingMode = UNSPECIFIED;
    ppm oscillatorCompensation = ppm(0); // 0 means no compensation, higher value means faster clock, e.g. 100 ppm value means the clock compensates 100 microseconds for every second in clock time
    // 100 ppm value means the oscillator tick length is compensated to be smaller by a factor of (1 / (1 + 100 / 1E+6)) than the actual tick length measured in clock time

protected:
    virtual void rescheduleClockEvents(clocktime_t oldClockTime, clocktime_t newClockTime);
    virtual simtime_t handleOverdueClockEvent(ClockEvent *event, simtime_t t);
    virtual void initialize(int stage) override;
    virtual void processCommand(const cXMLElement& node) override;
public:
    virtual void adjustClockTime(clocktime_t newClockTime) = 0;
    virtual void setOscillatorCompensation(ppm oscillatorCompensationValue);
    virtual void resetOscillator() const;


};

} /* namespace inet */

#endif
