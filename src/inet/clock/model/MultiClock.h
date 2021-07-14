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

#ifndef __INET_MULTICLOCK_H
#define __INET_MULTICLOCK_H

#include "inet/clock/common/ClockEvent.h"
#include "inet/clock/common/ClockTime.h"
#include "inet/clock/contract/IClock.h"

namespace inet {

class INET_API MultiClock : public cModule, public virtual IClock
{
  protected:
    IClock *activeClock = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

  public:
    virtual clocktime_t getClockTime() const override { return activeClock->getClockTime(); }
    virtual clocktime_t computeClockTimeFromSimTime(simtime_t time) const override { return activeClock->computeClockTimeFromSimTime(time); }
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t time) const override { return activeClock->computeSimTimeFromClockTime(time); }
    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *event) override { activeClock->scheduleClockEventAt(time, event); }
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) override { activeClock->scheduleClockEventAfter(delay, event); }
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override { return activeClock->cancelClockEvent(event); }
    virtual void handleClockEvent(ClockEvent *event) override { activeClock->handleClockEvent(event); }
};

} // namespace inet

#endif

