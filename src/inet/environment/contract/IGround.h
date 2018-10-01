//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_IGROUND_H
#define __INET_IGROUND_H

#include "inet/common/geometry/common/Coord.h"

namespace inet {

namespace physicalenvironment {

class INET_API IGround
{
  public:
    /**
     * Returns a point on the ground "underneath" (or above) the given position.
     * The projection might alter only the Z coordinate in simple cases, or it might
     * alter all coordinates in case of a large scene on a globe model, or a scene
     * placed at an angle above the ground (using a IGeographicCoordinateSystem).
     */
    virtual Coord computeGroundProjection(const Coord &position) const = 0;

    /**
     * Returns a unit length vector that is locally perpendicular to the ground at "position",
     * pointing up. If such vector cannot be determined, all components of the result are NaN.
     * The point given by "position" does not have to be on the ground. If necessary, it is
     * first projected to the ground internally by the implementation.
     */
    virtual Coord computeGroundNormal(const Coord &position) const = 0;
};

} // namespace physicalenvironment

} // namespace inet

#endif // ifndef __INET_IGROUND_H
