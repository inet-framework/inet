//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OSCILLATORBASEDCLOCK_H
#define __INET_OSCILLATORBASEDCLOCK_H

#include "inet/clock/base/ClockBase.h"
#include "inet/clock/contract/IOscillator.h"
#include "inet/common/SimTimeScale.h"
#include "inet/common/Units.h"

namespace inet {

using namespace units::values;

static bool compareClockEvents(const ClockEvent *e1, const ClockEvent *e2) {
    return e2->getArrivalClockTime() < e1->getArrivalClockTime() ? true :
           e2->getArrivalClockTime() > e1->getArrivalClockTime() ? false :
           e2->getSchedulingPriority() == e1->getSchedulingPriority() ? e2->getInsertionOrder() < e1->getInsertionOrder() :
           e2->getSchedulingPriority() < e1->getSchedulingPriority() ? true :
           e2->getSchedulingPriority() > e1->getSchedulingPriority() ? false :
           e2->getInsertionOrder() < e1->getInsertionOrder();
}

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
    bool useFutureEventSet = false;
    uint64_t insertionCount = 0;
    IOscillator *oscillator = nullptr;
    simtime_raw_t (*roundingFunction)(simtime_raw_t, simtime_raw_t) = nullptr;

    /**
     * The lower bound of the simulation time where the clock time equals with origin clock time (exactly at an oscillator tick).
     */
    simtime_t originSimulationTimeLowerBound;
    /**
     * The simulation time from which the clock computes the mapping between future simulation times and clock times.
     */
    simtime_t originSimulationTime;
    /**
     * The clock time from which the clock computes the mapping between future simulation times and clock times.
     */
    clocktime_t originClockTime;
    /**
     * The clock time accumulated from the oscillator compensation.
     */
    clocktime_t clockTimeCompensation;
    /**
     * When the clock origin changes we *lose* the fractional part of the oscillator
     * compensation accumulated up to that point. To preserve continuity we keep a
     * phase as an integer tick offset n0 such that:
     *
     *     φ = frac( x * n0 ),  where x = oscillatorCompensation
     *
     * Using n0, we can express the mappings without explicit φ and using only mulFloor/divCeil:
     *
     * Simulation time -> Clock time (Δn oscillator ticks -> compensated ticks):
     *     C(Δn) = floor( x * (Δn + n0) ) − floor( x * n0 )
     *
     * Clock time -> Simulation time (M compensated ticks -> oscillator ticks):
     *     Δn = ceil( ( M + floor( x * n0 ) ) / x ) − n0
     *
     * Implementation notes:
     * - Store n0 in compensationPhaseBaseTicks.
     * - On clock-origin move by Δticks (tick-aligned lower bounds), do:
     *       compensationPhaseBaseTicks += Δticks;
     * - (Optional) Canonicalize n0 to keep it small while preserving floor(x*n0):
     *       n0 = divCeil( mulFloor(n0) );
     */
    int64_t compensationPhaseBaseTicks = 0;
    int64_t numTicksAtOriginLowerBound = 0;

    uint64_t lastNumTicks = 0;
    clocktime_t clockTimeBeforeOscillatorStateChange = -1;

    std::vector<ClockEvent *> events;

    cMessage *executeEventsTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void scheduleTargetModuleClockEventAt(simtime_t time, ClockEvent *event) override;
    virtual void scheduleTargetModuleClockEventAfter(simtime_t time, ClockEvent *event) override;
    virtual ClockEvent *cancelTargetModuleClockEvent(ClockEvent *event) override;

    virtual void setOrigin(simtime_t simulationTime, clocktime_t clockTime);

    void checkAllClockEvents() {
        for (auto event : events)
            checkClockEvent(event);
    }

    int64_t computeCompensatedTicksFromTicks(int64_t numTicks) const;
    int64_t computeTicksFromCompensatedTicks(int64_t compensatedTicks) const;

    clocktime_t doComputeClockTimeFromSimTime(simtime_t t) const;
    simtime_t doComputeSimTimeFromClockTime(clocktime_t t, bool lowerBound) const;

  public:
    virtual ~OscillatorBasedClock();

    virtual clocktime_t getClockTime() const override;

    virtual const IOscillator *getOscillator() const { return oscillator; }
    virtual SimTimeScale getOscillatorCompensation() const { return SimTimeScale(); }

    virtual clocktime_t computeClockTimeFromSimTime(simtime_t t) const override;
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t t, bool lowerBound = true) const override;

    virtual void scheduleClockEventAt(clocktime_t t, ClockEvent *event) override;
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override;
    virtual void handleClockEvent(ClockEvent *event) override;
    virtual bool isScheduledClockEvent(ClockEvent *event) const override;

    virtual std::string resolveDirective(char directive) const override;

    virtual void receiveSignal(cComponent *source, int signal, uintval_t value, cObject *details) override;
    virtual void receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif

