//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CLOCKUSERMODULEMIXINIMPL_H
#define __INET_CLOCKUSERMODULEMIXINIMPL_H

#include "inet/common/clock/ClockUserModuleMixin.h"

namespace inet {

#ifdef INET_WITH_CLOCK

template<typename T>
void ClockUserModuleMixin<T>::findClockModule() {
    if (T::hasPar("clockModule")) {
        clock.reference(this, "clockModule", false);
    }
}

template<typename T>
ClockUserModuleMixin<T>::~ClockUserModuleMixin() {
#ifndef NDEBUG
    if (clock != nullptr && !usedClockApi)
        EV_WARN << "Class '" << className << "' has a clock module set but does not use the clock API (at least in this simulation)\n";
    if (!T::hasPar("clockModule") && usedClockApi)
        EV_WARN << "Class '" << className << "' uses the clock API but does not have a clock module parameter\n";
#endif
}

template<typename T>
void ClockUserModuleMixin<T>::initialize(int stage) {
    T::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        findClockModule();
#ifndef NDEBUG
        className = T::getClassName();
#endif
    }
}

template<typename T>
void ClockUserModuleMixin<T>::scheduleClockEventAt(clocktime_t t, ClockEvent *msg) {
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock != nullptr)
        clock->scheduleClockEventAt(t, msg);
    else
        T::scheduleAt(t.asSimTime(), msg);
}

template<typename T>
void ClockUserModuleMixin<T>::scheduleClockEventAfter(clocktime_t t, ClockEvent *msg) {
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock != nullptr)
        clock->scheduleClockEventAfter(t, msg);
    else
        T::scheduleAfter(t.asSimTime(), msg);
}

template<typename T>
cMessage *ClockUserModuleMixin<T>::cancelClockEvent(ClockEvent *msg) {
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock != nullptr)
        return clock->cancelClockEvent(msg);
    else
        return T::cancelEvent(msg);
}

template<typename T>
void ClockUserModuleMixin<T>::cancelAndDeleteClockEvent(ClockEvent *msg) {
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock) {
        if (msg)
            delete clock->cancelClockEvent(msg);
    }
    else
        T::cancelAndDelete(msg);
}

template<typename T>
void ClockUserModuleMixin<T>::rescheduleClockEventAt(clocktime_t t, ClockEvent* msg)
{
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock != nullptr)
        clock->scheduleClockEventAt(t, clock->cancelClockEvent(msg));
    else
        T::rescheduleAt(t.asSimTime(), msg);
}

template<typename T>
void ClockUserModuleMixin<T>::rescheduleClockEventAfter(clocktime_t t, ClockEvent* msg)
{
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock != nullptr)
        clock->scheduleClockEventAfter(t, clock->cancelClockEvent(msg));
    else
        T::rescheduleAfter(t.asSimTime(), msg);
}

template<typename T>
clocktime_t ClockUserModuleMixin<T>::computeClockTimeFromSimTime(simtime_t t) const
{
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock != nullptr)
        return clock->computeClockTimeFromSimTime(t);
    else
        return ClockTime::from(t);
}

template<typename T>
simtime_t ClockUserModuleMixin<T>::computeSimTimeFromClockTime(clocktime_t t) const
{
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock != nullptr)
        return clock->computeSimTimeFromClockTime(t);
    else
        return t.asSimTime();
}

template<typename T>
clocktime_t ClockUserModuleMixin<T>::getClockTime() const {
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock != nullptr)
        return clock->getClockTime();
    else
        return ClockTime::from(simTime());
}

template<typename T>
clocktime_t ClockUserModuleMixin<T>::getArrivalClockTime(ClockEvent *msg) const {
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock != nullptr)
        return msg->getArrivalClockTime();
    else
        return ClockTime::from(msg->getArrivalTime());
}

#endif // #ifdef INET_WITH_CLOCK

} // namespace inet

#endif

