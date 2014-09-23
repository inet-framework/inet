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

#ifndef __INET_QUATERNION_H
#define __INET_QUATERNION_H

#include "inet/common/geometry/common/Coord.h"

namespace inet {

/*
 * This class represents quaternions as a four dimensional vector space over the real field
 */
class INET_API Quaternion
{
    protected:
        double realPart;
        Coord vectorPart;

    public:
        Quaternion() : realPart(0), vectorPart(Coord(0,0,0)) {}
        Quaternion(double realPart, const Coord& vectorPart) :
            realPart(realPart), vectorPart(vectorPart) {};

        Quaternion operator*(double scalar) const;
        friend Quaternion operator*(double scalar, const Quaternion& lhs);
        Quaternion operator+(const Quaternion& rhs) const;
        Quaternion operator%(const Quaternion& rhs) const;
};

} /* namespace inet */

#endif // ifndef __INET_QUATERNION_H
