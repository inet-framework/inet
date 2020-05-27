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

#ifndef __INET_ICLOCK_H
#define __INET_ICLOCK_H

#include "inet/common/clock/common/ClockTime.h"

namespace inet {

/**
 * This class defines the interface for clocks.
 * See the corresponding NED file for details.
 */
class INET_API IClock
{
  public:
    virtual ~IClock() {}

    /**
     * Return the current time.
     */
    virtual clocktime_t getClockTime() const = 0;

    /**
     * Schedule an event to be delivered to the context module at the given time.
     */
    virtual void scheduleClockEvent(clocktime_t t, cMessage *msg) = 0;

    /**
     * Cancels an event.
     */
    virtual cMessage *cancelClockEvent(cMessage *msg) = 0;

    /**
     * Returns the arrival time of a message scheduled via scheduleClockEvent().
     */
    virtual clocktime_t getArrivalClockTime(cMessage *msg) const = 0;
};

} // namespace inet

#endif // ifndef __INET_ICLOCK_H

