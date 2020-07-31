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

#include "inet/clock/IdealClock.h"

namespace inet {

Define_Module(IdealClock);

simclocktime_t IdealClock::fromSimTime(simtime_t t) const
{
    return SimClockTime::from(t);
}

simtime_t IdealClock::toSimTime(simclocktime_t clock) const
{
    return clock.asSimTime();
}

simclocktime_t IdealClock::getArrivalClockTime(cMessage *msg) const
{
    return SimClockTime::from(msg->getArrivalTime());
}

} // namespace inet

