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

#ifndef __INET_ROTATION_H
#define __INET_ROTATION_H

#include "inet/common/geometry/common/EulerAngles.h"
#include "inet/common/geometry/common/Coord.h"

namespace inet {

/*
 * This class efficiently computes the rotation of an arbitrary vector by an Euler Angle
 */
class INET_API Rotation
{
    protected:
        double rotationMatrix[3][3];
        void computeRotationMatrices(const double& q0, const double& q1, const double& q2, const double& q3);
        Coord matrixMultiplication(const double matrix[3][3], const Coord& vector) const;
        Coord matrixTransposeMultiplication(const double matrix[3][3], const Coord& vector) const;

    public:
        Rotation();
        Rotation(const EulerAngles& eulerAngle);
        Coord rotateVectorClockwise(const Coord& vector) const;
        Coord rotateVectorCounterClockwise(const Coord& vector) const;
};

} /* namespace inet */

#endif // ifndef __INET_ROTATION_H
