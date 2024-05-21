//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PICLOCK_H
#define __INET_PICLOCK_H

#include "inet/clock/model/OscillatorBasedClock.h"
#include "inet/clock/model/SettableClock.h"
#include "inet/common/scenario/IScriptable.h"

namespace inet {

class INET_API PiClock : public SettableClock
{
  protected:
    static simsignal_t driftSignal;
    static simsignal_t kpSignal;
    OverdueClockEventHandlingMode defaultOverdueClockEventHandlingMode = UNSPECIFIED;

    int phase = 0;
    clocktime_t offset[2];
    clocktime_t local[2];

    cPar *offsetThreshold;

    double kp;   // proportional gain
    double ki; // integral gain

    ppm kpTerm = ppm(0);
    ppm kiTerm = ppm(0);

    ppm kpTermMax = ppm(100);
    ppm kpTermMin = ppm(-100);
    ppm kiTermMax = ppm(100);
    ppm kiTermMin = ppm(-100);

    ppm drift = ppm(0);

  protected:
    virtual void initialize(int stage) override;

  public:
    /**
     * Sets the clock time immediately to the given value. Greater than 1 oscillator
     * compensation factor means the clock measures time faster.
     *
     * @param newClockTime the new clock time which should be achieved
     * @return the clockTime which was set, which is:
     * - newClockTime, if the servo is in the initial phase
     * - the old clock time, if the servo is in the PI phase (only the oscillator compensation is updated)
     */
    clocktime_t setClockTime(clocktime_t newClockTime);
};

} // namespace inet

#endif
