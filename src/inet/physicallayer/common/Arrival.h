//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_ARRIVAL_H
#define __INET_ARRIVAL_H

#include "inet/physicallayer/contract/IArrival.h"

namespace inet {

namespace physicallayer {

class INET_API Arrival : public virtual IArrival
{
  protected:
    const simtime_t startPropagationTime;
    const simtime_t endPropagationTime;
    const simtime_t startTime;
    const simtime_t endTime;
    const Coord startPosition;
    const Coord endPosition;
    const EulerAngles startOrientation;
    const EulerAngles endOrientation;

  public:
    Arrival(const simtime_t startPropagationTime, const simtime_t endPropagationTime, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation);

    virtual void printToStream(std::ostream& stream) const;

    virtual const simtime_t getStartPropagationTime() const { return startPropagationTime; }
    virtual const simtime_t getEndPropagationTime() const { return endPropagationTime; }

    virtual const simtime_t getStartTime() const { return startTime; }
    virtual const simtime_t getEndTime() const { return endTime; }

    virtual const Coord getStartPosition() const { return startPosition; }
    virtual const Coord getEndPosition() const { return endPosition; }

    virtual const EulerAngles getStartOrientation() const { return startOrientation; }
    virtual const EulerAngles getEndOrientation() const { return endOrientation; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_ARRIVAL_H

