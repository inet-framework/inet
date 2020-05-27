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

#ifndef __INET_IDEALCLOCK_H
#define __INET_IDEALCLOCK_H

#include "inet/common/clock/base/PredictableClockBase.h"

namespace inet {

/**
 * Models a clock where the clock time is identical to the simulation time.
 */
class INET_API IdealClock : public PredictableClockBase
{
  public:
    virtual clocktime_t fromSimTime(simtime_t t) const override;
    virtual simtime_t toSimTime(clocktime_t t) const override;
    virtual clocktime_t getArrivalClockTime(cMessage *msg) const override;
};

} // namespace inet

#endif // ifndef __INET_IDEALCLOCK_H

