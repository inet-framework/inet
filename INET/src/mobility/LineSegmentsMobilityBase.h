//
// Copyright (C) 2005 Andras Varga
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

#ifndef LINESEGMENTS_MOBILITY_BASE_H
#define LINESEGMENTS_MOBILITY_BASE_H

#include <omnetpp.h>
#include "BasicMobility.h"


/**
 * @brief Base class for mobility models where movement consists of
 * a sequence of linear movements of constant speed.
 *
 * Subclasses must redefine setTargetPosition() which is suppsed to set
 * a new target position and target time once the previous one is reached.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class INET_API LineSegmentsMobilityBase : public BasicMobility
{
  protected:
    // config
    double updateInterval; ///< time interval to update the host's position

    // state
    simtime_t targetTime;  ///< end time of current linear movement
    Coord targetPos;       ///< end position of current linear movement
    Coord step;            ///< step size (added to pos every updateInterval)
    bool stationary;       ///< if set to true, host won't move

  protected:
    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int);

    /** @brief Called upon arrival of a self messages*/
    virtual void handleSelfMsg(cMessage *msg);

    /** @brief Begin new line segment after previous one finished */
    virtual void beginNextMove(cMessage *msg);

    /**
     * @brief Should be redefined in subclasses. This method gets called
     * when targetPos and targetTime has been reached, and its task is
     * to set a new targetPos and targetTime. At the end of the movement
     * sequence, it should set targetTime=0.
     */
    virtual void setTargetPosition() = 0;

    /**
     * @brief Should be redefined in subclasses. Should invoke handleIfOutside(),
     * or directly one of the methods it relies on.
     */
    virtual void fixIfHostGetsOutside() = 0;
};

#endif

