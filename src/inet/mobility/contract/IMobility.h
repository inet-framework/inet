/* -*- mode:c++ -*- ********************************************************
 * file:        IMobility.h
 *
 * author:      Levente Meszaros
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              (C) 2005 Andras Varga
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 *************************************************************************/

#ifndef __INET_IMOBILITY_H
#define __INET_IMOBILITY_H

#include "inet/common/INETDefs.h"
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
 * @author Levente Meszaros
 */
class INET_API IMobility
{
  public:
    /** @brief A signal used to publish mobility state changes. */
    static simsignal_t mobilityStateChangedSignal;

  public:
    virtual ~IMobility() {}

    /**
     * Returns the maximum possible speed at any future time.
     */
    virtual double getMaxSpeed() const = 0;

    /**
     * Returns the current position at the current simulation time.
     */
    virtual Coord getCurrentPosition() = 0;

    /**
     * Returns the current velocity at the current simulation time.
     */
    virtual Coord getCurrentVelocity() = 0;

    /**
     * Returns the current acceleration at the current simulation time.
     */
    virtual Coord getCurrentAcceleration() = 0;

    /**
     * Returns the current angular position at the current simulation time.
     */
    virtual Quaternion getCurrentAngularPosition() = 0;

    /**
     * Returns the current angular velocity at the current simulation time.
     */
    virtual Quaternion getCurrentAngularVelocity() = 0;

    /**
     * Returns the current angular acceleration at the current simulation time.
     */
    virtual Quaternion getCurrentAngularAcceleration() = 0;

    /**
     * Returns the maximum positions along each axes.
     */
    virtual Coord getConstraintAreaMax() const = 0;

    /**
     * Returns the minimum positions along each axes.
     */
    virtual Coord getConstraintAreaMin() const = 0;
};

} // namespace inet

#endif // ifndef __INET_IMOBILITY_H

