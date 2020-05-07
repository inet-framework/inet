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

#include "inet/common/base/ClockUsingModuleBase.h"

namespace inet {

#ifdef WITH_CLOCK_SUPPORT

void ClockUsingModuleBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == 0)
        clock = findClockModule();
#ifndef NDEBUG
    className = getClassName();
#endif
}

ClockUsingModuleBase::~ClockUsingModuleBase()
{
#ifndef NDEBUG
    bool supportsClock = hasPar("clockModule");
    if (supportsClock && !usedClockApi)
        std::cerr << "** Warning: Class '" << className << "' has a 'clockModule' parameter but does not use the clock API (at least in this simulation)\n";
    if (!supportsClock && usedClockApi)
        std::cerr << "** Warning: Class '" << className << "' uses the clock API but does not have a 'clockModule' parameter\n";
#endif
}

IClock *ClockUsingModuleBase::findClockModule() const
{
    if (hasPar("clockModule")) {
        const char *clockModulePath = par("clockModule");
        if (*clockModulePath) {
            cModule *clockModule = getModuleByPath(clockModulePath);
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

void ClockUsingModuleBase::scheduleClockEvent(simclocktime_t t, cMessage *msg)
{
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock != nullptr)
        clock->scheduleClockEvent(t, msg);
    else
        cSimpleModule::scheduleAt(t.asSimTime(), msg);
}

cMessage *ClockUsingModuleBase::cancelClockEvent(cMessage *msg)
{
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock != nullptr)
        return clock->cancelClockEvent(msg);
    else
        return cSimpleModule::cancelEvent(msg);
}

void ClockUsingModuleBase::cancelAndDeleteClockEvent(cMessage *msg)
{
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock)
        delete clock->cancelClockEvent(msg);
    else
        cSimpleModule::cancelAndDelete(msg);
}

simclocktime_t ClockUsingModuleBase::getClockTime() const
{
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock != nullptr)
        return clock->getClockTime();
    else
        return SimClockTime::from(simTime());
}

simclocktime_t ClockUsingModuleBase::getArrivalClockTime(cMessage *msg) const
{
#ifndef NDEBUG
    usedClockApi = true;
#endif
    if (clock != nullptr)
        return clock->getArrivalClockTime(msg);
    else
        return SimClockTime::from(msg->getArrivalTime());
}

#endif // #ifdef WITH_CLOCK_SUPPORT

} // namespace inet


