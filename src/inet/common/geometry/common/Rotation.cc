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
#include "inet/common/geometry/common/Rotation.h"

namespace inet {

Rotation::Rotation()
{
    // identity matrix
    matrix[0][0] = 1;
    matrix[0][1] = matrix[0][2] = 0;
    matrix[1][1] = 1;
    matrix[1][0] = matrix[1][2] = 0;
    matrix[2][2] = 1;
    matrix[2][1] = matrix[2][0] = 0;
}

Rotation::Rotation(const EulerAngles& eulerAngle)
{
    Quaternion q(eulerAngle);
    computeRotationMatrix(q.s, q.v.x, q.v.y, q.v.z);
}

void Rotation::computeRotationMatrix(const double& q0, const double& q1, const double& q2, const double& q3)
{
    // Ref: http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    matrix[0][0] = 1 - 2*(q2*q2 + q3*q3);
    matrix[0][1] = 2*(q1*q2 - q0*q3);
    matrix[0][2] = 2*(q0*q2 + q1*q3);
    matrix[1][0] = 2*(q1*q2 + q0*q3);
    matrix[1][1] = 1 - 2*(q1*q1 + q3*q3);
    matrix[1][2] = 2*(q2*q3 - q0*q1);
    matrix[2][0] = 2*(q1*q3 - q0*q2);
    matrix[2][1] = 2*(q0*q1 + q2*q3);
    matrix[2][2] = 1 - 2*(q1*q1 + q2*q2);
}

Coord Rotation::rotateVector(const Coord& vector) const
{
    return Coord(vector.x * matrix[0][0] + vector.y * matrix[0][1] + vector.z * matrix[0][2],
                 vector.x * matrix[1][0] + vector.y * matrix[1][1] + vector.z * matrix[1][2],
                 vector.x * matrix[2][0] + vector.y * matrix[2][1] + vector.z * matrix[2][2]);
}

Coord Rotation::rotateVectorInverse(const Coord& vector) const
{
    return Coord(vector.x * matrix[0][0] + vector.y * matrix[1][0] + vector.z * matrix[2][0],
                 vector.x * matrix[0][1] + vector.y * matrix[1][1] + vector.z * matrix[2][1],
                 vector.x * matrix[0][2] + vector.y * matrix[1][2] + vector.z * matrix[2][2]);
}

} /* namespace inet */
