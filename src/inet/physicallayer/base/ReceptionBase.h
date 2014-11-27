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

#ifndef __INET_RECEPTIONBASE_H
#define __INET_RECEPTIONBASE_H

#include "inet/physicallayer/contract/ITransmission.h"
#include "inet/physicallayer/contract/IReception.h"
#include "inet/physicallayer/contract/IRadio.h"

namespace inet {

namespace physicallayer {

class INET_API ReceptionBase : public IReception
{
  protected:
    const IRadio *receiver;
    const ITransmission *transmission;
    const simtime_t startTime;
    const simtime_t endTime;
    const Coord startPosition;
    const Coord endPosition;
    const EulerAngles startOrientation;
    const EulerAngles endOrientation;

  public:
    ReceptionBase(const IRadio *receiver, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation);

    virtual void printToStream(std::ostream& stream) const;

    virtual const IRadio *getReceiver() const { return receiver; }
    virtual const ITransmission *getTransmission() const { return transmission; }

    virtual const simtime_t getStartTime() const { return startTime; }
    virtual const simtime_t getEndTime() const { return endTime; }

    virtual const Coord getStartPosition() const { return startPosition; }
    virtual const Coord getEndPosition() const { return endPosition; }

    virtual const EulerAngles getStartOrientation() const { return startOrientation; }
    virtual const EulerAngles getEndOrientation() const { return endOrientation; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RECEPTIONBASE_H

