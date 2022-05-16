//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CLOCKUSERMODULEMIXIN_H
#define __INET_CLOCKUSERMODULEMIXIN_H

#include "inet/clock/contract/IClock.h"
#include "inet/clock/contract/ClockEvent.h"

#ifdef INET_WITH_CLOCK
#include "inet/clock/common/ClockEvent.h"
#endif

#include "inet/common/ModuleRefByPar.h"

namespace inet {

template<typename T>
class INET_API ClockUserModuleMixin : public T
{
#ifdef INET_WITH_CLOCK
  protected:
    ModuleRefByPar<IClock> clock;

#ifndef NDEBUG
    mutable bool usedClockApi = false;
    const char *className = nullptr; // saved class name for use in destructor
#endif

  protected:
    virtual void findClockModule();

  public:
    virtual ~ClockUserModuleMixin();

    virtual void initialize(int stage) override;

    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *msg);
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *msg);
    virtual cMessage *cancelClockEvent(ClockEvent *msg);
    virtual void cancelAndDeleteClockEvent(ClockEvent *msg);
    virtual void rescheduleClockEventAt(clocktime_t time, ClockEvent* msg);
    virtual void rescheduleClockEventAfter(clocktime_t time, ClockEvent* msg);

    virtual clocktime_t computeClockTimeFromSimTime(simtime_t time) const;
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t time) const;
    virtual clocktime_t getClockTime() const;
    virtual clocktime_t getArrivalClockTime(ClockEvent *msg) const;

    using T::uniform;
    using T::exponential;
    using T::normal;
    using T::truncnormal;
    virtual ClockTime uniform(ClockTime a, ClockTime b, int rng = 0) const { return uniform(a.dbl(), b.dbl(), rng); }
    virtual ClockTime exponential(ClockTime mean, int rng = 0) const { return exponential(mean.dbl(), rng); }
    virtual ClockTime normal(ClockTime mean, ClockTime stddev, int rng = 0) const { return normal(mean.dbl(), stddev.dbl(), rng); }
    virtual ClockTime truncnormal(ClockTime mean, ClockTime stddev, int rng = 0) const { return truncnormal(mean.dbl(), stddev.dbl(), rng); }
#else // #ifdef INET_WITH_CLOCK
  public:
    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *msg) { T::scheduleAt(time, msg); }
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *msg) { T::scheduleAfter(delay, msg); }
    virtual cMessage *cancelClockEvent(ClockEvent *msg) { return T::cancelEvent(msg); }
    virtual void cancelAndDeleteClockEvent(ClockEvent *msg) { T::cancelAndDelete(msg); }
    virtual void rescheduleClockEventAt(clocktime_t time, ClockEvent* msg) { T::rescheduleAt(time, msg); }
    virtual void rescheduleClockEventAfter(clocktime_t time, ClockEvent* msg) { T::rescheduleAfter(time, msg); }
    virtual clocktime_t computeClockTimeFromSimTime(simtime_t time) const { return time; }
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t time) const { return time; }
    virtual clocktime_t getClockTime() const { return simTime(); }
    virtual clocktime_t getArrivalClockTime(ClockEvent *msg) const { return msg->getArrivalTime(); }
#endif // #ifdef INET_WITH_CLOCK
};

} // namespace inet

#endif

