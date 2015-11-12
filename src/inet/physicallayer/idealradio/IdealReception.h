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

#ifndef __INET_IDEALRECEPTION_H
#define __INET_IDEALRECEPTION_H

#include "inet/physicallayer/base/packetlevel/ReceptionBase.h"

namespace inet {

namespace physicallayer {

/**
 * This model characterizes receptions with a simplified reception power that
 * falls into one of the categories: receivable, interfering, detectable, and
 * undetectable.
 */
class INET_API IdealReception : public ReceptionBase
{
  public:
    enum Power {
        POWER_UNDETECTABLE,
        POWER_DETECTABLE,
        POWER_INTERFERING,
        POWER_RECEIVABLE
    };

  protected:
    const Power power;

  public:
    IdealReception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, const Power power);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual Power getPower() const { return power; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IDEALRECEPTION_H

