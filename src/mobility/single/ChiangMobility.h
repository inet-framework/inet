//
// Author: Marcin Kosiba
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

#ifndef __INET_CHIANGMOBILITY_H
#define __INET_CHIANGMOBILITY_H

#include "inet/common/INETDefs.h"

#include "inet/mobility/common/LineSegmentsMobilityBase.h"

namespace inet {

/**
 * @brief Chiang's random walk movement model. See NED file for more info.
 *
 * @author Marcin Kosiba
 */
class INET_API ChiangMobility : public LineSegmentsMobilityBase
{
  protected:
    double speed;    ///< speed of the host
    double stateTransitionUpdateInterval;    ///< how often to calculate the new state
    int xState;    ///< 0 = negative direction, 1 = no move, 2 = positive direction
    int yState;    ///< 0 = negative direction, 1 = no move, 2 = positive direction

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage);

    /** @brief Gets the next state based on the current state. */
    int getNextStateIndex(int currentState);

    /** @brief Calculate a new target position to move to. */
    void setTargetPosition();

    /** @brief Move the host according to the current simulation time. */
    virtual void move();

  public:
    virtual double getMaxSpeed() const { return speed; }
    ChiangMobility();
};

} // namespace inet

#endif // ifndef __INET_CHIANGMOBILITY_H

