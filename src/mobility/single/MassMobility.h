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

#ifndef __INET_MASSMOBILITY_H
#define __INET_MASSMOBILITY_H

#include "inet/common/INETDefs.h"

#include "inet/mobility/common/LineSegmentsMobilityBase.h"

namespace inet {

/**
 * @brief Models the mobility of with mass, making random motions.
 * See NED file for more info.
 *
 * @ingroup mobility
 * @author Emin Ilker Cetinbas, Andras Varga
 */
class INET_API MassMobility : public LineSegmentsMobilityBase
{
  protected:
    // config (see NED file for explanation)
    cPar *changeIntervalParameter;
    cPar *changeAngleByParameter;
    cPar *speedParameter;

    // current state
    double angle;    ///< angle of linear motion

    simtime_t previousChange;
    Coord sourcePosition;

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters. */
    virtual void initialize(int stage);

    /** @brief Move the host according to the current simulation time. */
    virtual void move();

    /** @brief Calculate a new target position to move to. */
    virtual void setTargetPosition();

  public:
    MassMobility();
    virtual double getMaxSpeed() const;
};

} // namespace inet

#endif // ifndef __INET_MASSMOBILITY_H

