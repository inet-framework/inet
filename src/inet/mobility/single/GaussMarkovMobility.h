//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
// Author: Marcin Kosiba
//

#ifndef __INET_GAUSSMARKOVMOBILITY_H
#define __INET_GAUSSMARKOVMOBILITY_H

#include "inet/mobility/base/LineSegmentsMobilityBase.h"

namespace inet {

/**
 * @brief Gauss Markov movement model. See NED file for more info.
 *
 * @author Marcin Kosiba
 */
class INET_API GaussMarkovMobility : public LineSegmentsMobilityBase
{
  protected:
    double speed = 0.0; ///< speed of the host
    double speedMean = 0.0; ///< speed mean
    double speedStdDev = 0.0; ///< speed standard deviation
    rad angle = rad(0.0); ///< angle of linear motion
    rad angleMean = rad(0.0); ///< angle mean
    rad angleStdDev = rad(0.0); ///< angle standard deviation
    double alpha = 0.0; ///< alpha parameter in [0;1] interval
    double margin = 0.0; ///< margin at which the host gets repelled from the border

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** @brief Initializes mobility model parameters.*/
    virtual void initialize(int stage) override;

    /** @brief If the host is too close to the border it is repelled */
    void preventBorderHugging();

    /** @brief Move the host*/
    virtual void move() override;

    /** @brief Calculate a new target position to move to. */
    virtual void setTargetPosition() override;

  public:
    virtual double getMaxSpeed() const override { return speed; }
    GaussMarkovMobility();
};

} // namespace inet

#endif

