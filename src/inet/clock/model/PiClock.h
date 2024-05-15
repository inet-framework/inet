//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PICONTROLCLOCK_H
#define __INET_PICONTROLCLOCK_H

#include "inet/clock/model/OscillatorBasedClock.h"
#include "inet/common/scenario/IScriptable.h"

namespace inet {

class INET_API PiClock : public OscillatorBasedClock, public IScriptable
{
  protected:
    static simsignal_t driftSignal;
    static simsignal_t kpSignal;
    OverdueClockEventHandlingMode defaultOverdueClockEventHandlingMode = UNSPECIFIED;
    ppm oscillatorCompensation = ppm(0); // 0 means no compensation, higher value means faster clock, e.g. 100 ppm value
                                         // means the clock compensates 100 microseconds for every second in clock time
    // 100 ppm value means the oscillator tick length is compensated to be smaller by a factor of (1 / (1 + 100 / 1E+6))
    // than the actual tick length measured in clock time

    int phase = 0;
    clocktime_t offset[2];
    clocktime_t local[2];

    cPar *offsetThreshold;

    double kp;   // proportional gain
    double ki; // integral gain
    double kd = 0;

    ppm kpTerm = ppm(0);
    ppm kiTerm = ppm(0);
    ppm kdTerm = ppm(0);

    ppm kpTermMax = ppm(100);
    ppm kpTermMin = ppm(-100);
    ppm kiTermMax = ppm(100);
    ppm kiTermMin = ppm(-100);
    ppm kdTermMax = ppm(100);
    ppm kdTermMin = ppm(-100);

    ppm drift = ppm(0);

  protected:
    virtual void initialize(int stage) override;

    virtual OverdueClockEventHandlingMode getOverdueClockEventHandlingMode(ClockEvent *event) const;

    virtual simtime_t handleOverdueClockEvent(ClockEvent *event, simtime_t t);

    // IScriptable implementation
    virtual void processCommand(const cXMLElement &node) override;

  public:
    virtual ppm getOscillatorCompensation() const override { return oscillatorCompensation; }

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
