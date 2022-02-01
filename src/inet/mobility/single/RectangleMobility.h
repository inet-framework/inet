//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECTANGLEMOBILITY_H
#define __INET_RECTANGLEMOBILITY_H

#include "inet/mobility/base/MovingMobilityBase.h"

namespace inet {

/**
 * @brief Rectangle movement model. See NED file for more info.
 *
 * @ingroup mobility
 */
class INET_API RectangleMobility : public MovingMobilityBase
{
  protected:
    // configuration
    double speed; ///< speed of the host

    // state
    double d; ///< distance from (x1,y1), measured clockwise on the perimeter
    double corner1, corner2, corner3, corner4;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters.
     *
     * If the host is not stationary it calculates a random position on the rectangle.
     */
    virtual void initialize(int stage) override;

    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

    /** @brief Move the host */
    virtual void move() override;

  public:
    virtual double getMaxSpeed() const override { return speed; }
    RectangleMobility();
};

} // namespace inet

#endif

