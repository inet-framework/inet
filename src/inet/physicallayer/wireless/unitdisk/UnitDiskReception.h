//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_UNITDISKRECEPTION_H
#define __INET_UNITDISKRECEPTION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ReceptionBase.h"

namespace inet {

namespace physicallayer {

/**
 * This model characterizes receptions with a simplified reception power that
 * falls into one of the categories: receivable, interfering, detectable, and
 * undetectable.
 */
class INET_API UnitDiskReception : public ReceptionBase
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
    UnitDiskReception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, const Power power);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual Power getPower() const { return power; }
};

} // namespace physicallayer

} // namespace inet

#endif

