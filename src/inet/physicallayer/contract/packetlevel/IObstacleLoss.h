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

#ifndef __INET_IOBSTACLELOSS_H
#define __INET_IOBSTACLELOSS_H

#include "inet/common/geometry/common/Coord.h"
#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"

namespace inet {

namespace physicallayer {

/**
 * This interface models obstacle loss that is the reduction in power density of
 * a radio signal as it propagates through physical objects present in space.
 */
class INET_API IObstacleLoss : public IPrintableObject
{
  public:
    /**
     * Returns the obstacle loss factor caused by physical objects present in
     * the environment as a function of frequency, transmission position, and
     * reception position. The value is in the range [0, 1] where 1 means no
     * loss at all and 0 means all power is lost.
     */
    virtual double computeObstacleLoss(Hz frequency, const Coord& transmissionPosition, const Coord& receptionPosition) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IOBSTACLELOSS_H

