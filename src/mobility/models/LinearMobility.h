//
// Author: Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)
// Copyright (C) 2005 Emin Ilker Cetinbas
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#ifndef LINEAR_MOBILITY_H
#define LINEAR_MOBILITY_H

#include "INETDefs.h"

#include "MovingMobilityBase.h"


/**
 * @brief Linear movement model. See NED file for more info.
 *
 * @ingroup mobility
 * @author Emin Ilker Cetinbas
 */
class INET_API LinearMobility : public MovingMobilityBase
{
  protected:
    double speed;          ///< speed of the host
    double angle;          ///< angle of linear motion
    double acceleration;   ///< acceleration of linear motion

  protected:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage);

    /** @brief Move the host*/
    virtual void move();

  public:
    LinearMobility();
};

#endif
