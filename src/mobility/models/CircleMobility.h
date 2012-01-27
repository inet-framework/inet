//
// Copyright (C) 2005 Andras Varga
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


#ifndef CIRCLE_MOBILITY_H
#define CIRCLE_MOBILITY_H

#include "INETDefs.h"

#include "MovingMobilityBase.h"


/**
 * @brief Circle movement model. See NED file for more info.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class INET_API CircleMobility : public MovingMobilityBase
{
  protected:
    double cx;
    double cy;
    double cz;
    double r;
    double startAngle;
    double speed;
    /** @brief angular velocity [rad/s], derived from speed and radius. */
    double omega;

    /** @brief Direction from the center of the circle. */
    double angle;

  protected:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage);

    /** @brief Initializes the position according to the mobility model. */
    virtual void initializePosition();

    /** @brief Move the host according to the current simulation time. */
    virtual void move();

  public:
    CircleMobility();
};

#endif
