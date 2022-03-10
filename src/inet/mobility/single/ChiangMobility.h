//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
// Author: Marcin Kosiba
//

#ifndef __INET_CHIANGMOBILITY_H
#define __INET_CHIANGMOBILITY_H

#include "inet/mobility/base/LineSegmentsMobilityBase.h"

namespace inet {

/**
 * @brief Chiang's random walk movement model. See NED file for more info.
 *
 * @author Marcin Kosiba
 */
class INET_API ChiangMobility : public LineSegmentsMobilityBase
{
  protected:
    double speed; ///< speed of the host
    double stateTransitionUpdateInterval; ///< how often to calculate the new state
    int xState; ///< 0 = negative direction, 1 = no move, 2 = positive direction
    int yState; ///< 0 = negative direction, 1 = no move, 2 = positive direction

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage) override;

    /** @brief Gets the next state based on the current state. */
    int getNextStateIndex(int currentState);

    /** @brief Calculate a new target position to move to. */
    void setTargetPosition() override;

    /** @brief Move the host according to the current simulation time. */
    virtual void move() override;

  public:
    virtual double getMaxSpeed() const override { return speed; }
    ChiangMobility();
};

} // namespace inet

#endif

