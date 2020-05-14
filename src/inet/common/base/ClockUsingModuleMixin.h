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
// along with this program.  If not, see http://www.gnu.org/licenses/.
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
    virtual IClock *findClockModule() const {
        if (T::hasPar("clockModule")) {
            const char *clockModulePath = T::par("clockModule");
            if (*clockModulePath) {
                cModule *clockModule = T::getModuleByPath(clockModulePath);
                if (clockModule == nullptr)
                    throw cRuntimeError("Clock module '%s' not found", clockModulePath);
                return check_and_cast<IClock *>(clockModule);
            }
            else
                return nullptr;
        }
        else
            return nullptr;
    }

  public:
    virtual ~ClockUsingModuleMixin() {
#ifndef NDEBUG
    bool supportsClock = T::hasPar("clockModule");
    if (supportsClock && !usedClockApi)
        std::cerr << "** Warning: Class '" << className << "' has a 'clockModule' parameter but does not use the clock API (at least in this simulation)\n";
    if (!supportsClock && usedClockApi)
        std::cerr << "** Warning: Class '" << className << "' uses the clock API but does not have a 'clockModule' parameter\n";
#endif
    }

    virtual void initialize(int stage) {
        T::initialize(stage);
        if (stage == 0)
            clock = findClockModule();
#ifndef NDEBUG
        className = T::getClassName();
#endif
    }

    virtual void scheduleClockEvent(simclocktime_t t, cMessage *msg) {
#ifndef NDEBUG
        usedClockApi = true;
#endif
        if (clock != nullptr)
            clock->scheduleClockEvent(t, msg);
        else
            T::scheduleAt(t.asSimTime(), msg);
    }

    virtual cMessage *cancelClockEvent(cMessage *msg) {
#ifndef NDEBUG
        usedClockApi = true;
#endif
        if (clock != nullptr)
            return clock->cancelClockEvent(msg);
        else
            return T::cancelEvent(msg);
    }

    virtual void cancelAndDeleteClockEvent(cMessage *msg) {
#ifndef NDEBUG
        usedClockApi = true;
#endif
        if (clock)
            delete clock->cancelClockEvent(msg);
        else
            T::cancelAndDelete(msg);
    }

    virtual simclocktime_t getClockTime() const {
#ifndef NDEBUG
        usedClockApi = true;
#endif
        if (clock != nullptr)
            return clock->getClockTime();
        else
            return SimClockTime::from(simTime());
    }

    virtual simclocktime_t getArrivalClockTime(cMessage *msg) const {
#ifndef NDEBUG
        usedClockApi = true;
#endif
        if (clock != nullptr)
            return clock->getArrivalClockTime(msg);
        else
            return SimClockTime::from(msg->getArrivalTime());
    }

    using T::uniform;
    using T::exponential;
    using T::normal;
    using T::truncnormal;
    virtual SimClockTime uniform(SimClockTime a, SimClockTime b, int rng=0) const  {return uniform(a.dbl(), b.dbl(), rng);}
    virtual SimClockTime exponential(SimClockTime mean, int rng=0) const  {return exponential(mean.dbl(), rng);}
    virtual SimClockTime normal(SimClockTime mean, SimClockTime stddev, int rng=0) const  {return normal(mean.dbl(), stddev.dbl(), rng);}
    virtual SimClockTime truncnormal(SimClockTime mean, SimClockTime stddev, int rng=0) const  {return truncnormal(mean.dbl(), stddev.dbl(), rng);}
#else // #ifdef WITH_CLOCK_SUPPORT
    virtual void scheduleClockEvent(simclocktime_t t, cMessage *msg) { scheduleAt(t, msg); }
    virtual cMessage *cancelClockEvent(cMessage *msg) { return cancelEvent(msg); }
    virtual void cancelAndDeleteClockEvent(cMessage *msg) { cancelAndDelete(msg); }
    virtual simclocktime_t getClockTime() const { return simTime(); }
    virtual simclocktime_t getArrivalClockTime(cMessage *msg) const { return msg->getArrivalTime(); }
#endif // #ifdef WITH_CLOCK_SUPPORT
};

} // namespace inet

#endif // ifndef __INET_CLOCKUSINGMODULEMIXIN_H

