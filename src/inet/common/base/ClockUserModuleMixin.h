//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_CLOCKUSERMODULEMIXIN_H
#define __INET_CLOCKUSERMODULEMIXIN_H

#include "inet/clock/contract/ClockTime.h"
#include "inet/clock/contract/IClock.h"

namespace inet {

template<typename T>
class INET_API ClockUserModuleMixin : public T, public cListener
{
#ifdef WITH_CLOCK_SUPPORT
  protected:
    IClock *clock = nullptr;

#ifndef NDEBUG
    mutable bool usedClockApi = false;
    const char *className = nullptr; // saved class name for use in destructor
#endif

  protected:
    virtual IClock *findClockModule() const;

  public:
    virtual ~ClockUserModuleMixin();

    virtual void initialize(int stage) override;

    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *msg);
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *msg);
    virtual cMessage *cancelClockEvent(ClockEvent *msg);
    virtual void cancelAndDeleteClockEvent(ClockEvent *msg);

    virtual clocktime_t getClockTime() const;
    virtual clocktime_t getArrivalClockTime(ClockEvent *msg) const;

    using T::uniform;
    using T::exponential;
    using T::normal;
    using T::truncnormal;
    virtual ClockTime uniform(ClockTime a, ClockTime b, int rng=0) const  {return uniform(a.dbl(), b.dbl(), rng);}
    virtual ClockTime exponential(ClockTime mean, int rng=0) const  {return exponential(mean.dbl(), rng);}
    virtual ClockTime normal(ClockTime mean, ClockTime stddev, int rng=0) const  {return normal(mean.dbl(), stddev.dbl(), rng);}
    virtual ClockTime truncnormal(ClockTime mean, ClockTime stddev, int rng=0) const  {return truncnormal(mean.dbl(), stddev.dbl(), rng);}

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;
#else // #ifdef WITH_CLOCK_SUPPORT
  public:
    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *msg) { T::scheduleAt(time, msg); }
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *msg) { T::scheduleAfter(delay, msg); }
    virtual cMessage *cancelClockEvent(ClockEvent *msg) { return T::cancelEvent(msg); }
    virtual void cancelAndDeleteClockEvent(ClockEvent *msg) { T::cancelAndDelete(msg); }
    virtual clocktime_t getClockTime() const { return simTime(); }
    virtual clocktime_t getArrivalClockTime(ClockEvent *msg) const { return msg->getArrivalTime(); }
#endif // #ifdef WITH_CLOCK_SUPPORT
};

} // namespace inet

#endif

