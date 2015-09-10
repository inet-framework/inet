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

#include "inet/physicallayer/contract/packetlevel/ITransmission.h"
#include "inet/physicallayer/contract/packetlevel/IReception.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

class INET_API ReceptionBase : public virtual IReception, public virtual IReceptionAnalogModel
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

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const IRadio *getReceiver() const override { return receiver; }
    virtual const ITransmission *getTransmission() const override { return transmission; }

    virtual const simtime_t getStartTime() const override { return startTime; }
    virtual const simtime_t getEndTime() const override { return endTime; }
    virtual const simtime_t getDuration() const override { return endTime - startTime; }

    virtual const Coord getStartPosition() const override { return startPosition; }
    virtual const Coord getEndPosition() const override { return endPosition; }

    virtual const EulerAngles getStartOrientation() const override { return startOrientation; }
    virtual const EulerAngles getEndOrientation() const override { return endOrientation; }

    virtual const IReceptionAnalogModel *getAnalogModel() const override { return check_and_cast<const IReceptionAnalogModel *>(this); }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RECEPTIONBASE_H

