//
// Copyright (C) 2007 Peterpaul Klein Haneveld
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/* -*- mode:c++ -*- ********************************************************/
//

#ifndef __INET_TRACTORMOBILITY_H
#define __INET_TRACTORMOBILITY_H

#include "inet/mobility/base/LineSegmentsMobilityBase.h"

namespace inet {

/**
 * @brief Tractor movement model. See NED file for more info.
 *
 * NOTE: Does not yet support 3-dimensional movement.
 * @ingroup mobility
 * @author Peterpaul Klein Haneveld
 */
class INET_API TractorMobility : public LineSegmentsMobilityBase
{
  protected:
    double speed; // < speed along the trajectory
    double x1, y1, x2, y2; ///< rectangle bounds of the field
    int rowCount; ///< the number of rows that the tractor must take
    int step;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters. */
    virtual void initialize(int) override;

    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

    /** @brief Calculate a new target position to move to. */
    void setTargetPosition() override;

  public:
    TractorMobility();
    virtual double getMaxSpeed() const override { return speed; }
};

} // namespace inet

#endif

