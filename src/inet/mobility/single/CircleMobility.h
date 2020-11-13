//
// Copyright (C) 2005 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_CIRCLEMOBILITY_H
#define __INET_CIRCLEMOBILITY_H

#include "inet/mobility/base/MovingMobilityBase.h"

namespace inet {

/**
 * @brief Circle movement model. See NED file for more info.
 *
 * @ingroup mobility
 */
class INET_API CircleMobility : public MovingMobilityBase
{
  protected:
    double cx = 0;
    double cy = 0;
    double cz = 0;
    double r = -1;
    rad startAngle = rad(0);
    double speed = 0;
    /** @brief angular velocity [rad/s], derived from speed and radius. */
    double omega = 0;

    /** @brief Direction from the center of the circle. */
    rad angle = rad(0);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage) override;

    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

    /** @brief Move the host according to the current simulation time. */
    virtual void move() override;

  public:
    virtual double getMaxSpeed() const override { return speed; }

    virtual const Quaternion& getCurrentAngularVelocity() override { return lastAngularVelocity; }
    virtual const Quaternion& getCurrentAngularAcceleration() override { return Quaternion::IDENTITY; }
};

} // namespace inet

#endif

