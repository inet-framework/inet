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

#include "INETDefs.h"

#include "MovingMobilityBase.h"


/**
 * @brief Rectangle movement model. See NED file for more info.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class INET_API RectangleMobility : public MovingMobilityBase
{
  protected:
    // configuration
    double speed;          ///< speed of the host

    // state
    double d;  ///< distance from (x1,y1), measured clockwise on the perimeter
    double corner1, corner2, corner3, corner4;

  protected:
    /** @brief Initializes mobility model parameters.
     *
     * If the host is not stationary it calculates a random position on the rectangle.
     */
    virtual void initialize(int stage);

    /** @brief Initializes the position according to the mobility model. */
    virtual void initializePosition();

    /** @brief Move the host */
    virtual void move();

  public:
    RectangleMobility();
};

#endif
