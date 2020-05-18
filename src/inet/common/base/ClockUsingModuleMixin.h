//
// Copyright (C) OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_CLOCKUSINGMODULEMIXIN_H
#define __INET_CLOCKUSINGMODULEMIXIN_H

#include "inet/common/clock/common/SimClockTime.h"

#ifdef WITH_CLOCK_SUPPORT
#include "inet/common/clock/contract/IClock.h"
#endif

namespace inet {

template<typename T>
class INET_API ClockUsingModuleMixin : public T
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
    virtual ~ClockUsingModuleMixin();

    virtual void initialize(int stage);

    virtual void scheduleClockEvent(simclocktime_t t, cMessage *msg);
    virtual cMessage *cancelClockEvent(cMessage *msg);
    virtual void cancelAndDeleteClockEvent(cMessage *msg);

    virtual simclocktime_t getClockTime() const;
    virtual simclocktime_t getArrivalClockTime(cMessage *msg) const;

    using T::uniform;
    using T::exponential;
    using T::normal;
    using T::truncnormal;
    virtual SimClockTime uniform(SimClockTime a, SimClockTime b, int rng=0) const  {return uniform(a.dbl(), b.dbl(), rng);}
    virtual SimClockTime exponential(SimClockTime mean, int rng=0) const  {return exponential(mean.dbl(), rng);}
    virtual SimClockTime normal(SimClockTime mean, SimClockTime stddev, int rng=0) const  {return normal(mean.dbl(), stddev.dbl(), rng);}
    virtual SimClockTime truncnormal(SimClockTime mean, SimClockTime stddev, int rng=0) const  {return truncnormal(mean.dbl(), stddev.dbl(), rng);}
#else // #ifdef WITH_CLOCK_SUPPORT
  public:
    virtual void scheduleClockEvent(simclocktime_t t, cMessage *msg) { T::scheduleAt(t, msg); }
    virtual cMessage *cancelClockEvent(cMessage *msg) { return T::cancelEvent(msg); }
    virtual void cancelAndDeleteClockEvent(cMessage *msg) { T::cancelAndDelete(msg); }
    virtual simclocktime_t getClockTime() const { return simTime(); }
    virtual simclocktime_t getArrivalClockTime(cMessage *msg) const { return msg->getArrivalTime(); }
#endif // #ifdef WITH_CLOCK_SUPPORT
};

} // namespace inet

#endif // ifndef __INET_CLOCKUSINGMODULEMIXIN_H

