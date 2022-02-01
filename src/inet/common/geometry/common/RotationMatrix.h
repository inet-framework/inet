//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ROTATIONMATRIX_H
#define __INET_ROTATIONMATRIX_H

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/EulerAngles.h"
#include "inet/common/geometry/common/Quaternion.h"

namespace inet {

/*
 * Represents a 3D rotation matrix.
 */
class INET_API RotationMatrix
{
  protected:
    double matrix[3][3];

  protected:
    void computeRotationMatrix(const double& q0, const double& q1, const double& q2, const double& q3);
    double computeDeterminant() const;

  public:
    RotationMatrix();
    RotationMatrix(const double matrix[3][3]);
    RotationMatrix(const EulerAngles& eulerAngles);

    Coord rotateVector(const Coord& vector) const;
    Coord rotateVectorInverse(const Coord& vector) const;

    EulerAngles toEulerAngles() const;
    Quaternion toQuaternion() const;
};

} // namespace inet

#endif

