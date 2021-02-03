//
// Copyright (C) 2021 OpenSim Ltd.
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

#ifndef __INET_COMMON_CLOCKEVENT_H
#define __INET_COMMON_CLOCKEVENT_H

#include "inet/clock/common/ClockEvent_m.h"

namespace inet {

class INET_API ClockEvent : public ClockEvent_Base
{
  public:
    ClockEvent(const char *name = nullptr, short kind = 0) : ClockEvent_Base(name, kind) {}
    ClockEvent(const ClockEvent& other) : ClockEvent_Base(other) {}

    ClockEvent& operator=(const ClockEvent& other) {
        if (this == &other) return *this;
        ClockEvent_Base::operator=(other);
        return *this;
    }
    virtual ClockEvent *dup() const override { return new ClockEvent(*this); }

    virtual clocktime_t getArrivalClockTime() const override;

    virtual void execute() override;
};

} // namespace inet

#endif

