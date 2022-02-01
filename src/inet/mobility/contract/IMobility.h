//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IMOBILITY_H
#define __INET_IMOBILITY_H

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/Quaternion.h"

namespace inet {

/**
 * Abstract base class defining the public interface that must be provided by
 * all mobility modules. The mobility interface uses a 3D right-handed Euclidean
 * coordinate system.
 *
 * Coordinates are represented by 3D double precision tuples called Coord. The
 * coordinates are in X, Y, Z order, they are measured in metres. Conceptually,
 * the X axis goes to the right, the Y axis goes forward, the Z axis goes upward.
 *
 * Orientations are represented by 3D double precision Tait-Bryan (Euler) tuples
 * called EulerAngles. The angles are in Z, Y', X" order that is often called
 * intrinsic rotations, they are measured in radians. The default (unrotated)
 * orientation is along the X axis. Conceptually, the Z axis rotation is heading,
 * the Y' axis rotation is descending, the X" axis rotation is bank. For example,
 * positive rotation along the Z axis rotates X into Y (turns left), positive
 * rotation along the Y axis rotates Z into X (leans forward), positive rotation
 * along the X axis rotates Y into Z (leans right).
 *
 * @ingroup mobility
 */
class INET_API IMobility
{
  public:
    /** @brief A signal used to publish mobility state changes. */
    static simsignal_t mobilityStateChangedSignal;

  public:
    virtual ~IMobility() {}

    virtual int getId() const = 0;

    /**
     * Returns the maximum possible speed at any future time.
     */
    virtual double getMaxSpeed() const = 0;

    /**
     * Returns the position at the current simulation time.
     */
    virtual const Coord& getCurrentPosition() = 0;

    /**
     * Returns the velocity at the current simulation time.
     */
    virtual const Coord& getCurrentVelocity() = 0;

    /**
     * Returns the acceleration at the current simulation time.
     */
    virtual const Coord& getCurrentAcceleration() = 0;

    /**
     * Returns the angular position at the current simulation time.
     */
    virtual const Quaternion& getCurrentAngularPosition() = 0;

    /**
     * Returns the angular velocity at the current simulation time.
     */
    virtual const Quaternion& getCurrentAngularVelocity() = 0;

    /**
     * Returns the angular acceleration at the current simulation time.
     */
    virtual const Quaternion& getCurrentAngularAcceleration() = 0;

    /**
     * Returns the maximum position along each axes for.
     */
    virtual const Coord& getConstraintAreaMax() const = 0;

    /**
     * Returns the minimum position along each axes for.
     */
    virtual const Coord& getConstraintAreaMin() const = 0;
};

} // namespace inet

#endif

