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

#include "inet/common/geometry/common/Quaternion.h"

namespace inet {

Quaternion Quaternion::operator*(double scalar) const
{
    return Quaternion(this->realPart * scalar, this->vectorPart * scalar);
}

Quaternion inet::Quaternion::operator+(const Quaternion& rhs) const
{
    return Quaternion(this->realPart + rhs.realPart, this->vectorPart + rhs.vectorPart);
}

Quaternion Quaternion::operator%(const Quaternion& rhs) const
{
    return Quaternion(realPart * rhs.realPart - vectorPart * rhs.vectorPart,
                     rhs.vectorPart * realPart + vectorPart * rhs.realPart +
                     vectorPart % rhs.vectorPart);
}

Quaternion operator*(double scalar, const Quaternion& lhs)
{
    return Quaternion(lhs.realPart * scalar, lhs.vectorPart * scalar);
}

} /* namespace inet */
