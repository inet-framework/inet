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

#include "inet/common/clock/LinearClock.h"

namespace inet {

Define_Module(LinearClock);

void LinearClock::initialize()
{
    origin = par("origin");
    driftRate = par("driftRate").doubleValue() / 1e6;
}

clocktime_t LinearClock::fromSimTime(simtime_t t) const
{
    return ClockTime::from((t-origin) / (1 + driftRate));
}

simtime_t LinearClock::toSimTime(clocktime_t clock) const
{
    return clock.asSimTime() * (1 + driftRate) + origin;
}

clocktime_t LinearClock::getArrivalClockTime(cMessage *msg) const
{
    return fromSimTime(msg->getArrivalTime()); // note: imprecision due to conversion to simtime and forth
}

} // namespace inet

