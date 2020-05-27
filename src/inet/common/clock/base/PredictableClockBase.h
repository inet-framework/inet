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

#ifndef __INET_PREDICTABLECLOCKBASE_H
#define __INET_PREDICTABLECLOCKBASE_H

#include "inet/common/clock/base/ClockBase.h"
#include "inet/common/clock/contract/IClock.h"

namespace inet {

/**
 * Abstract base class for clocks where the mapping between the simulation time
 * and the clock time is known in advance. The mapping should be provided by
 * overriding the fromSimTime() and toSimTime() methods.
 */
class INET_API PredictableClockBase : public ClockBase, public IClock
{
  public:
    virtual clocktime_t fromSimTime(simtime_t t) const = 0;
    virtual simtime_t toSimTime(clocktime_t t) const = 0;

    virtual clocktime_t getClockTime() const override;
    virtual void scheduleClockEvent(clocktime_t t, cMessage *msg) override;
    virtual cMessage *cancelClockEvent(cMessage *msg) override;
};

} // namespace inet

#endif // ifndef __INET_PREDICTABLECLOCKBASE_H

