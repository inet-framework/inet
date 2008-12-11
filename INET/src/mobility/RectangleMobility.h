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

#ifndef RECTANGLE_MOBILITY_H
#define RECTANGLE_MOBILITY_H

#include <omnetpp.h>
#include "BasicMobility.h"


/**
 * @brief Rectangle movement model. See NED file for more info.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class INET_API RectangleMobility : public BasicMobility
{
  protected:
    // configuration
    double x1, y1, x2, y2; ///< rectangle bounds
    double speed;          ///< speed of the host
    double updateInterval; ///< time interval to update the hosts position
    bool stationary;       ///< if true, the host doesn't move

    // state
    double d;  ///< distance from (x1,y1), measured clockwise on the perimeter
    double corner1, corner2, corner3, corner4;

  protected:
    /** @brief Initializes mobility model parameters. */
    virtual void initialize(int);

    /** @brief Called upon arrival of a self messages */
    virtual void handleSelfMsg(cMessage *msg);

    /** @brief Move the host */
    virtual void move();

    /** @brief Maps d to (x,y) coordinates */
    virtual void calculateXY();
};

#endif

