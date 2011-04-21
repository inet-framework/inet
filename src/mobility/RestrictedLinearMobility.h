//
// Author: Alfonso Ariza
// Copyright (C) 2009 Alfonso Ariza
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

#ifndef _RESTRICTED_LINEAR_MOBILITY_H
#define _RESTRICTED_LINEAR_MOBILITY_H

#include <omnetpp.h>
#include "LinearMobility.h"


/**
 * @brief Linear movement model. See NED file for more info.
 *
 * @ingroup mobility
 * @author Emin Ilker Cetinbas
 */
class INET_API RestrictedLinearMobility : public LinearMobility
{
  protected:
    double x1;
    double y1;
    double x2;
    double y2;

  protected:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);
    virtual Coord getRandomPosition();
    virtual void reflectIfOutside(Coord& targetPos, Coord& step, double& angle);
};

#endif

