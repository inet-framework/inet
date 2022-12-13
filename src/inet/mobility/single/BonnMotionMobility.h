//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BONNMOTIONMOBILITY_H
#define __INET_BONNMOTIONMOBILITY_H

#include "inet/mobility/base/LineSegmentsMobilityBase.h"
#include "inet/mobility/single/BonnMotionFileCache.h"

namespace inet {

/**
 * @brief Uses the BonnMotion native file format. See NED file for more info.
 *
 * @ingroup mobility
 */
class INET_API BonnMotionMobility : public LineSegmentsMobilityBase
{
  protected:
    BonnMotionFileCache& cache = SIMULATION_SHARED_VARIABLE(cache);

    // state
    bool is3D = false;
    const BonnMotionFile::Line *lines = nullptr;
    int currentLine = -1;
    double maxSpeed = 0; // the possible maximum speed at any future time

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters. */
    virtual void initialize(int stage) override;

    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

    /** @brief Overridden from LineSegmentsMobilityBase. */
    virtual void setTargetPosition() override;

    /** @brief Overridden from LineSegmentsMobilityBase. */
    virtual void move() override;

    virtual void computeMaxSpeed();

  public:
    virtual double getMaxSpeed() const override { return maxSpeed; }
};

} // namespace inet

#endif

