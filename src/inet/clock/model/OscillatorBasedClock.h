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

#ifndef __INET_OSCILLATORBASEDCLOCK_H
#define __INET_OSCILLATORBASEDCLOCK_H

#include "inet/clock/base/ClockBase.h"
#include "inet/clock/contract/IOscillator.h"

namespace inet {

class INET_API OscillatorBasedClock : public ClockBase, public cListener
{
  protected:
    IOscillator *oscillator = nullptr;
    int64_t (*roundingFunction)(int64_t, int64_t) = nullptr;

    int64_t originClockTick = -1; // the value of the clock internal register representing the time as known by the clock measured in oscillator nominal tick intervals
    std::vector<ClockEvent *> events;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual ~OscillatorBasedClock();

    virtual clocktime_t computeClockTimeFromSimTime(simtime_t t) const override;
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t t) const override;

    virtual void scheduleClockEventAt(clocktime_t t, ClockEvent *event) override;
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override;
    virtual void handleClockEventOccurred(ClockEvent *event) override;

    virtual const char *resolveDirective(char directive) const override;

    virtual void receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif

