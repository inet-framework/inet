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

#include "Rotation.h"

namespace inet {

Rotation::Rotation(const EulerAngles& eulerAngle)
{
    // Note that, we don't need to use our Quaternion class since we don't use its operators
    // roll (z), pitch (y), yaw (z)
    double q0 = cos(eulerAngle.gamma/2) * cos(eulerAngle.beta/2) * cos(eulerAngle.alpha/2) +
                sin(eulerAngle.gamma/2) * sin(eulerAngle.beta/2) * sin(eulerAngle.alpha/2);
    double q1 = sin(eulerAngle.gamma/2) * cos(eulerAngle.beta/2) * cos(eulerAngle.alpha/2) -
                cos(eulerAngle.gamma/2) * sin(eulerAngle.beta/2) * sin(eulerAngle.alpha/2);
    double q2 = cos(eulerAngle.gamma/2) * sin(eulerAngle.beta/2) * cos(eulerAngle.alpha/2) +
                sin(eulerAngle.gamma/2) * cos(eulerAngle.beta/2) * sin(eulerAngle.alpha/2);
    double q3 = cos(eulerAngle.gamma/2) * cos(eulerAngle.beta/2) * sin(eulerAngle.alpha/2) -
                sin(eulerAngle.gamma/2) * sin(eulerAngle.beta/2) * cos(eulerAngle.alpha/2);
    computeRotationMatrix(q0, q1, q2, q3);
}

void Rotation::computeRotationMatrix(const double& q0, const double& q1, const double& q2, const double& q3)
{
    // Ref: http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    rotationMatrix[0][0] = 1 - 2*(q2*q2 + q3*q3);
    rotationMatrix[0][1] = 2*(q1*q2 - q0*q3);
    rotationMatrix[0][2] = 2*(q0*q2 + q1*q3);
    rotationMatrix[1][0] = 2*(q1*q2 + q0*q3);
    rotationMatrix[1][1] = 1 - 2*(q1*q1 + q3*q3);
    rotationMatrix[1][2] = 2*(q2*q3 - q0*q1);
    rotationMatrix[2][0] = 2*(q1*q3 - q0*q2);
    rotationMatrix[2][1] = 2*(q0*q1 + q2*q3);
    rotationMatrix[2][2] = 1 - 2*(q1*q1 - q2*q2);
}

Coord Rotation::rotateVector(const Coord& vector) const
{
    // Compute rotationMatrix * vector
    return Coord(
            vector.x * rotationMatrix[0][0] + vector.y * rotationMatrix[0][1] + vector.z * rotationMatrix[0][2],
            vector.x * rotationMatrix[1][0] + vector.y * rotationMatrix[1][1] + vector.z * rotationMatrix[1][2],
            vector.x * rotationMatrix[2][0] + vector.y * rotationMatrix[2][1] + vector.z * rotationMatrix[2][2]);
}

} /* namespace inet */
