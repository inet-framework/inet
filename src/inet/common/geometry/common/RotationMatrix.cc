//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/common/RotationMatrix.h"

#include "inet/common/INETMath.h"
#include "inet/common/geometry/common/Quaternion.h"

namespace inet {

RotationMatrix::RotationMatrix()
{
    // identity matrix
    matrix[0][0] = 1;
    matrix[0][1] = matrix[0][2] = 0;
    matrix[1][1] = 1;
    matrix[1][0] = matrix[1][2] = 0;
    matrix[2][2] = 1;
    matrix[2][1] = matrix[2][0] = 0;
}

RotationMatrix::RotationMatrix(const double matrix[3][3])
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            this->matrix[i][j] = matrix[i][j];
    ASSERT(inet::math::close(computeDeterminant(), 1));
}

RotationMatrix::RotationMatrix(const EulerAngles& eulerAngles)
{
    Quaternion q(eulerAngles);
    computeRotationMatrix(q.s, q.v.x, q.v.y, q.v.z);
}

void RotationMatrix::computeRotationMatrix(const double& q0, const double& q1, const double& q2, const double& q3)
{
    // Ref: http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    matrix[0][0] = 1 - 2 * (q2 * q2 + q3 * q3);
    matrix[0][1] = 2 * (q1 * q2 - q0 * q3);
    matrix[0][2] = 2 * (q0 * q2 + q1 * q3);
    matrix[1][0] = 2 * (q1 * q2 + q0 * q3);
    matrix[1][1] = 1 - 2 * (q1 * q1 + q3 * q3);
    matrix[1][2] = 2 * (q2 * q3 - q0 * q1);
    matrix[2][0] = 2 * (q1 * q3 - q0 * q2);
    matrix[2][1] = 2 * (q0 * q1 + q2 * q3);
    matrix[2][2] = 1 - 2 * (q1 * q1 + q2 * q2);
}

double RotationMatrix::computeDeterminant() const
{
    return matrix[0][0] * ((matrix[1][1] * matrix[2][2]) - (matrix[2][1] * matrix[1][2])) - matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[2][0] * matrix[1][2]) + matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[2][0] * matrix[1][1]);
}

Coord RotationMatrix::rotateVector(const Coord& vector) const
{
    return Coord(vector.x * matrix[0][0] + vector.y * matrix[0][1] + vector.z * matrix[0][2],
                 vector.x * matrix[1][0] + vector.y * matrix[1][1] + vector.z * matrix[1][2],
                 vector.x * matrix[2][0] + vector.y * matrix[2][1] + vector.z * matrix[2][2]);
}

Coord RotationMatrix::rotateVectorInverse(const Coord& vector) const
{
    return Coord(vector.x * matrix[0][0] + vector.y * matrix[1][0] + vector.z * matrix[2][0],
                 vector.x * matrix[0][1] + vector.y * matrix[1][1] + vector.z * matrix[2][1],
                 vector.x * matrix[0][2] + vector.y * matrix[1][2] + vector.z * matrix[2][2]);
}

EulerAngles RotationMatrix::toEulerAngles() const
{
    // Ref: http://planning.cs.uiuc.edu/node103.html
    // NOTE: this algorithm works only if matrix[0][0] != 0 and matrix[2][2] != 0
    double tx = atan2(matrix[2][1], matrix[2][2]);
    double ty = atan2(-matrix[2][0], sqrt(matrix[2][1] * matrix[2][1] + matrix[2][2] * matrix[2][2]));
    double tz = atan2(matrix[1][0], matrix[0][0]);
    return EulerAngles(rad(tz), rad(ty), rad(tx));
}

Quaternion RotationMatrix::toQuaternion() const
{
    // Ref: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
    double w, x, y, z;
    double trace = matrix[0][0] + matrix[1][1] + matrix[2][2];
    if (trace > 0) {
        double s = 0.5 / sqrt(trace + 1.0);
        w = 0.25 / s;
        x = (matrix[2][1] - matrix[1][2]) * s;
        y = (matrix[0][2] - matrix[2][0]) * s;
        z = (matrix[1][0] - matrix[0][1]) * s;
    }
    else {
        if (matrix[0][0] > matrix[1][1] && matrix[0][0] > matrix[2][2]) {
            double s = 2.0 * sqrt(1.0 + matrix[0][0] - matrix[1][1] - matrix[2][2]);
            w = (matrix[2][1] - matrix[1][2]) / s;
            x = 0.25 * s;
            y = (matrix[0][1] + matrix[1][0]) / s;
            z = (matrix[0][2] + matrix[2][0]) / s;
        }
        else if (matrix[1][1] > matrix[2][2]) {
            double s = 2.0 * sqrt(1.0 + matrix[1][1] - matrix[0][0] - matrix[2][2]);
            w = (matrix[0][2] - matrix[2][0]) / s;
            x = (matrix[0][1] + matrix[1][0]) / s;
            y = 0.25 * s;
            z = (matrix[1][2] + matrix[2][1]) / s;
        }
        else {
            double s = 2.0 * sqrt(1.0 + matrix[2][2] - matrix[0][0] - matrix[1][1]);
            w = (matrix[1][0] - matrix[0][1]) / s;
            x = (matrix[0][2] + matrix[2][0]) / s;
            y = (matrix[1][2] + matrix[2][1]) / s;
            z = 0.25 * s;
        }
    }
    return Quaternion(w, x, y, z);
}

} /* namespace inet */

