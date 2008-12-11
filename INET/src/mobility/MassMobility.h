//
// Author: Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)
// Generalization: Andras Varga
// Copyright (C) 2005 Emin Ilker Cetinbas, Andras Varga
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


#ifndef MASS_MOBILITY_H
#define MASS_MOBILITY_H

#include <omnetpp.h>

#include "BasicMobility.h"


/**
 * @brief Models the mobility of with mass, making random motions.
 * See NED file for more info.
 *
 * @ingroup mobility
 * @author Emin Ilker Cetinbas, Andras Varga
 */
class INET_API MassMobility : public BasicMobility
{
  protected:
    // config (see NED file for explanation)
    cPar *changeInterval;
    cPar *changeAngleBy;
    cPar *speed;

    // current state
    double currentSpeed;   ///< speed of the host
    double currentAngle;   ///< angle of linear motion
    double updateInterval; ///< time interval to update the hosts position
    Coord step;            ///< calculated from speed, angle and updateInterval

  protected:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

   protected:
    /** @brief Called upon arrival of a self messages*/
    virtual void handleSelfMsg(cMessage *msg);

    /** @brief Move the host*/
    virtual void move();
};

#endif
