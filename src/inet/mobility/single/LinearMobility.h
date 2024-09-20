//
// Copyright (C) 2005 Emin Ilker Cetinbas
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
// Author: Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)
//

#ifndef __INET_LINEARMOBILITY_H
#define __INET_LINEARMOBILITY_H

#include "inet/mobility/base/MovingMobilityBase.h"

namespace inet {

/**
 * @brief Linear movement model. See NED file for more info.
 *
 * @ingroup mobility
 * @author Emin Ilker Cetinbas
 */
class INET_API LinearMobility : public MovingMobilityBase
{
  protected:
    double speed;

  protected:
    int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters.*/
    void initialize(int stage) override;

    /** @brief Move the host*/
    void move() override;

  public:
    double getMaxSpeed() const override { return speed; }
    LinearMobility();
};

} // namespace inet

#endif

