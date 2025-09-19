//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OSCILLATORBASEDCLOCK_H
#define __INET_OSCILLATORBASEDCLOCK_H

#include "inet/clock/base/ClockBase.h"
#include "inet/clock/contract/IOscillator.h"
#include "inet/common/Units.h"

namespace inet {

using namespace units::values;

/**
 * @brief A clock model that is counting the ticks of an oscillator.
 *
 * The following properties always hold for all oscillator based clocks:
 *
 * 1. The clock origin simulation time is always less than or equal to the computation origin of the oscillator.
 *
 * @note For detailed configuration, see the corresponding NED file.
 */
class INET_API OscillatorBasedClock : public ClockBase, public cListener
{
  protected:
    IOscillator *oscillator = nullptr;
    int64_t (*roundingFunction)(int64_t, int64_t) = nullptr;

    /**
     * The simulation time from which the clock computes the mapping between future simulation times and clock times.
     */
    simtime_t originSimulationTime;
    /**
     * The clock time from which the clock computes the mapping between future simulation times and clock times.
     */
    clocktime_t originClockTime;

    std::vector<ClockEvent *> events;

  protected:
    virtual void initialize(int stage) override;

    virtual void setOrigin(simtime_t simulationTime, clocktime_t clockTime);

    void checkAllClockEvents() {
        for (auto event : events)
            checkClockEvent(event);
    }

    clocktime_t doComputeClockTimeFromSimTime(simtime_t t) const;
    simtime_t doComputeSimTimeFromClockTime(clocktime_t t, bool lowerBound) const;

  public:
    virtual ~OscillatorBasedClock();

    virtual clocktime_t getClockTime() const override;

    virtual const IOscillator *getOscillator() const { return oscillator; }
    virtual ppm getOscillatorCompensation() const { return ppm(0); }

    virtual clocktime_t computeClockTimeFromSimTime(simtime_t t) const override;
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t t, bool lowerBound = true) const override;

    virtual void scheduleClockEventAt(clocktime_t t, ClockEvent *event) override;
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override;
    virtual void handleClockEvent(ClockEvent *event) override;

    virtual std::string resolveDirective(char directive) const override;

    virtual void receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif

